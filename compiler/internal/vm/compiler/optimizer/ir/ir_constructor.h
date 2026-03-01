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

#ifndef COMPILER_OPTIMIZER_IR_IR_CONSTRUCTOR_H
#define COMPILER_OPTIMIZER_IR_IR_CONSTRUCTOR_H

#include <memory>
#include "graph.h"
#include "graph_checker.h"
#include "mark_word.h"
#include "optimizer/ir_builder/inst_builder.h"

namespace panda::compiler {
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

    static constexpr DataType::Type MARK_WORD_TYPE = DataType::GetIntTypeBySize(sizeof(MarkWord::markWordSize));

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
        bb_map_.clear();
        bb_map_[ID_ENTRY_BB] = graph_->GetStartBlock();
        bb_map_[ID_EXIT_BB] = graph_->GetEndBlock();
        bb_succs_map_.clear();
        inst_map_.clear();
        inst_inputs_map_.clear();
        save_state_inst_vregs_map_.clear();
        phi_inst_inputs_map_.clear();
        return *this;
    }

    template <size_t id>
    IrConstructor &NewBlock()
    {
        ASSERT(id != ID_ENTRY_BB && id != ID_EXIT_BB);
        ASSERT(bb_map_.count(id) == 0);
        ASSERT(CurrentBb() == nullptr);
        auto bb = graph_->GetAllocator()->New<BasicBlock>(graph_);
        bb->SetGuestPc(0U);
#ifdef NDEBUG
        graph_->AddBlock(bb);
#else
        graph_->AddBlock(bb, id);
#endif
        current_bb_ = {id, bb};
        bb_map_[id] = bb;
        // add connection the first custom block with entry
        if (bb_succs_map_.empty()) {
            graph_->GetStartBlock()->AddSucc(bb);
        }
        return *this;
    }

    template <typename... Args>
    IrConstructor &NewInst(size_t id, Args &&... args)
    {
        ASSERT_DO(inst_map_.find(id) == inst_map_.end(),
                  std::cerr << "Instruction with same Id " << id << "already exists");
        auto inst = graph_->CreateInst(std::forward<Args>(args)...);
        inst->SetId(id);
        for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
            inst->SetSrcReg(i, INVALID_REG);
        }
        current_inst_ = {id, inst};
        inst_map_[id] = inst;
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
        ASSERT_DO(inst_map_.find(id) == inst_map_.end(),
                  std::cerr << "Instruction with same Id " << id << "already exists");
        Inst *inst = nullptr;
        auto to_start_bb = (CurrentBbIndex() == 0) || (CurrentBbIndex() == -1);
        if (to_start_bb) {
            inst = graph_->FindOrCreateConstant(value);
        } else {
            inst = graph_->CreateInstConstant(value, graph_->IsBytecodeOptimizer());
            CurrentBb()->AppendInst(inst);
        }
        inst->SetId(id);
        inst_map_[id] = inst;
        current_inst_ = {id, inst};
        return *this;
    }

    IrConstructor &NewParameter(int id, uint16_t arg_number)
    {
        ASSERT_DO(inst_map_.find(id) == inst_map_.end(),
                  std::cerr << "Instruction with same Id " << id << "already exists");
        auto inst = graph_->AddNewParameter(arg_number);
        inst->SetId(id);
        inst_map_[id] = inst;
        current_inst_ = {id, inst};
        return *this;
    }

    IrConstructor &Succs(std::vector<int> succs)
    {
        bb_succs_map_.emplace_back(CurrentBbIndex(), std::move(succs));
        return *this;
    }

    /// Define inputs for current instruction.
    /// Input is an index of input instruction.
    template <typename... Args>
    IrConstructor &Inputs(Args... inputs)
    {
        ASSERT(!CurrentInst()->IsCall() && !CurrentInst()->IsIntrinsic());
        inst_inputs_map_[CurrentInstIndex()].reserve(sizeof...(inputs));
        if constexpr (sizeof...(inputs) != 0) {
            AddInput(inputs...);
        }
        return *this;
    }

    /// Define inputs for current call-, intrinsic-, or phi-instriction.
    /// Input is defined by std::pair.
    /// In case of phi: first is index of basic block, second is index of input instruction.
    /// In case of call and intrinsic: first is Type of input, second is index of input instruction.
    IrConstructor &Inputs(std::initializer_list<std::pair<int, int>> inputs)
    {
        ASSERT(CurrentInst()->IsPhi() || CurrentInst()->IsCall() || CurrentInst()->IsInitObject() ||
               CurrentInst()->IsIntrinsic());
        if (CurrentInst()->IsPhi()) {
            phi_inst_inputs_map_[CurrentInstIndex()].reserve(inputs.size());
            for (const auto &input : inputs) {
                phi_inst_inputs_map_[CurrentInstIndex()].push_back(input);
            }
        } else {
            auto opc = CurrentInst()->GetOpcode();
            InputTypesMixin *types;
            switch (opc) {
                case Opcode::Intrinsic:
                    types = static_cast<InputTypesMixin *>(CurrentInst()->CastToIntrinsic());
                    break;
                case Opcode::CallIndirect:
                    types = static_cast<InputTypesMixin *>(CurrentInst()->CastToCallIndirect());
                    break;
                case Opcode::Builtin:
                    types = static_cast<InputTypesMixin *>(CurrentInst()->CastToBuiltin());
                    break;
                case Opcode::InitObject:
                    types = static_cast<InputTypesMixin *>(CurrentInst()->CastToInitObject());
                    break;
                default:
                    ASSERT(CurrentInst()->IsCall());
                    types = static_cast<InputTypesMixin *>(static_cast<CallInst *>(CurrentInst()));
                    break;
            }

            inst_inputs_map_[CurrentInstIndex()].reserve(inputs.size());
            types->AllocateInputTypes(graph_->GetAllocator(), inputs.size());
            for (const auto &input : inputs) {
                types->AddInputType(static_cast<DataType::Type>(input.first));
                inst_inputs_map_[CurrentInstIndex()].push_back(input.second);
            }
        }
        return *this;
    }

    /// Same as the default Inputs() method, but defines inputs' types with respect to call-instructions.
    /// Copies types of inputs to the instruction's input_types_.
    template <typename... Args>
    IrConstructor &InputsAutoType(Args... inputs)
    {
        ASSERT(CurrentInst()->IsCall());
        auto *call_inst = static_cast<CallInst *>(CurrentInst());
        call_inst->AllocateInputTypes(graph_->GetAllocator(), sizeof...(inputs));
        ((call_inst->AddInputType(GetInst(inputs).GetType())), ...);
        inst_inputs_map_[CurrentInstIndex()].reserve(sizeof...(inputs));
        ((inst_inputs_map_[CurrentInstIndex()].push_back(inputs)), ...);
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
        ASSERT(inst->GetOpcode() == Opcode::CallStatic || inst->GetOpcode() == Opcode::CallVirtual);
        static_cast<CallInst *>(inst)->SetInlined(true);
        inst->SetFlag(inst_flags::NO_DST);
        return *this;
    }

    IrConstructor &Scale(uint64_t scale)
    {
        auto inst = CurrentInst();
        return *this;
    }

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
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &Shift(ShiftType shift_type, uint64_t imm)
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
                static_cast<BinaryShiftedRegisterOperation *>(inst)->SetShiftType(shift_type);
                static_cast<BinaryShiftedRegisterOperation *>(inst)->SetImm(imm);
                break;
            case Opcode::NegSR:
                static_cast<UnaryShiftedRegisterOperation *>(inst)->SetShiftType(shift_type);
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

    IrConstructor &Fcmpg(bool fcmpg)
    {
        CurrentInst()->CastToCmp()->SetFcmpg(fcmpg);
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
    IrConstructor &AnyType(AnyBaseType any_type)
    {
        auto *atm = static_cast<AnyTypeMixin<FixedInputsInst1> *>(CurrentInst());
        atm->SetAnyType(any_type);
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

    IrConstructor &TypeId(uint32_t type_id)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::Call:
                inst->CastToCall()->SetCallMethodId(type_id);
                break;
            case Opcode::LoadString:
                inst->CastToLoadString()->SetTypeId(type_id);
                break;
            case Opcode::LoadType:
                inst->CastToLoadType()->SetTypeId(type_id);
                break;
            case Opcode::UnresolvedLoadType:
                inst->CastToUnresolvedLoadType()->SetTypeId(type_id);
                break;
            case Opcode::StoreStatic:
                inst->CastToStoreStatic()->SetTypeId(type_id);
                break;
            case Opcode::UnresolvedStoreStatic:
                inst->CastToUnresolvedStoreStatic()->SetTypeId(type_id);
                break;
            case Opcode::LoadStatic:
                inst->CastToLoadStatic()->SetTypeId(type_id);
                break;
            case Opcode::UnresolvedLoadStatic:
                inst->CastToUnresolvedLoadStatic()->SetTypeId(type_id);
                break;
            case Opcode::LoadObject:
                inst->CastToLoadObject()->SetTypeId(type_id);
                break;
            case Opcode::UnresolvedLoadObject:
                inst->CastToUnresolvedLoadObject()->SetTypeId(type_id);
                break;
            case Opcode::StoreObject:
                inst->CastToStoreObject()->SetTypeId(type_id);
                break;
            case Opcode::UnresolvedStoreObject:
                inst->CastToUnresolvedStoreObject()->SetTypeId(type_id);
                break;
            case Opcode::NewObject:
                inst->CastToNewObject()->SetTypeId(type_id);
                break;
            case Opcode::InitObject:
                inst->CastToInitObject()->SetCallMethodId(type_id);
                break;
            case Opcode::NewArray:
                inst->CastToNewArray()->SetTypeId(type_id);
                break;
            case Opcode::CheckCast:
                inst->CastToCheckCast()->SetTypeId(type_id);
                break;
            case Opcode::IsInstance:
                inst->CastToIsInstance()->SetTypeId(type_id);
                break;
            case Opcode::InitClass:
                inst->CastToInitClass()->SetTypeId(type_id);
                break;
            case Opcode::LoadClass:
                inst->CastToLoadClass()->SetTypeId(type_id);
                break;
            case Opcode::LoadAndInitClass:
                inst->CastToLoadAndInitClass()->SetTypeId(type_id);
                break;
            case Opcode::UnresolvedLoadAndInitClass:
                inst->CastToUnresolvedLoadAndInitClass()->SetTypeId(type_id);
                break;
            default:
                UNREACHABLE();
        }
        return *this;
    }

    IrConstructor &ObjField(RuntimeInterface::FieldPtr field)
    {
        return *this;
    }

    IrConstructor &SetNeedBarrier(bool need_barrier)
    {
        auto inst = CurrentInst();
        switch (inst->GetOpcode()) {
            case Opcode::Store:
                inst->CastToStore()->SetNeedBarrier(need_barrier);
                break;
            case Opcode::StoreI:
                inst->CastToStoreI()->SetNeedBarrier(need_barrier);
                break;
            case Opcode::StoreObject:
                inst->CastToStoreObject()->SetNeedBarrier(need_barrier);
                break;
            case Opcode::StoreArray:
                inst->CastToStoreArray()->SetNeedBarrier(need_barrier);
                break;
            case Opcode::StoreArrayI:
                inst->CastToStoreArrayI()->SetNeedBarrier(need_barrier);
                break;
            case Opcode::StoreArrayPair:
                inst->CastToStoreArrayPair()->SetNeedBarrier(need_barrier);
                break;
            case Opcode::StoreArrayPairI:
                inst->CastToStoreArrayPairI()->SetNeedBarrier(need_barrier);
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
        if (save_state_inst_vregs_map_.count(CurrentInstIndex()) == 0) {
            save_state_inst_vregs_map_.emplace(CurrentInstIndex(), std::move(vregs));
        }
        return *this;
    }

    IrConstructor &NoVregs()
    {
        ASSERT(CurrentInst()->IsSaveState());
        return *this;
    }

    IrConstructor &CatchTypeIds(std::vector<uint16_t> &&ids)
    {
        auto inst = CurrentInst();
        ASSERT(inst->GetOpcode() == Opcode::Try);
        auto try_inst = inst->CastToTry();
        for (auto id : ids) {
            try_inst->AppendCatchTypeId(id, 0);
        }
        return *this;
    }

    IrConstructor &ThrowableInsts(std::vector<int> &&ids)
    {
        auto inst = CurrentInst();
        ASSERT(inst->GetOpcode() == Opcode::CatchPhi);
        auto catch_phi = inst->CastToCatchPhi();
        for (auto id : ids) {
            ASSERT(inst_map_.count(id) > 0);
            catch_phi->AppendThrowableInst(inst_map_.at(id));
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
        AdjustFlags(id, inst);
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

    void CheckInputType(Inst *inst, Inst *input_inst, size_t input_idx)
    {
        auto type = input_inst->GetType();
        auto prev_type = inst->GetInputType(input_idx);
        if (prev_type == DataType::Type::NO_TYPE) {
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
            CHECK_EQ(type, prev_type);
        }
    }

    void ConstructControlFlow()
    {
        for (auto [bbi, succs] : bb_succs_map_) {
            auto bb = bb_map_.at(bbi);
            for (auto succ : succs) {
                bb->AddSucc(bb_map_.at(succ == -1 ? 1 : succ));
            }
            if (succs.size() > 1 && bb->IsEmpty()) {
                bb->SetTryEnd(true);
            }
        }
        auto end_block = graph_->GetEndBlock();
        if (end_block->GetPredsBlocks().empty()) {
            graph_->EraseBlock(end_block);
            graph_->SetEndBlock(nullptr);
        }
    }

    void ConstructDataFlow()
    {
        for (auto [insti, inputs] : inst_inputs_map_) {
            auto inst = inst_map_.at(insti);
            const auto &vregs {inst->IsSaveState() ? save_state_inst_vregs_map_[insti] : std::vector<int> {}};
            ASSERT(!inst->IsSaveState() || inputs.size() == vregs.size());
            size_t idx = 0;
            if (inst->IsOperandsDynamic()) {
                inst->ReserveInputs(inputs.size());
            }
            for (auto input_idx : inputs) {
                ASSERT_DO(inst_map_.find(input_idx) != inst_map_.end(),
                          std::cerr << "Input with Id " << input_idx << " isn't found, inst: " << *inst << std::endl);
                auto input_inst = inst_map_.at(input_idx);
                auto op = inst->GetOpcode();
                if (!input_inst->IsConst() &&
                    (op == Opcode::Cmp || op == Opcode::Compare || op == Opcode::If || op == Opcode::IfImm ||
                     op == Opcode::Select || op == Opcode::SelectImm)) {
                    CheckInputType(inst, input_inst, idx);
                }
                if (inst->IsOperandsDynamic()) {
                    inst->AppendInput(input_inst);
                    if (inst->IsSaveState()) {
                        static_cast<SaveStateInst *>(inst)->SetVirtualRegister(idx, VirtualRegister(vregs[idx], false));
                    }
                } else {
                    inst->SetInput(idx, input_inst);
                }
                ++idx;
            }
        }

        for (auto [insti, inputs] : phi_inst_inputs_map_) {
            auto inst = inst_map_.at(insti);
            for (auto input : inputs) {
                auto input_inst = inst_map_.at(input.second);
                size_t idx = inst->GetBasicBlock()->GetPredBlockIndex(bb_map_.at(input.first));
                auto i {inst->AppendInput(input_inst)};
                inst->CastToPhi()->SetPhiInputBbNum(i, idx);
            }
        }
    }

    void UpdateSpecialFlags()
    {
        int max_id = graph_->GetCurrentInstructionId();
        for (auto pair : inst_map_) {
            auto id = pair.first;
            auto inst = pair.second;
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
                if (inst->GetOpcode() == Opcode::UnresolvedStoreObject) {
                    inst->CastToUnresolvedStoreObject()->SetNeedBarrier(true);
                }
                if (inst->GetOpcode() == Opcode::StoreArrayPair) {
                    inst->CastToStoreArrayPair()->SetNeedBarrier(true);
                }
                if (inst->GetOpcode() == Opcode::StoreArrayPairI) {
                    inst->CastToStoreArrayPairI()->SetNeedBarrier(true);
                }
            }
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
            if (id >= max_id) {
                max_id = id + 1;
            }
        }
        graph_->SetCurrentInstructionId(max_id);
    }

    // Create SaveState instructions thet weren't explicitly constructed in the test
    void CreateSaveStates()
    {
        for (auto [insti, inputs] : inst_inputs_map_) {
            auto inst = inst_map_.at(insti);
            if (!inst->IsOperandsDynamic() && inst->RequireState() && inst->GetInputsCount() > inputs.size()) {
                auto save_state = graph_->CreateInstSaveState();
                save_state->SetId(static_cast<int>(graph_->GetCurrentInstructionId()) + 1);
                graph_->SetCurrentInstructionId(save_state->GetId() + 1);
                inst->GetBasicBlock()->InsertBefore(save_state, inst);
                inst->SetSaveState(save_state);
            }
        }
    }

    void SetSpillFillData()
    {
        graph_->ResetParameterInfo();
        // Count number of parameters (needed for bytecode optimizer) in first cycle and set SpillFillData for each
        // parameter in second cycle
        uint32_t num_args = 0;
        for (auto inst : graph_->GetStartBlock()->Insts()) {
            if (inst->GetOpcode() == Opcode::Parameter) {
                ++num_args;
            }
        }
        uint32_t i = 0;
        for (auto inst : graph_->GetStartBlock()->Insts()) {
            if (inst->GetOpcode() != Opcode::Parameter) {
                continue;
            }
            ++i;

            auto type = inst->GetType();
            InstBuilder::SetParamSpillFill(graph_, static_cast<ParameterInst *>(inst), num_args, i - 1, type);
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
        if (enable_graph_checker_) {
            GraphChecker(graph_).Check();
        }
    }

    Inst &GetInst(unsigned index)
    {
        return *inst_map_.at(index);
    }

    BasicBlock &GetBlock(unsigned index)
    {
        return *bb_map_.at(index);
    }

    void EnableGraphChecker(bool value)
    {
        enable_graph_checker_ = value;
    }

private:
    void AddInput(int v)
    {
        inst_inputs_map_[CurrentInstIndex()].push_back(v);
    }

    template <typename T, typename... Args>
    void AddInput(T v, Args... args)
    {
        inst_inputs_map_[CurrentInstIndex()].push_back(v);
        AddInput(args...);
    }

    BasicBlock *GetBbByIndex(int index)
    {
        return bb_map_.at(index);
    }

    BasicBlock *CurrentBb()
    {
        return current_bb_.second;
    }

    int CurrentBbIndex()
    {
        return current_bb_.first;
    }

    Inst *CurrentInst()
    {
        return current_inst_.second;
    }

    int CurrentInstIndex()
    {
        return current_inst_.first;
    }

    void ResetCurrentBb()
    {
        current_bb_ = {-1, nullptr};
    }

    void ResetCurrentInst()
    {
        current_inst_ = {-1, nullptr};
    }

    void PropagateRegisters()
    {
        for (auto bb : graph_->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetDstReg() != INVALID_REG && !inst->IsOperandsDynamic()) {
                    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
                        inst->SetSrcReg(i, inst->GetInputs()[i].GetInst()->GetDstReg());
                    }
                }
            }
        }
    }

private:
    Graph *graph_ {nullptr};
    ArenaAllocator aa_;
    std::pair<int, BasicBlock *> current_bb_;
    std::pair<int, Inst *> current_inst_;
    ArenaUnorderedMap<int, BasicBlock *> bb_map_ {aa_.Adapter()};
    ArenaVector<std::pair<int, std::vector<int>>> bb_succs_map_ {aa_.Adapter()};
    ArenaUnorderedMap<int, Inst *> inst_map_ {aa_.Adapter()};
    ArenaUnorderedMap<int, std::vector<int>> inst_inputs_map_ {aa_.Adapter()};
    ArenaUnorderedMap<int, std::vector<int>> save_state_inst_vregs_map_ {aa_.Adapter()};
    ArenaUnorderedMap<int, std::vector<std::pair<int, int>>> phi_inst_inputs_map_ {aa_.Adapter()};
    bool enable_graph_checker_ {true};
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GRAPH(GRAPH) if (auto __g = builder_->SetGraph(GRAPH).ScopedLife<Graph>(); true)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BASIC_BLOCK(ID, ...) \
    if (auto __b = builder_->NewBlock<ID>().Succs({__VA_ARGS__}).ScopedLife<BasicBlock>(); true)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST(ID, ...) builder_->NewInst(ID, __VA_ARGS__)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CONSTANT(ID, VALUE) builder_->NewConstant(ID, VALUE)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PARAMETER(ID, ARG_NUM) builder_->NewParameter(ID, ARG_NUM)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INS(INDEX) builder_->GetInst(INDEX)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BB(INDEX) builder_->GetBlock(INDEX)
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_IR_CONSTRUCTOR_H
