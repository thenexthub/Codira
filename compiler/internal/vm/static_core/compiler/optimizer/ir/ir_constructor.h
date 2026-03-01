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

#ifndef PANDA_IR_CONSTRUCTOR_H
#define PANDA_IR_CONSTRUCTOR_H

#include <memory>

#ifdef PANDA_COMPILER_DEBUG_INFO
#include "debug_info.h"
#endif

#include "graph.h"
#include "graph_checker.h"
#include "mark_word.h"
#include "optimizer/ir_builder/inst_builder.h"

namespace ark::compiler {
/**
 * This class aims to simplify IR construction.
 *
 * Example:
 *  auto g = CreateEmptyGraph();
 *  GRAPH(g) {
 *      BASIC_BLOCK(0, 1, 2) {
 *          INST(0, Opcode::IntConstant).Constant(12);
 *          INST(1, Opcode::IntConstant).Constant(12);
 *          INST(2, Opcode::Add).Inputs(0, 1);
 *          INST(6, Opcode::Compare).Inputs(2).CC(ConditionCode::CC_AE);
 *          INST(7, Opcode::If).Inputs(6);
 *      }
 *      BASIC_BLOCK(1, 2) {
 *          INST(3, Opcode::Not).Inputs(0);
 *      }
 *      BASIC_BLOCK(2, -1) {
 *          INST(4, Opcode::Phi).Inputs(2, 3);
 *          INST(5, Opcode::Not).Inputs(4);
 *      }
 *  }
 *  g->Dump(cerr);
 *
 * GRAPH(g) macro initializies Builder object by 'g' graph. Builder works with this graph only inside followed scope
 *          in braces.
 * BASIC_BLOCK creates new basic block and add it to the current graph. All code inside followed scope will work with
 *          this basic block.
 *          First argument is ID of basic block. It must be unique for graph.
 *          All remaining arguments are IDs of successors blocks. '-1' value means that there is no successor. Block
 *          that hasn't successors is considered as end block.
 *          Block with '0' ID is considered as start block.
 * INST creates new instruction and append it to the current basic block.
 *          First parameter is ID of instruction. It must be unique within the current graph
 *          Second parameter is an opcode.
 *          Dataflow can be constructed via 'Inputs' method, that gets IDs of the input instructions as parameters.
 *          All other properties of instruction may be set via corresponding proxy methods, defined in Builder.
 */
class IrConstructor final {
public:
    static const size_t ID_ENTRY_BB = 0;
    static const size_t ID_EXIT_BB = 1U;

    static constexpr DataType::Type MARK_WORD_TYPE = DataType::GetIntTypeBySize(sizeof(MarkWord::MarkWordSize));

    IrConstructor() : aa_(SpaceType::SPACE_TYPE_COMPILER) {}

    IrConstructor &SetGraph(Graph *graph)
    {
        graph_ = graph;
        if (graph_->GetStartBlock() == nullptr) {
            graph_->CreateStartBlock();
        }
        if (graph_->GetEndBlock() == nullptr) {
            graph_->CreateEndBlock(0U);
        }
        ASSERT(graph_->GetVectorBlocks().size() == 2U);
        bbMap_.clear();
        bbMap_[ID_ENTRY_BB] = graph_->GetStartBlock();
        bbMap_[ID_EXIT_BB] = graph_->GetEndBlock();
        bbSuccsMap_.clear();
        instMap_.clear();
        instInputsMap_.clear();
        saveStateInstVregsMap_.clear();
        phiInstInputsMap_.clear();
        return *this;
    }

    template <size_t ID>
    IrConstructor &NewBlock()
    {
        ASSERT(ID != ID_ENTRY_BB && ID != ID_EXIT_BB);
        ASSERT(bbMap_.count(ID) == 0);
        ASSERT(CurrentBb() == nullptr);
        auto bb = graph_->GetAllocator()->New<BasicBlock>(graph_);
        bb->SetGuestPc(0U);
#ifdef NDEBUG
        graph_->AddBlock(bb);
#else
        graph_->AddBlock(bb, ID);
#endif
        currentBb_ = {ID, bb};
        bbMap_[ID] = bb;
        // add connection the first custom block with entry
        if (bbSuccsMap_.empty()) {
            graph_->GetStartBlock()->AddSucc(bb);
        }
        return *this;
    }

    template <typename... Args>
    IrConstructor &NewInst(size_t id, Args &&...args)
    {
        ASSERT_DO(instMap_.find(id) == instMap_.end(),
                  std::cerr << "Instruction with same Id " << id << " already exists");
        auto inst = graph_->CreateInst(std::forward<Args>(args)...);
        inst->SetId(id);
        for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
            inst->SetSrcReg(i, GetInvalidReg());
        }
        currentInst_ = {id, inst};
        instMap_[id] = inst;
        auto block = CurrentBb();
        if (block == nullptr) {
            block = graph_->GetStartBlock();
        }
        ASSERT(block);
        if (inst->IsPhi()) {
            block->AppendPhi(inst);
        } else {
            block->AppendInst(inst);
        }
#ifndef NDEBUG
        if (inst->IsLowLevel()) {
            // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
            graph_->SetLowLevelInstructionsEnabled();
        }
#endif
        if (inst->IsSaveState()) {
            graph_->SetVRegsCount(std::max(graph_->GetVRegsCount(), sizeof...(args)));
        }
        return *this;
    }

    template <typename T>
    IrConstructor &NewConstant(size_t id, [[maybe_unused]] T value)
    {
        ASSERT_DO(instMap_.find(id) == instMap_.end(),
                  std::cerr << "Instruction with same Id " << id << "already exists");
        Inst *inst = nullptr;
        auto toStartBb = (CurrentBbIndex() == 0) || (CurrentBbIndex() == -1);
        if constexpr (std::is_same<T, std::nullptr_t>()) {
            if (toStartBb) {
                inst = graph_->GetOrCreateNullPtr();
            } else {
                inst = graph_->CreateInstNullPtr(DataType::REFERENCE);
                CurrentBb()->AppendInst(inst);
            }
        } else {  // NOLINT(readability-misleading-indentation)
            if (toStartBb) {
                inst = graph_->FindOrCreateConstant(value);
            } else {
                inst = graph_->CreateInstConstant(value, graph_->IsBytecodeOptimizer());
                CurrentBb()->AppendInst(inst);
            }
        }
        inst->SetId(id);
        instMap_[id] = inst;
        currentInst_ = {id, inst};
        return *this;
    }

    IrConstructor &NewParameter(int id, uint16_t argNumber)
    {
        ASSERT_DO(instMap_.find(id) == instMap_.end(),
                  std::cerr << "Instruction with same Id " << id << "already exists");
        auto inst = graph_->AddNewParameter(argNumber);
        inst->SetId(id);
        instMap_[id] = inst;
        currentInst_ = {id, inst};
        return *this;
    }

    IrConstructor &AddNullptrInst(int id)
    {
        ASSERT_DO(instMap_.find(id) == instMap_.end(),
                  std::cerr << "Instruction with same Id " << id << "already exists");
        auto inst = graph_->GetOrCreateNullPtr();
        inst->SetId(id);
        instMap_[id] = inst;
        currentInst_ = {id, inst};
        return *this;
    }

    IrConstructor &Succs(std::vector<int> succs)
    {
        bbSuccsMap_.emplace_back(CurrentBbIndex(), std::move(succs));
        return *this;
    }

    /**
     * Define inputs for current instruction.
     * Input is an index of input instruction.
     */
    template <typename... Args>
    IrConstructor &Inputs(Args... inputs)
    {
        ASSERT(!CurrentInst()->IsCall() && !CurrentInst()->IsIntrinsic());
        instInputsMap_[CurrentInstIndex()].reserve(sizeof...(inputs));
        // NOLINTNEXTLINE(bugprone-suspicious-semicolon)
        if constexpr (sizeof...(inputs) != 0) {
            AddInput(inputs...);
        }
        return *this;
    }

    /**
     * Define inputs for current call-, intrinsic-, or phi-instriction.
     * Input is defined by std::pair.
     * In case of phi: first is index of basic block, second is index of input instruction.
     * In case of call and intrinsic: first is Type of input, second is index of input instruction.
     */
    IrConstructor &Inputs(std::initializer_list<std::pair<int, int>> inputs)
    {
        ASSERT(CurrentInst()->IsPhi() || CurrentInst()->IsCall() || CurrentInst()->IsInitObject() ||
               CurrentInst()->IsIntrinsic());
        if (CurrentInst()->IsPhi()) {
            phiInstInputsMap_[CurrentInstIndex()].reserve(inputs.size());
            for (const auto &input : inputs) {
                phiInstInputsMap_[CurrentInstIndex()].push_back(input);
            }
        } else {
            auto opc = CurrentInst()->GetOpcode();
            using Types = InputTypesMixin<DynamicInputsInst>;
            Types *types;
            switch (opc) {
                case Opcode::Intrinsic:
                    types = static_cast<Types *>(CurrentInst()->CastToIntrinsic());
                    break;
                case Opcode::CallIndirect:
                    types = static_cast<Types *>(CurrentInst()->CastToCallIndirect());
                    break;
                case Opcode::Builtin:
                    types = static_cast<Types *>(CurrentInst()->CastToBuiltin());
                    break;
                case Opcode::InitObject:
                    types = static_cast<Types *>(CurrentInst()->CastToInitObject());
                    break;
                default:
                    ASSERT(CurrentInst()->IsCall());
                    types = static_cast<Types *>(static_cast<CallInst *>(CurrentInst()));
                    break;
            }

            instInputsMap_[CurrentInstIndex()].reserve(inputs.size());
            types->AllocateInputTypes(graph_->GetAllocator(), inputs.size());
            for (const auto &input : inputs) {
                types->AddInputType(static_cast<DataType::Type>(input.first));
                instInputsMap_[CurrentInstIndex()].push_back(input.second);
            }
        }
        return *this;
    }

    /**
     * Same as the default Inputs() method, but defines inputs' types with respect to call-instructions.
     * Copies types of inputs to the instruction's input_types_.
     */
    template <typename... Args>
    IrConstructor &InputsAutoType(Args... inputs)
    {
        using Types = InputTypesMixin<DynamicInputsInst>;
        Types *types;
        switch (CurrentInst()->GetOpcode()) {
            case Opcode::Intrinsic:
                types = static_cast<Types *>(CurrentInst()->CastToIntrinsic());
                break;
            case Opcode::MultiArray:
                types = static_cast<Types *>(CurrentInst()->CastToMultiArray());
                break;
            default:
                ASSERT(CurrentInst()->IsCall());
                types = static_cast<Types *>(static_cast<CallInst *>(CurrentInst()));
                break;
        }
        types->AllocateInputTypes(graph_->GetAllocator(), sizeof...(inputs));
        ((types->AddInputType(GetInst(inputs).GetType())), ...);
        instInputsMap_[CurrentInstIndex()].reserve(sizeof...(inputs));
        ((instInputsMap_[CurrentInstIndex()].push_back(inputs)), ...);
        return *this;
    }

    IrConstructor &Pc(uint32_t pc)
    {
        ASSERT(CurrentInst());
        CurrentInst()->SetPc(pc);
        return *this;
    }

    IrConstructor &Volatile(bool volat = true)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::StoreObject:
                inst->CastToStoreObject()->SetVolatile(volat);
                break;
            case Opcode::LoadObject:
                inst->CastToLoadObject()->SetVolatile(volat);
                break;
            case Opcode::StoreStatic:
                inst->CastToStoreStatic()->SetVolatile(volat);
                break;
            case Opcode::LoadStatic:
                inst->CastToLoadStatic()->SetVolatile(volat);
                break;
            case Opcode::Store:
                inst->CastToStore()->SetVolatile(volat);
                break;
            case Opcode::Load:
                inst->CastToLoad()->SetVolatile(volat);
                break;
            case Opcode::StoreI:
                inst->CastToStoreI()->SetVolatile(volat);
                break;
            case Opcode::LoadI:
                inst->CastToLoadI()->SetVolatile(volat);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &Likely()
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::If:
                ASSERT(!inst->CastToIf()->IsUnlikely());
                inst->CastToIf()->SetLikely();
                break;
            case Opcode::IfImm:
                ASSERT(!inst->CastToIfImm()->IsUnlikely());
                inst->CastToIfImm()->SetLikely();
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &Unlikely()
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::If:
                ASSERT(!inst->CastToIf()->IsLikely());
                inst->CastToIf()->SetUnlikely();
                break;
            case Opcode::IfImm:
                ASSERT(!inst->CastToIfImm()->IsLikely());
                inst->CastToIfImm()->SetUnlikely();
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &IsArray(bool value)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::LoadArray:
                inst->CastToLoadArray()->SetIsArray(value);
                break;
            case Opcode::LenArray:
                inst->CastToLenArray()->SetIsArray(value);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &CC(ConditionCode cc)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::Compare:
                inst->CastToCompare()->SetCc(cc);
                break;
            case Opcode::If:
                inst->CastToIf()->SetCc(cc);
                break;
            case Opcode::AddOverflow:
                inst->CastToAddOverflow()->SetCc(cc);
                break;
            case Opcode::SubOverflow:
                inst->CastToSubOverflow()->SetCc(cc);
                break;
            case Opcode::IfImm:
                inst->CastToIfImm()->SetCc(cc);
                break;
            case Opcode::Select:
                inst->CastToSelect()->SetCc(cc);
                break;
            case Opcode::SelectImm:
                inst->CastToSelectImm()->SetCc(cc);
                break;
            case Opcode::SelectTransform:
                inst->CastToSelectTransform()->SetCc(cc);
                break;
            case Opcode::SelectImmTransform:
                inst->CastToSelectImmTransform()->SetCc(cc);
                break;
            case Opcode::DeoptimizeCompare:
                inst->CastToDeoptimizeCompare()->SetCc(cc);
                break;
            case Opcode::DeoptimizeCompareImm:
                inst->CastToDeoptimizeCompareImm()->SetCc(cc);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &SelectTransform(SelectTransformType type)
    {
        auto *inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::SelectTransform:
                inst->CastToSelectTransform()->SetSelectTransformType(type);
                break;
            case Opcode::SelectImmTransform:
                inst->CastToSelectImmTransform()->SetSelectTransformType(type);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &SetFlag(compiler::inst_flags::Flags flag)
    {
        CurrentInst()->SetFlag(flag);
        return *this;
    }

    IrConstructor &ClearFlag(compiler::inst_flags::Flags flag)
    {
        CurrentInst()->ClearFlag(flag);
        return *this;
    }

    IrConstructor &Inlined()
    {
        auto inst = CurrentInst();
        if (inst->GetOpcode() == Opcode::Intrinsic) {
            inst->CastToIntrinsic()->SetInlined(true);
            return *this;
        }

        ASSERT(inst->GetOpcode() == Opcode::CallStatic || inst->GetOpcode() == Opcode::CallVirtual ||
               inst->GetOpcode() == Opcode::CallDynamic);
        static_cast<CallInst *>(inst)->SetInlined(true);
        inst->SetFlag(inst_flags::NO_DST);
        return *this;
    }

    IrConstructor &Caller(unsigned index)
    {
        auto inst = CurrentInst();
        ASSERT(inst->GetOpcode() == Opcode::SaveState);
        auto callInst = &GetInst(index);
        ASSERT(callInst->GetOpcode() == Opcode::CallStatic || callInst->GetOpcode() == Opcode::CallVirtual ||
               callInst->GetOpcode() == Opcode::CallDynamic);
        inst->CastToSaveState()->SetCallerInst(static_cast<CallInst *>(callInst));
        return *this;
    }

    IrConstructor &Scale(uint64_t scale)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::Load:
                inst->CastToLoad()->SetScale(scale);
                break;
            case Opcode::Store:
                inst->CastToStore()->SetScale(scale);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    // CC-OFFNXT(huge_method, G.FUN.01) big switch-case
    IrConstructor &Imm(uint64_t imm)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::AddI:
            case Opcode::SubI:
            case Opcode::MulI:
            case Opcode::DivI:
            case Opcode::ModI:
            case Opcode::ShlI:
            case Opcode::ShrI:
            case Opcode::AShrI:
            case Opcode::AndI:
            case Opcode::OrI:
            case Opcode::XorI:
                static_cast<BinaryImmOperation *>(inst)->SetImm(imm);
                break;
            case Opcode::BoundsCheckI:
                inst->CastToBoundsCheckI()->SetImm(imm);
                break;
            case Opcode::LoadArrayI:
                inst->CastToLoadArrayI()->SetImm(imm);
                break;
            case Opcode::StoreArrayI:
                inst->CastToStoreArrayI()->SetImm(imm);
                break;
            case Opcode::LoadI:
                inst->CastToLoadI()->SetImm(imm);
                break;
            case Opcode::StoreI:
                inst->CastToStoreI()->SetImm(imm);
                break;
            case Opcode::ReturnI:
                inst->CastToReturnI()->SetImm(imm);
                break;
            case Opcode::IfImm:
                inst->CastToIfImm()->SetImm(imm);
                break;
            case Opcode::SelectImm:
                inst->CastToSelectImm()->SetImm(imm);
                break;
            case Opcode::SelectImmTransform:
                inst->CastToSelectImmTransform()->SetImm(imm);
                break;
            case Opcode::LoadArrayPairI:
                inst->CastToLoadArrayPairI()->SetImm(imm);
                break;
            case Opcode::StoreArrayPairI:
                inst->CastToStoreArrayPairI()->SetImm(imm);
                break;
            case Opcode::LoadPairPart:
                inst->CastToLoadPairPart()->SetImm(imm);
                break;
            case Opcode::DeoptimizeCompareImm:
                inst->CastToDeoptimizeCompareImm()->SetImm(imm);
                break;
            case Opcode::LoadArrayPair:
                inst->CastToLoadArrayPair()->SetImm(imm);
                break;
            case Opcode::StoreArrayPair:
                inst->CastToStoreArrayPair()->SetImm(imm);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &Shift(ShiftType shiftType, uint64_t imm)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::AndSR:
            case Opcode::OrSR:
            case Opcode::XorSR:
            case Opcode::AndNotSR:
            case Opcode::OrNotSR:
            case Opcode::XorNotSR:
            case Opcode::AddSR:
            case Opcode::SubSR:
                static_cast<BinaryShiftedRegisterOperation *>(inst)->SetShiftType(shiftType);
                static_cast<BinaryShiftedRegisterOperation *>(inst)->SetImm(imm);
                break;
            case Opcode::NegSR:
                static_cast<UnaryShiftedRegisterOperation *>(inst)->SetShiftType(shiftType);
                static_cast<UnaryShiftedRegisterOperation *>(inst)->SetImm(imm);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    };

    IrConstructor &Exit()
    {
        CurrentInst()->CastToMonitor()->SetExit();
        return *this;
    }

    IrConstructor &Entry()
    {
        CurrentInst()->CastToMonitor()->SetEntry();
        return *this;
    }

    IrConstructor &Fcmpg(bool fcmpg = true)
    {
        CurrentInst()->CastToCmp()->SetFcmpg(fcmpg);
        return *this;
    }

    IrConstructor &Fcmpl(bool fcmpl = true)
    {
        CurrentInst()->CastToCmp()->SetFcmpl(fcmpl);
        return *this;
    }

    IrConstructor &u8()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::UINT8);
        return *this;
    }
    IrConstructor &u16()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::UINT16);
        return *this;
    }
    IrConstructor &u32()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::UINT32);
        return *this;
    }
    IrConstructor &u64()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::UINT64);
        return *this;
    }
    IrConstructor &s8()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::INT8);
        return *this;
    }
    IrConstructor &s16()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::INT16);
        return *this;
    }
    IrConstructor &s32()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::INT32);
        return *this;
    }
    IrConstructor &s64()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::INT64);
        return *this;
    }
    IrConstructor &i8()  // NOLINT(readability-identifier-naming)
    {
        return s8();
    }
    IrConstructor &i16()  // NOLINT(readability-identifier-naming)
    {
        return s16();
    }
    IrConstructor &i32()  // NOLINT(readability-identifier-naming)
    {
        return s32();
    }
    IrConstructor &i64()  // NOLINT(readability-identifier-naming)
    {
        return s64();
    }
    IrConstructor &b()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::BOOL);
        return *this;
    }
    IrConstructor &ref()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::REFERENCE);
        return *this;
    }
    IrConstructor &ptr()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::POINTER);
        return *this;
    }
    IrConstructor &w()  // NOLINT(readability-identifier-naming)
    {
        return ptr();
    }
    // Type representing MarkWord
    IrConstructor &mw()  // NOLINT(readability-identifier-naming)
    {
        return type(MARK_WORD_TYPE);
    }
    // Type representing uint type for ref
    IrConstructor &ref_uint()  // NOLINT(readability-identifier-naming)
    {
        return type(DataType::GetIntTypeForReference(graph_->GetArch()));
    }
    IrConstructor &f32()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::FLOAT32);
        return *this;
    }
    IrConstructor &f64()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::FLOAT64);
        return *this;
    }
    IrConstructor &any()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::ANY);
        return *this;
    }
    IrConstructor &SetType(DataType::Type type)  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(type);
        return *this;
    }
    IrConstructor &AnyType(AnyBaseType anyType)
    {
        auto *atm = static_cast<AnyTypeMixin<FixedInputsInst1> *>(CurrentInst());
        atm->SetAnyType(anyType);
        return *this;
    }
    IrConstructor &v0id()  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(DataType::VOID);
        return *this;
    }
    IrConstructor &type(DataType::Type type)  // NOLINT(readability-identifier-naming)
    {
        CurrentInst()->SetType(type);
        return *this;
    }

    IrConstructor &Terminator()
    {
        CurrentInst()->SetFlag(inst_flags::TERMINATOR);
        return *this;
    }

    IrConstructor &AddImm(uint32_t imm)
    {
        CurrentInst()->CastToIntrinsic()->AddImm(graph_->GetAllocator(), imm);
        return *this;
    }

    IrConstructor &DstReg(uint8_t reg)
    {
        CurrentInst()->SetDstReg(reg);
        if (DataType::IsFloatType(CurrentInst()->GetType())) {
            graph_->SetUsedReg<DataType::FLOAT64>(reg);
        }
        return *this;
    }

    IrConstructor &SrcReg(uint8_t id, uint8_t reg)
    {
        CurrentInst()->SetSrcReg(id, reg);
        if (DataType::IsFloatType(CurrentInst()->GetType())) {
            graph_->SetUsedReg<DataType::FLOAT64>(reg);
        }
        graph_->SetUsedReg<DataType::INT64>(reg);
        return *this;
    }

    // CC-OFFNXT(huge_method, huge_cyclomatic_complexity, G.FUN.01) big switch-case
    IrConstructor &TypeId(uint32_t typeId)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::Call:
                inst->CastToCall()->SetCallMethodId(typeId);
                break;
            case Opcode::LoadString:
                inst->CastToLoadString()->SetTypeId(typeId);
                break;
            case Opcode::LoadType:
                inst->CastToLoadType()->SetTypeId(typeId);
                break;
            case Opcode::UnresolvedLoadType:
                inst->CastToUnresolvedLoadType()->SetTypeId(typeId);
                break;
            case Opcode::LoadFromConstantPool:
                inst->CastToLoadFromConstantPool()->SetTypeId(typeId);
                break;
            case Opcode::StoreStatic:
                inst->CastToStoreStatic()->SetTypeId(typeId);
                break;
            case Opcode::UnresolvedStoreStatic:
                inst->CastToUnresolvedStoreStatic()->SetTypeId(typeId);
                break;
            case Opcode::ResolveObjectField:
                inst->CastToResolveObjectField()->SetTypeId(typeId);
                break;
            case Opcode::ResolveObjectFieldStatic:
                inst->CastToResolveObjectFieldStatic()->SetTypeId(typeId);
                break;
            case Opcode::StoreResolvedObjectField:
                inst->CastToStoreResolvedObjectField()->SetTypeId(typeId);
                break;
            case Opcode::StoreResolvedObjectFieldStatic:
                inst->CastToStoreResolvedObjectFieldStatic()->SetTypeId(typeId);
                break;
            case Opcode::LoadStatic:
                inst->CastToLoadStatic()->SetTypeId(typeId);
                break;
            case Opcode::LoadObject:
                inst->CastToLoadObject()->SetTypeId(typeId);
                break;
            case Opcode::StoreObject:
                inst->CastToStoreObject()->SetTypeId(typeId);
                break;
            case Opcode::NewObject:
                inst->CastToNewObject()->SetTypeId(typeId);
                break;
            case Opcode::InitObject:
                inst->CastToInitObject()->SetCallMethodId(typeId);
                break;
            case Opcode::NewArray:
                inst->CastToNewArray()->SetTypeId(typeId);
                break;
            case Opcode::CheckCast:
                inst->CastToCheckCast()->SetTypeId(typeId);
                break;
            case Opcode::IsInstance:
                inst->CastToIsInstance()->SetTypeId(typeId);
                break;
            case Opcode::InitClass:
                inst->CastToInitClass()->SetTypeId(typeId);
                break;
            case Opcode::LoadClass:
                inst->CastToLoadClass()->SetTypeId(typeId);
                break;
            case Opcode::LoadAndInitClass:
                inst->CastToLoadAndInitClass()->SetTypeId(typeId);
                break;
            case Opcode::UnresolvedLoadAndInitClass:
                inst->CastToUnresolvedLoadAndInitClass()->SetTypeId(typeId);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &ObjField(RuntimeInterface::FieldPtr field)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::StoreStatic:
                inst->CastToStoreStatic()->SetObjField(field);
                break;
            case Opcode::LoadStatic:
                inst->CastToLoadStatic()->SetObjField(field);
                break;
            case Opcode::LoadObject:
                inst->CastToLoadObject()->SetObjField(field);
                break;
            case Opcode::StoreObject:
                inst->CastToStoreObject()->SetObjField(field);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &ObjectType(ObjectType objType)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::LoadObject:
                inst->CastToLoadObject()->SetObjectType(objType);
                break;
            case Opcode::StoreObject:
                inst->CastToStoreObject()->SetObjectType(objType);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &ObjectTypeLoadImm(LoadImmediateInst::ObjectType objType)
    {
        auto inst = CurrentInst();
        ASSERT(inst->GetOpcode() == Opcode::LoadImmediate);
        inst->CastToLoadImmediate()->SetObjectType(objType);
        return *this;
    }

    IrConstructor &SetChecks(HclassChecks checks)
    {
        auto inst = CurrentInst();
        ASSERT(inst->GetOpcode() == Opcode::HclassCheck);
        switch (checks) {
            case HclassChecks::ALL_CHECKS:
                inst->CastToHclassCheck()->SetCheckFunctionIsNotClassConstructor(true);
                inst->CastToHclassCheck()->SetCheckIsFunction(true);
                break;
            case HclassChecks::IS_FUNCTION:
                inst->CastToHclassCheck()->SetCheckIsFunction(true);
                break;
            case HclassChecks::IS_NOT_CLASS_CONSTRUCTOR:
                inst->CastToHclassCheck()->SetCheckFunctionIsNotClassConstructor(true);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &SetNeedBarrier(bool needBarrier)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::Store:
                inst->CastToStore()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::StoreI:
                inst->CastToStoreI()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::StoreObject:
                inst->CastToStoreObject()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::StoreArray:
                inst->CastToStoreArray()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::StoreArrayI:
                inst->CastToStoreArrayI()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::StoreArrayPair:
                inst->CastToStoreArrayPair()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::StoreArrayPairI:
                inst->CastToStoreArrayPairI()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::Load:
                inst->CastToLoad()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadI:
                inst->CastToLoadI()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadObject:
                inst->CastToLoadObject()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadObjectPair:
                inst->CastToLoadObjectPair()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadResolvedObjectField:
                inst->CastToLoadResolvedObjectField()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadResolvedObjectFieldStatic:
                inst->CastToLoadResolvedObjectFieldStatic()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadArray:
                inst->CastToLoadArray()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadArrayI:
                inst->CastToLoadArrayI()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadArrayPair:
                inst->CastToLoadArrayPair()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadArrayPairI:
                inst->CastToLoadArrayPairI()->SetNeedBarrier(needBarrier);
                break;
            case Opcode::LoadStatic:
                inst->CastToLoadStatic()->SetNeedBarrier(needBarrier);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &SrcVregs(std::vector<int> &&vregs)
    {
        ASSERT(CurrentInst()->IsSaveState());
        if (!vregs.empty()) {
            graph_->SetVRegsCount(
                std::max<size_t>(graph_->GetVRegsCount(), *std::max_element(vregs.begin(), vregs.end())));
        }
        if (saveStateInstVregsMap_.count(CurrentInstIndex()) == 0) {
            saveStateInstVregsMap_.emplace(CurrentInstIndex(), std::move(vregs));
        }
        return *this;
    }

    IrConstructor &NoVregs()
    {
        ASSERT(CurrentInst()->IsSaveState());
        return *this;
    }

    // Useful for parametrized tests
    IrConstructor &CleanupInputs()
    {
        ASSERT(CurrentInst()->IsSaveState());
        auto &inputs = instInputsMap_[CurrentInstIndex()];
        auto &vregs = saveStateInstVregsMap_[CurrentInstIndex()];
        for (auto i = static_cast<int>(inputs.size()) - 1; i >= 0; i--) {
            if (instMap_.count(inputs[i]) == 0) {
                inputs.erase(inputs.begin() + i);
                vregs.erase(vregs.begin() + i);
            }
        }
        return *this;
    }

    IrConstructor &CatchTypeIds(std::vector<uint16_t> &&ids)
    {
        auto inst = CurrentInst();
        ASSERT(inst->GetOpcode() == Opcode::Try);
        auto tryInst = inst->CastToTry();
        for (auto id : ids) {
            tryInst->AppendCatchTypeId(id, 0);
        }
        return *this;
    }

    IrConstructor &ThrowableInsts(std::vector<int> &&ids)
    {
        auto inst = CurrentInst();
        ASSERT(inst->GetOpcode() == Opcode::CatchPhi);
        auto catchPhi = inst->CastToCatchPhi();
        for (auto id : ids) {
            ASSERT(instMap_.count(id) > 0);
            catchPhi->AppendThrowableInst(instMap_.at(id));
        }
        return *this;
    }

    IrConstructor &DeoptimizeType(DeoptimizeType type)
    {
        auto inst = CurrentInst();
        if (inst->GetOpcode() == Opcode::Deoptimize) {
            inst->CastToDeoptimize()->SetDeoptimizeType(type);
        } else {
            ASSERT(inst->GetOpcode() == Opcode::DeoptimizeIf);
            inst->CastToDeoptimizeIf()->SetDeoptimizeType(type);
        }
        return *this;
    }

    IrConstructor &SrcType(DataType::Type type)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::Compare:
                inst->CastToCompare()->SetOperandsType(type);
                break;
            case Opcode::Cmp:
                inst->CastToCmp()->SetOperandsType(type);
                break;
            case Opcode::If:
                inst->CastToIf()->SetOperandsType(type);
                break;
            case Opcode::AddOverflow:
                inst->CastToAddOverflow()->SetOperandsType(type);
                break;
            case Opcode::SubOverflow:
                inst->CastToSubOverflow()->SetOperandsType(type);
                break;
            case Opcode::Select:
                inst->CastToSelect()->SetOperandsType(type);
                break;
            case Opcode::SelectImm:
                inst->CastToSelectImm()->SetOperandsType(type);
                break;
            case Opcode::SelectTransform:
                inst->CastToSelectTransform()->SetOperandsType(type);
                break;
            case Opcode::SelectImmTransform:
                inst->CastToSelectImmTransform()->SetOperandsType(type);
                break;
            case Opcode::IfImm:
                inst->CastToIfImm()->SetOperandsType(type);
                break;
            case Opcode::Cast:
                inst->CastToCast()->SetOperandsType(type);
                break;
            case Opcode::Bitcast:
                inst->CastToBitcast()->SetOperandsType(type);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &IntrinsicId(RuntimeInterface::IntrinsicId id)
    {
        auto inst = CurrentInst();
        ASSERT(inst->IsIntrinsic());
        inst->CastToIntrinsic()->SetIntrinsicId(id);
        return *this;
    }

    IrConstructor &Relocate()
    {
        auto inst = CurrentInst();
        ASSERT(inst->IsIntrinsic());
        inst->CastToIntrinsic()->SetRelocate();
        return *this;
    }

    IrConstructor &Class(RuntimeInterface::ClassPtr klass)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::InitClass:
                inst->CastToInitClass()->SetClass(klass);
                break;
            case Opcode::LoadClass:
                inst->CastToLoadClass()->SetClass(klass);
                break;
            case Opcode::LoadAndInitClass:
                inst->CastToLoadAndInitClass()->SetClass(klass);
                break;
            case Opcode::UnresolvedLoadAndInitClass:
                inst->CastToUnresolvedLoadAndInitClass()->SetClass(klass);
                break;
            case Opcode::LoadImmediate:
                inst->CastToLoadImmediate()->SetObjectType(LoadImmediateInst::ObjectType::CLASS);
                inst->CastToLoadImmediate()->SetClass(klass);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &IntegerWasSeen(bool value)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::CastAnyTypeValue:
                inst->CastToCastAnyTypeValue()->SetIsIntegerWasSeen(value);
                break;
            case Opcode::AnyTypeCheck:
                inst->CastToAnyTypeCheck()->SetIsIntegerWasSeen(value);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &AllowedInputType(profiling::AnyInputType type)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::CastAnyTypeValue:
                inst->CastToCastAnyTypeValue()->SetAllowedInputType(type);
                break;
            case Opcode::AnyTypeCheck:
                inst->CastToAnyTypeCheck()->SetAllowedInputType(type);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &Loc([[maybe_unused]] uint32_t dirNumber, [[maybe_unused]] uint32_t fileNumber,
                       [[maybe_unused]] uint32_t lineNumber)
    {
#ifdef PANDA_COMPILER_DEBUG_INFO
        auto debugInfo = graph_->GetAllocator()->New<InstDebugInfo>(dirNumber, fileNumber, lineNumber);
        CurrentInst()->SetDebugInfo(debugInfo);
#endif
        return *this;
    }

    IrConstructor &SetAccessType(DynObjectAccessType type)
    {
        auto *inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::LoadObjectDynamic:
                inst->CastToLoadObjectDynamic()->SetAccessType(type);
                break;
            case Opcode::StoreObjectDynamic:
                inst->CastToStoreObjectDynamic()->SetAccessType(type);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &SetPtr(uintptr_t ptr)
    {
        auto *inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::LoadObjFromConst:
                inst->CastToLoadObjFromConst()->SetObjPtr(ptr);
                break;
            case Opcode::FunctionImmediate:
                inst->CastToFunctionImmediate()->SetFunctionPtr(ptr);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &SetAccessMode(DynObjectAccessMode mode)
    {
        auto *inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::LoadObjectDynamic:
                inst->CastToLoadObjectDynamic()->SetAccessMode(mode);
                break;
            case Opcode::StoreObjectDynamic:
                inst->CastToStoreObjectDynamic()->SetAccessMode(mode);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    template <typename T>
    std::shared_ptr<IrConstructor> ScopedLife()
    {
#ifndef __clang_analyzer__
        if constexpr (std::is_same_v<T, BasicBlock>) {
            return std::shared_ptr<IrConstructor>(this, [](IrConstructor *b) { b->ResetCurrentBb(); });
        } else if constexpr (std::is_same_v<T, Inst>) {
            return std::shared_ptr<IrConstructor>(this, [](IrConstructor *b) { b->ResetCurrentInst(); });
        } else if constexpr (std::is_same_v<T, Graph>) {
            return std::shared_ptr<IrConstructor>(this, [](IrConstructor *b) { b->Finalize(); });
        }
#else
        return nullptr;
#endif
    }

    void CheckInputType(Inst *inst, Inst *inputInst, size_t inputIdx)
    {
        auto type = inputInst->GetType();
        auto prevType = inst->GetInputType(inputIdx);
        if (prevType == DataType::Type::NO_TYPE) {
            switch (inst->GetOpcode()) {
                case Opcode::Cmp:
                    inst->CastToCmp()->SetOperandsType(type);
                    break;
                case Opcode::Compare:
                    inst->CastToCompare()->SetOperandsType(type);
                    break;
                case Opcode::If:
                    inst->CastToIf()->SetOperandsType(type);
                    break;
                case Opcode::IfImm:
                    inst->CastToIfImm()->SetOperandsType(type);
                    break;
                case Opcode::Select:
                    inst->CastToSelect()->SetOperandsType(type);
                    break;
                case Opcode::SelectImm:
                    inst->CastToSelectImm()->SetOperandsType(type);
                    break;
                default:
                    UNREACHABLE();
            }
        } else {
            // Disable check for Negation (Neg+Add), we can't add deeper verification,
            // because the graph has not been built yet
            if (inputInst->GetOpcode() == Opcode::Add) {
                return;
            }
            CHECK_EQ(type, prevType);
        }
    }

    void ConstructControlFlow()
    {
        for (auto [bbi, succs] : bbSuccsMap_) {
            auto bb = bbMap_.at(bbi);
            for (auto succ : succs) {
                bb->AddSucc(bbMap_.at(succ == -1 ? 1 : succ));
            }
            if (succs.size() > 1 && bb->IsEmpty()) {
                bb->SetTryEnd(true);
            }
        }
        auto endBlock = graph_->GetEndBlock();
        if (endBlock->GetPredsBlocks().empty()) {
            graph_->EraseBlock(endBlock);
            graph_->SetEndBlock(nullptr);
        }
    }

    void ConstructInput(Inst *inst, size_t inputIdx, const std::vector<int> &vregs, size_t idx)
    {
        ASSERT_DO(instMap_.find(inputIdx) != instMap_.end(),
                  std::cerr << "Input with Id " << inputIdx << " isn't found, inst: " << *inst << std::endl);
        auto inputInst = instMap_.at(inputIdx);
        auto op = inst->GetOpcode();
        if (!inputInst->IsConst() && (op == Opcode::Cmp || op == Opcode::Compare || op == Opcode::If ||
                                      op == Opcode::IfImm || op == Opcode::Select || op == Opcode::SelectImm)) {
            CheckInputType(inst, inputInst, idx);
        }
        if (inst->IsOperandsDynamic()) {
            inst->AppendInput(inputInst);
            if (inst->IsSaveState()) {
                static_cast<SaveStateInst *>(inst)->SetVirtualRegister(idx,
                                                                       VirtualRegister(vregs[idx], VRegType::VREG));
            }
        } else {
            inst->SetInput(idx, inputInst);
        }
    }

    void ConstructDataFlow()
    {
        for (auto [insti, inputs] : instInputsMap_) {
            auto inst = instMap_.at(insti);
            const auto &vregs {inst->IsSaveState() ? saveStateInstVregsMap_[insti] : std::vector<int> {}};
            ASSERT(!inst->IsSaveState() || inputs.size() == vregs.size());
            size_t idx = 0;
            if (inst->IsOperandsDynamic()) {
                inst->ReserveInputs(inputs.size());
            }
            for (auto inputIdx : inputs) {
                ConstructInput(inst, inputIdx, vregs, idx);
                ++idx;
            }
        }

        for (auto [insti, inputs] : phiInstInputsMap_) {
            auto inst = instMap_.at(insti);
            for (auto input : inputs) {
                auto inputInst = instMap_.at(input.second);
                size_t idx = inst->GetBasicBlock()->GetPredBlockIndex(bbMap_.at(input.first));
                auto i {inst->AppendInput(inputInst)};
                inst->CastToPhi()->SetPhiInputBbNum(i, idx);
            }
        }
    }

    void UpdateSpecialFlagsForReference(Inst *inst)
    {
        if (inst->GetType() == DataType::REFERENCE) {
            if (inst->GetOpcode() == Opcode::StoreArray) {
                inst->CastToStoreArray()->SetNeedBarrier(true);
            }
            if (inst->GetOpcode() == Opcode::StoreArrayI) {
                inst->CastToStoreArrayI()->SetNeedBarrier(true);
            }
            if (inst->GetOpcode() == Opcode::StoreStatic) {
                inst->CastToStoreStatic()->SetNeedBarrier(true);
            }
            if (inst->GetOpcode() == Opcode::UnresolvedStoreStatic) {
                inst->CastToUnresolvedStoreStatic()->SetNeedBarrier(true);
            }
            if (inst->GetOpcode() == Opcode::StoreObject) {
                inst->CastToStoreObject()->SetNeedBarrier(true);
            }
            if (inst->GetOpcode() == Opcode::StoreResolvedObjectField) {
                inst->CastToStoreResolvedObjectField()->SetNeedBarrier(true);
            }
            if (inst->GetOpcode() == Opcode::StoreResolvedObjectFieldStatic) {
                inst->CastToStoreResolvedObjectFieldStatic()->SetNeedBarrier(true);
            }
            if (inst->GetOpcode() == Opcode::StoreArrayPair) {
                inst->CastToStoreArrayPair()->SetNeedBarrier(true);
            }
            if (inst->GetOpcode() == Opcode::StoreArrayPairI) {
                inst->CastToStoreArrayPairI()->SetNeedBarrier(true);
            }
        }
    }

    void UpdateSpecialFlags()
    {
        size_t maxId = graph_->GetCurrentInstructionId();
        for (auto pair : instMap_) {
            auto id = static_cast<size_t>(pair.first);
            auto inst = pair.second;
            UpdateSpecialFlagsForReference(inst);
            if (inst->GetOpcode() == Opcode::Try) {
                auto bb = inst->GetBasicBlock();
                bb->SetTryBegin(true);
                bb->GetSuccessor(0)->SetTry(true);
                for (size_t idx = 1; idx < bb->GetSuccsBlocks().size(); idx++) {
                    bb->GetSuccessor(idx)->SetCatchBegin(true);
                }
            }
            if (inst->GetOpcode() == Opcode::SaveStateOsr) {
                inst->GetBasicBlock()->SetOsrEntry(true);
            }
            if (id >= maxId) {
                maxId = id + 1;
            }
        }
        graph_->SetCurrentInstructionId(maxId);
    }

    // Create SaveState instructions thet weren't explicitly constructed in the test
    void CreateSaveStates()
    {
        for (auto [insti, inputs] : instInputsMap_) {
            auto inst = instMap_.at(insti);
            if (!inst->IsOperandsDynamic() && inst->RequireState() && inst->GetInputsCount() > inputs.size()) {
                auto saveState = graph_->CreateInstSaveState();
                saveState->SetId(static_cast<int>(graph_->GetCurrentInstructionId()) + 1);
                graph_->SetCurrentInstructionId(saveState->GetId() + 1);
                inst->GetBasicBlock()->InsertBefore(saveState, inst);
                inst->SetSaveState(saveState);
            }
        }
    }

    void SetSpillFillData()
    {
        graph_->ResetParameterInfo();
        // Count number of parameters (needed for bytecode optimizer) in first cycle and set SpillFillData for each
        // parameter in second cycle
        uint32_t numArgs = 0;
        for (auto inst : graph_->GetStartBlock()->Insts()) {
            if (inst->GetOpcode() == Opcode::Parameter) {
                ++numArgs;
            }
        }
        uint32_t i = 0;
        for (auto inst : graph_->GetStartBlock()->Insts()) {
            if (inst->GetOpcode() != Opcode::Parameter) {
                continue;
            }

            auto type = inst->GetType();
            InstBuilder::SetParamSpillFill(graph_, static_cast<ParameterInst *>(inst), numArgs, i++, type);
        }
    }

    void Finalize()
    {
        ConstructControlFlow();
        ConstructDataFlow();
        UpdateSpecialFlags();
        CreateSaveStates();
        SetSpillFillData();
        ResetCurrentBb();
        ResetCurrentInst();
        graph_->RunPass<LoopAnalyzer>();
        PropagateRegisters();
        if (enableGraphChecker_) {
            GraphChecker(graph_).Check();
        }
    }

    Inst &GetInst(unsigned index)
    {
        ASSERT_DO(instMap_.find(index) != instMap_.end(), std::cerr << "Inst with Id " << index << " isn't found\n");
        return *instMap_.at(index);
    }

    BasicBlock &GetBlock(unsigned index)
    {
        return *bbMap_.at(index);
    }

    void EnableGraphChecker(bool value)
    {
        enableGraphChecker_ = value;
    }

private:
    template <typename T>
    void AddInput(T v)
    {
        static_assert(std::is_integral_v<T>);
        instInputsMap_[CurrentInstIndex()].push_back(v);
    }

    template <typename T, typename... Args>
    void AddInput(T v, Args... args)
    {
        instInputsMap_[CurrentInstIndex()].push_back(v);
        AddInput(args...);
    }

    BasicBlock *GetBbByIndex(int index)
    {
        return bbMap_.at(index);
    }

    BasicBlock *CurrentBb()
    {
        return currentBb_.second;
    }

    int CurrentBbIndex()
    {
        return currentBb_.first;
    }

    Inst *CurrentInst()
    {
        return currentInst_.second;
    }

    int CurrentInstIndex()
    {
        return currentInst_.first;
    }

    void ResetCurrentBb()
    {
        currentBb_ = {-1, nullptr};
    }

    void ResetCurrentInst()
    {
        currentInst_ = {-1, nullptr};
    }

    void PropagateRegisters()
    {
        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetDstReg() == GetInvalidReg() || inst->IsOperandsDynamic()) {
                    continue;
                }
                for (size_t i = 0; i < inst->GetInputsCount(); i++) {
                    inst->SetSrcReg(i, inst->GetInputs()[i].GetInst()->GetDstReg());
                }
            }
        }
    }

private:
    Graph *graph_ {nullptr};
    ArenaAllocator aa_;
    std::pair<int, BasicBlock *> currentBb_;
    std::pair<int, Inst *> currentInst_;
    ArenaUnorderedMap<int, BasicBlock *> bbMap_ {aa_.Adapter()};
    ArenaVector<std::pair<int, std::vector<int>>> bbSuccsMap_ {aa_.Adapter()};
    ArenaUnorderedMap<int, Inst *> instMap_ {aa_.Adapter()};
    ArenaUnorderedMap<int, std::vector<int>> instInputsMap_ {aa_.Adapter()};
    ArenaUnorderedMap<int, std::vector<int>> saveStateInstVregsMap_ {aa_.Adapter()};
    ArenaUnorderedMap<int, std::vector<std::pair<int, int>>> phiInstInputsMap_ {aa_.Adapter()};
    bool enableGraphChecker_ {true};
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GRAPH(GRAPH) if (auto __g = builder_->SetGraph(GRAPH).ScopedLife<Graph>(); true)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BASIC_BLOCK(ID, ...) \
    if (auto __b = builder_->NewBlock<ID>().Succs({__VA_ARGS__}).template ScopedLife<BasicBlock>(); true)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST(ID, ...) builder_->NewInst(ID, __VA_ARGS__)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CONSTANT(ID, VALUE) builder_->NewConstant(ID, VALUE)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PARAMETER(ID, ARG_NUM) builder_->NewParameter(ID, ARG_NUM)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define NULLPTR(ID) builder_->AddNullptrInst(ID)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INS(INDEX) builder_->GetInst(INDEX)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BB(INDEX) builder_->GetBlock(INDEX)
}  // namespace ark::compiler

#endif  // PANDA_IR_CONSTRUCTOR_H
