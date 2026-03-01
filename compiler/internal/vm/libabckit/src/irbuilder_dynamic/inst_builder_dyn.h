/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_INST_BUILDER_H
#define LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_INST_BUILDER_H

#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir/basicblock.h"
#include "static_core/compiler/optimizer/analysis/loop_analyzer.h"

#include "libabckit/src/adapter_dynamic/runtime_adapter_dynamic.h"
#include "libabckit/src/irbuilder_dynamic/bytecode_inst.h"

namespace libabckit {

constexpr int64_t INVALID_OFFSET = std::numeric_limits<int64_t>::max();
using ark::BytecodeInstruction;

class InstBuilder {
public:
    InstBuilder(ark::compiler::Graph *graph, AbckitRuntimeAdapterDynamic::MethodPtr method)
        : graph_(graph),
          runtime_(graph->GetRuntime()),
          defs_(graph->GetLocalAllocator()->Adapter()),
          method_(method),
          vregsAndArgsCount_(graph->GetRuntime()->GetMethodRegistersCount(method) +
                             graph->GetRuntime()->GetMethodTotalArgumentsCount(method)),
          instructionsBuf_(GetGraph()->GetRuntime()->GetMethodCode(GetGraph()->GetMethod())),
          classId_ {runtime_->GetClassIdForMethod(method_)}
    {
        noTypeMarker_ = GetGraph()->NewMarker();
        visitedBlockMarker_ = GetGraph()->NewMarker();

        defs_.resize(graph_->GetVectorBlocks().size(),
                     ark::compiler::InstVector(graph->GetLocalAllocator()->Adapter()));
        for (auto &v : defs_) {
            v.resize(vregsAndArgsCount_ + 1);
        }

        for (auto bb : graph->GetBlocksRPO()) {
            if (bb->IsCatchBegin()) {
                for (size_t vreg = 0; vreg < GetVRegsCount(); vreg++) {
                    auto catchPhi = GetGraph()->CreateInstCatchPhi();
                    catchPhi->SetPc(bb->GetGuestPc());
                    catchPhi->SetMarker(GetNoTypeMarker());
                    bb->AppendInst(catchPhi);
                    if (vreg == vregsAndArgsCount_) {
                        catchPhi->SetIsAcc();
                    }
                }
            }
        }
    }

    NO_COPY_SEMANTIC(InstBuilder);
    NO_MOVE_SEMANTIC(InstBuilder);
    ~InstBuilder()
    {
        GetGraph()->EraseMarker(noTypeMarker_);
        GetGraph()->EraseMarker(visitedBlockMarker_);
    }

    /**
     * Content of this function is auto generated from inst_builder.erb and is located in inst_builder_gen.cpp file
     * @param instruction Pointer to bytecode instruction
     */
    void BuildInstruction(const BytecodeInst *instruction);

    bool IsFailed() const
    {
        return failed_;
    }

    /**
     * Return jump offset for instruction `inst`, 0 if it is not jump instruction.
     */
    static int64_t GetInstructionJumpOffset(const BytecodeInst *inst);

    void SetCurrentBlock(ark::compiler::BasicBlock *bb)
    {
        currentBb_ = bb;
        currentDefs_ = &defs_[bb->GetId()];
    }

    void Prepare();

    void UpdatePreds(ark::compiler::BasicBlock *bb, ark::compiler::Inst *inst);
    void SetType(ark::compiler::Inst *inst);
    void FixInstructions();
    void CheckInstructions(ark::compiler::Inst *inst);
    void ResolveConstants();
    void SplitConstant(ark::compiler::ConstantInst *constInst);
    void CleanupInst(ark::compiler::BasicBlock *block, ark::compiler::Inst *inst);
    void CleanupCatchPhis();
    void AddPhiToDifferent();

    static void RemoveNotDominateInputs(ark::compiler::SaveStateInst *saveState);

    size_t GetPc(const uint8_t *instPtr) const;

    void UpdateDefs();

    const auto &GetCurrentDefs()
    {
        ASSERT(currentDefs_ != nullptr);
        return *currentDefs_;
    }

    void AddCatchPhiInputs(const ark::ArenaUnorderedSet<ark::compiler::BasicBlock *> &catchHandlers,
                           const ark::compiler::InstVector &defs, ark::compiler::Inst *throwableInst);

    ark::compiler::SaveStateInst *CreateSaveState(ark::compiler::Opcode opc, size_t pc);

    static void SetParamSpillFill(ark::compiler::Graph *graph, ark::compiler::ParameterInst *paramInst, size_t numArgs,
                                  size_t i, ark::compiler::DataType::Type type);

private:
    void UpdateDefsForCatch();
    void UpdateDefsForLoopHead();

    size_t GetVRegsCount() const
    {
        return vregsAndArgsCount_ + 1;
    }

    void AddInstruction(ark::compiler::Inst *inst)
    {
        ASSERT(currentBb_);
        currentBb_->AppendInst(inst);
        LIBABCKIT_LOG(DEBUG) << *inst << std::endl;
    }

    void UpdateDefinition(size_t vreg, ark::compiler::Inst *inst)
    {
        ASSERT(vreg < currentDefs_->size());
        (*currentDefs_)[vreg] = inst;
    }

    void UpdateDefinitionAcc(ark::compiler::Inst *inst)
    {
        (*currentDefs_)[vregsAndArgsCount_] = inst;
    }

    ark::compiler::Inst *GetDefinition(size_t vreg)
    {
        ASSERT(vreg < currentDefs_->size());
        ASSERT((*currentDefs_)[vreg] != nullptr);

        if (vreg >= currentDefs_->size() || (*currentDefs_)[vreg] == nullptr) {
            failed_ = true;
            LIBABCKIT_LOG(DEBUG) << "GetDefinition failed for verg " << vreg << std::endl;
            return nullptr;
        }
        return (*currentDefs_)[vreg];
    }

    ark::compiler::Inst *GetDefinitionAcc()
    {
        auto *accInst = (*currentDefs_)[vregsAndArgsCount_];
        ASSERT(accInst != nullptr);

        if (accInst == nullptr) {
            failed_ = true;
            LIBABCKIT_LOG(DEBUG) << "GetDefinitionAcc failed\n";
        }
        return accInst;
    }

    auto FindOrCreate32BitConstant(uint32_t value)
    {
        auto inst = GetGraph()->FindOrCreateConstant<uint32_t>(value);
        if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
            LIBABCKIT_LOG(DEBUG) << "create new constant: value=" << value << ", inst=" << inst->GetId() << std::endl;
        }
        return inst;
    }

    auto FindOrCreateConstant(uint64_t value)
    {
        auto inst = GetGraph()->FindOrCreateConstant<uint64_t>(value);
        if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
            LIBABCKIT_LOG(DEBUG) << "create new constant: value=" << value << ", inst=" << inst->GetId() << std::endl;
        }
        return inst;
    }

    auto FindOrCreateDoubleConstant(double value)
    {
        auto inst = GetGraph()->FindOrCreateConstant<double>(value);
        if (inst->GetId() == GetGraph()->GetCurrentInstructionId() - 1) {
            LIBABCKIT_LOG(DEBUG) << "create new constant: value=" << value << ", inst=" << inst->GetId() << std::endl;
        }
        return inst;
    }

    void BuildEcma([[maybe_unused]] const BytecodeInst *bcInst);
    template <bool WITH_SPECULATIVE = false>
    void BuildEcmaAsIntrinsics([[maybe_unused]] const BytecodeInst *bcInst);
    void BuildAbcKitLoadStringIntrinsic(const BytecodeInst *bcInst);

    template <ark::compiler::Opcode OPCODE>
    void BuildLoadFromPool(const BytecodeInst *bcInst);

    ark::compiler::Graph *GetGraph()
    {
        return graph_;
    }

    const ark::compiler::Graph *GetGraph() const
    {
        return graph_;
    }

    const ark::compiler::RuntimeInterface *GetRuntime() const
    {
        return runtime_;
    }

    ark::compiler::RuntimeInterface *GetRuntime()
    {
        return runtime_;
    }

    auto GetMethod() const
    {
        return method_;
    }

    auto GetClassId() const
    {
        return classId_;
    }

    ark::compiler::Marker GetNoTypeMarker() const
    {
        return noTypeMarker_;
    }

    ark::compiler::Marker GetVisitedBlockMarker() const
    {
        return visitedBlockMarker_;
    }

    void SetTypeRec(ark::compiler::Inst *inst, ark::compiler::DataType::Type type);

    size_t GetMethodArgumentsCount(uintptr_t id) const;

private:
    static constexpr size_t INPUT_2 = 2;
    static constexpr size_t INPUT_3 = 3;
    static constexpr size_t TWO_INPUTS = 2;

    ark::compiler::Graph *graph_ {nullptr};
    ark::compiler::RuntimeInterface *runtime_ {nullptr};
    ark::compiler::BasicBlock *currentBb_ {nullptr};

    // Definitions vector of currently processed basic block
    ark::compiler::InstVector *currentDefs_ {nullptr};
    // Contains definitions of the virtual registers in all basic blocks
    ark::ArenaVector<ark::compiler::InstVector> defs_;

    AbckitRuntimeAdapterDynamic::MethodPtr method_ {nullptr};
    // Set to true if builder failed to build IR
    bool failed_ {false};
    // Number of virtual registers and method arguments
    const size_t vregsAndArgsCount_;
    // Marker for instructions with undefined type in the building phase
    ark::compiler::Marker noTypeMarker_;
    ark::compiler::Marker visitedBlockMarker_;

    // Pointer to start position of bytecode instructions buffer
    const uint8_t *instructionsBuf_ {nullptr};

    size_t classId_;

#include "intrinsics_ir_build.inl.h"
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_INST_BUILDER_H
