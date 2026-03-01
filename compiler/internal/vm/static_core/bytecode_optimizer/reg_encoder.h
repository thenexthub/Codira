/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_REG_ENCODER_H
#define PANDA_REG_ENCODER_H

#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/ir/graph_visitor.h"

/*
 * Register Encoder.
 *
 * After compiler's register allocation layout of the virtual frame looks like:
 *
 * |<-locals->|<-free registers->|<-RegAlloc temps->|<-arguments->|
 *
 * where:
 *
 * * locals (>= 0) are registers allocated for function's local variables;
 * * temps (>= 0) are temporary registers that can be allocated by spill-fill
 *   resolver during breaking mov chain cycles;
 * * arguments (>= 0) are registers allocated for function's arguemnts.
 *
 * The size of the frame is fixed (see register allocator for details).
 *
 * Locals and temps are allocated densely: if there are more than 0 locals
 * (or temps) then all registers go strictly in order without "holes".
 * Space for arguments, however, can contain "holes" (each "hole" corresponds
 * to an unused argument).
 *
 * For locals and temps, it is not guaranteed that their number equals to the
 * number of registers used in the unoptimized bytecode. Total number of arguments
 * is of course constant throughout optimization pipeline.
 *
 * This pass solves following problems:
 *
 * 1. Shift temps and arguments to the left adjacently to locals, and reserve the
 *    range temps from free registers if necessary, consistently changing all register
 *    numbers for affected instructions. After this is done, the virtual frame looks like:
 *
 * |<-locals->|<-range temps->|<-RegAlloc temps->|<-arguments->|<-free registers->|
 *
 * 2. Check if IR contains any instructions that can encode only [r0, r15] with
 *    actual inputs r16+. If such instructions are found, some lower registers
 *    (starting with r0, but no more than MAX_NUM_INPUTS registers) are reserved as
 *    temps and corresponding spills are emitted where needed. After this is done,
 *    the virtual frame looks like:
 *
 * |<-Renumber temps->|<-locals->|<-range temps->|<-RegAlloc temps->|<-arguments->|<-free registers->|
 *
 * After the pass:
 *
 *  * Usage mask is updated accordingly.
 *  * num_locals + num_temps + num_max_range_input is stored to graph with SetStackSlotsCount
 */

namespace ark::bytecodeopt {
struct RegContent {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    compiler::Register reg;
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    compiler::DataType::Type type;

    RegContent() : reg(compiler::GetInvalidReg()), type(compiler::DataType::NO_TYPE) {}
    RegContent(compiler::Register r, compiler::DataType::Type t) : reg(r), type(t) {}

    bool operator==(const RegContent &other) const
    {
        return reg == other.reg && type == other.type;
    }

    bool operator!=(const RegContent &other) const
    {
        return !(*this == other);
    }
};
using RegContentMap = ArenaUnorderedMap<compiler::Register, RegContent>;
using RegContentVec = ArenaVector<std::pair<compiler::Register, RegContent>>;

enum class RegEncoderState { IDLE, RENUMBER_ARGS, RESERVE_TEMPS, INSERT_SPILLS };

using ark::compiler::BasicBlock;
using ark::compiler::Inst;
using ark::compiler::Opcode;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class PANDA_PUBLIC_API RegEncoder : public compiler::Optimization, public compiler::GraphVisitor {
public:
    explicit RegEncoder(compiler::Graph *graph) : compiler::Optimization(graph) {}

    ~RegEncoder() override = default;
    NO_COPY_SEMANTIC(RegEncoder);
    NO_MOVE_SEMANTIC(RegEncoder);

    // true: Pass applied successfully
    // false: Unable to apply pass because of insufficient number of registers
    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "RegEncoder";
    }

    void Check4Width(compiler::Inst *inst);
    void Check8Width(compiler::Inst *inst);

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    static void VisitSpillFill([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitConstant([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitCatchPhi([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitCallStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallVirtual(GraphVisitor *v, Inst *inst);
    static void VisitInitObject(GraphVisitor *v, Inst *inst);
    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);
    static void VisitLoadObject(GraphVisitor *v, Inst *instBase);
    static void VisitLoadStatic(GraphVisitor *v, Inst *instBase);
    static void VisitStoreObject(GraphVisitor *v, Inst *instBase);
    static void VisitStoreStatic(GraphVisitor *v, Inst *instBase);
    static void VisitLoadString([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);
    static void VisitReturn([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst);

    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst);

#include "generated/reg_encoder_visitors.inc"

#include "generated/check_width.h"

    void VisitDefault(Inst *inst) override
    {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Opcode " << compiler::GetOpcodeString(inst->GetOpcode())
                                       << " not yet implemented in RegEncoder";
        success_ = false;
    }

#include "compiler/optimizer/ir/visitor.inc"

private:
    void CalculateNumNeededTemps();
    void CalculateNumNeededTempsForInst(compiler::Inst *inst);
    void RenumberRegs(compiler::Register minReg, compiler::Register delta);
    bool RenumberArgRegs();
    void InsertSpills();
    void InsertSpillsForInst(compiler::Inst *inst);
    void InsertSpillsForDynInputsInst(compiler::Inst *inst);
    bool InsertSpillsForDynRangeInst(compiler::Inst *inst, size_t nargs, size_t start);
    size_t GetStartInputIndex(compiler::Inst *inst);
    void RenumberDstReg(compiler::Inst *inst, size_t temp, size_t rangeTemp = 0, bool largeTemp = false);

    compiler::Register GetNumArgsFromGraph() const
    {
        auto adapter = GetGraph()->GetRuntime();
        auto method = GetGraph()->GetMethod();
        auto numArgs = adapter->GetMethodTotalArgumentsCount(method);
        ASSERT(numArgs <= compiler::GetFrameSize());
        return numArgs;
    }

    compiler::Register GetNumLocalsFromGraph() const
    {
        auto numLocals = GetGraph()->GetStackSlotsCount();
        ASSERT(numLocals <= compiler::GetFrameSize());
        return numLocals;
    }

    compiler::Register GetNumRegs() const
    {
        auto numRegs = GetNumLocalsFromGraph() + GetNumArgsFromGraph();
        ASSERT(numRegs <= compiler::GetFrameSize());
        return numRegs;
    }

    void SaveNumLocalsToGraph(uint32_t numLocals) const
    {
        ASSERT(numLocals <= compiler::GetFrameSize());
        GetGraph()->SetStackSlotsCount(numLocals);
    }

    bool GetStatus() const
    {
        return success_;
    }

    bool CheckStatus() const
    {
        return !GetGraph()->IsAbcKit() || success_;
    }

    static void CallHelper(compiler::GraphVisitor *visitor, Inst *inst)
    {
        auto *re = static_cast<RegEncoder *>(visitor);
        re->Check4Width(inst);
    }

private:
    compiler::Register numTemps_ {0};
    RegEncoderState state_ {RegEncoderState::IDLE};
    compiler::Register numMaxRangeInput_ {0};
    compiler::Register rangeTempsStart_ {0};
    compiler::Register numChangedWidth_ {0};

    bool success_ {true};
};
}  // namespace ark::bytecodeopt

#endif  // PANDA_REG_ENCODER_H
