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

#ifndef COMPILER_TESTS_INST_GENERATOR_H
#define COMPILER_TESTS_INST_GENERATOR_H

#include "unit_test.h"

namespace panda::compiler {
class GraphCreator {
public:
    GraphCreator() = delete;
    explicit GraphCreator(ArenaAllocator &allocator, ArenaAllocator &local_allocator)
        : allocator_(allocator), local_allocator_(local_allocator)
    {
    }

    Graph *GenerateGraph(Inst *inst);

    void SetRuntimeTargetArch(Arch arch)
    {
        arch_ = arch;
    }

    RuntimeInterface *GetRuntime()
    {
        return &runtime_;
    }

    ArenaAllocator *GetAllocator() const
    {
        return &allocator_;
    }

    ArenaAllocator *GetLocalAllocator() const
    {
        return &local_allocator_;
    }

    void SetNumVRegsArgs(size_t regs, size_t args)
    {
        runtime_.vregs_count = regs;
        runtime_.args_count = args;
    }

private:
    Graph *CreateGraph();
    Graph *GenerateOperation(Inst *inst, int32_t n);
    Graph *GenerateCheckOperation(Inst *inst);
    Graph *GenerateSSOperation(Inst *inst);
    Graph *GenerateBoundaryCheckOperation(Inst *inst);
    Graph *GenerateThrowOperation(Inst *inst);
    Graph *GenerateMultiArrayOperation(Inst *inst);
    Graph *GeneratePhiOperation(Inst *inst);

    Graph *CreateGraphWithOneBasicBlock();
    Graph *CreateGraphWithTwoBasicBlock();
    Graph *CreateGraphWithThreeBasicBlock();
    Graph *CreateGraphWithFourBasicBlock();
    ParameterInst *CreateParamInst(Graph *graph, DataType::Type type, uint8_t slot);

    // need to create graphs
    ArenaAllocator &allocator_;
    ArenaAllocator &local_allocator_;
    RuntimeInterfaceMock runtime_;
    Arch arch_ {Arch::AARCH64};
#include "generate_operations_intrinsic_graph.inl"
};

class InstGenerator {
public:
    InstGenerator() = delete;
    explicit InstGenerator(ArenaAllocator &allocator) : allocator_(allocator) {}

    std::vector<Inst *> &Generate(Opcode OpCode);

    int GetAllPossibleInstToGenerateNumber()
    {
        int result = 0;
        for (auto &it : opcode_x_possible_types_) {
            result += it.second.size();
        }
        return result;
    }

    int GetPossibleInstToGenerateNumber(Opcode OpCode)
    {
        return opcode_x_possible_types_[OpCode].size();
    }

    std::map<Opcode, std::vector<DataType::Type>> &GetMap()
    {
        return opcode_x_possible_types_;
    }

    ArenaAllocator *GetAllocator()
    {
        return &allocator_;
    }

private:
    template <class T>
    std::vector<Inst *> &GenerateOperations(Opcode OpCode);

    template <class T>
    std::vector<Inst *> &GenerateOperationsImm(Opcode OpCode);

    template <class T>
    std::vector<Inst *> &GenerateOperationsShiftedRegister(Opcode OpCode);

    void GenerateIntrinsic(DataType::Type Type, RuntimeInterface::IntrinsicId intrinsic_id)
    {
        auto inst = Inst::New<IntrinsicInst>(&allocator_, Opcode::Intrinsic);
        inst->SetType(Type);
        inst->SetIntrinsicId(intrinsic_id);
        insts_.push_back(inst);
    }

    std::vector<DataType::Type> integer_types_ {DataType::UINT8,  DataType::INT8,  DataType::UINT16, DataType::INT16,
                                                DataType::UINT32, DataType::INT32, DataType::UINT64, DataType::INT64};

    std::vector<DataType::Type> numeric_types_ {DataType::BOOL,  DataType::UINT8,   DataType::INT8,   DataType::UINT16,
                                                DataType::INT16, DataType::UINT32,  DataType::INT32,  DataType::UINT64,
                                                DataType::INT64, DataType::FLOAT32, DataType::FLOAT64};

    std::vector<DataType::Type> ref_num_types_ {
        DataType::REFERENCE, DataType::BOOL,  DataType::UINT8,  DataType::INT8,  DataType::UINT16,  DataType::INT16,
        DataType::UINT32,    DataType::INT32, DataType::UINT64, DataType::INT64, DataType::FLOAT32, DataType::FLOAT64};

    std::vector<DataType::Type> ref_int_types_ {DataType::REFERENCE, DataType::BOOL,  DataType::UINT8,  DataType::INT8,
                                                DataType::UINT16,    DataType::INT16, DataType::UINT32, DataType::INT32,
                                                DataType::UINT64,    DataType::INT64};

    std::vector<DataType::Type> all_types_ {DataType::REFERENCE, DataType::BOOL,  DataType::UINT8,   DataType::INT8,
                                            DataType::UINT16,    DataType::INT16, DataType::UINT32,  DataType::INT32,
                                            DataType::UINT64,    DataType::INT64, DataType::FLOAT32, DataType::FLOAT64,
                                            DataType::VOID};

    std::vector<DataType::Type> floats_types_ {DataType::FLOAT32, DataType::FLOAT64};

    std::map<Opcode, std::vector<DataType::Type>> opcode_x_possible_types_ = {
        {Opcode::Neg, numeric_types_},
        {Opcode::Abs, numeric_types_},
        {Opcode::Not, integer_types_},
        {Opcode::Add, numeric_types_},
        {Opcode::Sub, numeric_types_},
        {Opcode::Mul, numeric_types_},
        {Opcode::Div, numeric_types_},
        {Opcode::Min, numeric_types_},
        {Opcode::Max, numeric_types_},
        {Opcode::Shl, integer_types_},
        {Opcode::Shr, integer_types_},
        {Opcode::AShr, integer_types_},
        {Opcode::Mod, numeric_types_},
        {Opcode::And, integer_types_},
        {Opcode::Or, integer_types_},
        {Opcode::Xor, integer_types_},
        {Opcode::Compare, ref_num_types_},
        {Opcode::If, ref_int_types_},
        {Opcode::Cmp, {DataType::INT32}},
        {Opcode::Constant, {DataType::INT64, DataType::FLOAT32, DataType::FLOAT64}},
        {Opcode::Phi, ref_num_types_},
        {Opcode::IfImm, ref_int_types_},
        {Opcode::Cast, numeric_types_},
        {Opcode::Parameter, ref_num_types_},
        {Opcode::IsInstance, {DataType::BOOL}},
        {Opcode::LenArray, {DataType::INT32}},
        {Opcode::LoadArray, ref_num_types_},
        {Opcode::StoreArray, ref_num_types_},
        {Opcode::LoadArrayI, ref_num_types_},
        {Opcode::StoreArrayI, ref_num_types_},
        {Opcode::CheckCast, {DataType::NO_TYPE}},
        {Opcode::NullCheck, {DataType::NO_TYPE}},
        {Opcode::ZeroCheck, {DataType::NO_TYPE}},
        {Opcode::NegativeCheck, {DataType::NO_TYPE}},
        {Opcode::BoundsCheck, {DataType::NO_TYPE}},
        {Opcode::BoundsCheckI, {DataType::NO_TYPE}},
        {Opcode::SaveState, {DataType::NO_TYPE}},
        {Opcode::ReturnVoid, {DataType::NO_TYPE}},
        {Opcode::Throw, {DataType::NO_TYPE}},
        {Opcode::NewArray, {DataType::REFERENCE}},
        {Opcode::Return, ref_num_types_},
        {Opcode::ReturnI, numeric_types_},
        {Opcode::CallStatic, all_types_},
        {Opcode::CallVirtual, all_types_},
        {Opcode::AddI, integer_types_},
        {Opcode::SubI, integer_types_},
        {Opcode::AndI, integer_types_},
        {Opcode::OrI, integer_types_},
        {Opcode::XorI, integer_types_},
        {Opcode::ShrI, integer_types_},
        {Opcode::ShlI, integer_types_},
        {Opcode::AShrI, integer_types_},
        {Opcode::SpillFill, {DataType::NO_TYPE}},
        {Opcode::NewObject, {DataType::REFERENCE}},
        {Opcode::LoadObject, ref_num_types_},
        {Opcode::LoadStatic, ref_num_types_},
        {Opcode::StoreObject, ref_num_types_},
        {Opcode::StoreStatic, ref_num_types_},
        {Opcode::LoadString, {DataType::REFERENCE}},
        {Opcode::LoadType, {DataType::REFERENCE}},
        {Opcode::SafePoint, {DataType::NO_TYPE}},
        {Opcode::ReturnInlined, {DataType::NO_TYPE}},
        {Opcode::Monitor, {DataType::VOID}},
        {Opcode::Intrinsic, {}},
        {Opcode::Select, ref_int_types_},
        {Opcode::SelectImm, ref_int_types_},
        {Opcode::NullPtr, {DataType::REFERENCE}},
        {Opcode::LoadArrayPair,
         {DataType::UINT32, DataType::INT32, DataType::UINT64, DataType::INT64, DataType::FLOAT32, DataType::FLOAT64,
          DataType::REFERENCE}},
        {Opcode::LoadArrayPairI,
         {DataType::UINT32, DataType::INT32, DataType::UINT64, DataType::INT64, DataType::FLOAT32, DataType::FLOAT64,
          DataType::REFERENCE}},
        {Opcode::StoreArrayPair,
         {DataType::UINT32, DataType::INT32, DataType::UINT64, DataType::INT64, DataType::FLOAT32, DataType::FLOAT64,
          DataType::REFERENCE}},
        {Opcode::StoreArrayPairI,
         {DataType::UINT32, DataType::INT32, DataType::UINT64, DataType::INT64, DataType::FLOAT32, DataType::FLOAT64,
          DataType::REFERENCE}},
        {Opcode::AndNot, integer_types_},
        {Opcode::OrNot, integer_types_},
        {Opcode::XorNot, integer_types_},
        {Opcode::MNeg, numeric_types_},
        {Opcode::MAdd, numeric_types_},
        {Opcode::MSub, numeric_types_},
        {Opcode::AddSR, integer_types_},
        {Opcode::SubSR, integer_types_},
        {Opcode::AndSR, integer_types_},
        {Opcode::OrSR, integer_types_},
        {Opcode::XorSR, integer_types_},
        {Opcode::AndNotSR, integer_types_},
        {Opcode::OrNotSR, integer_types_},
        {Opcode::XorNotSR, integer_types_},
        {Opcode::NegSR, integer_types_},
    };

    std::vector<ShiftType> only_shifts_ = {ShiftType::LSL, ShiftType::LSR, ShiftType::ASR};
    std::vector<ShiftType> shifts_and_rotation_ = {ShiftType::LSL, ShiftType::LSR, ShiftType::ASR, ShiftType::ROR};
    std::map<Opcode, std::vector<ShiftType>> opcode_x_possible_shift_types_ = {{Opcode::AddSR, only_shifts_},
                                                                               {Opcode::SubSR, only_shifts_},
                                                                               {Opcode::AndSR, shifts_and_rotation_},
                                                                               {Opcode::OrSR, shifts_and_rotation_},
                                                                               {Opcode::XorSR, shifts_and_rotation_},
                                                                               {Opcode::AndNotSR, shifts_and_rotation_},
                                                                               {Opcode::OrNotSR, shifts_and_rotation_},
                                                                               {Opcode::XorNotSR, shifts_and_rotation_},
                                                                               {Opcode::NegSR, only_shifts_}};
    std::vector<Inst *> insts_;

    // need to create graphs
    ArenaAllocator &allocator_;
};

class StatisticGenerator {
public:
    StatisticGenerator(InstGenerator &inst_generator, GraphCreator &graph_creator)
        : inst_generator_(inst_generator), graph_creator_(graph_creator)
    {
    }

    virtual ~StatisticGenerator() = default;

    typedef std::map<DataType::Type, int8_t> FULL_INST_STAT;
    typedef std::map<RuntimeInterface::IntrinsicId, bool> FULL_INTRINSIC_STAT;
    typedef std::pair<std::map<Opcode, FULL_INST_STAT>, FULL_INTRINSIC_STAT> FULL_STAT;

    virtual void Generate() = 0;

    FULL_STAT &GetStatistic()
    {
        return statistic_;
    }

    void GenerateHTMLPage(std::string file_name);

protected:
    InstGenerator &inst_generator_;
    GraphCreator &graph_creator_;

    int all_inst_number_ = 0;
    int positive_inst_number = 0;

    int all_opcode_number_ = 0;
    int implemented_opcode_number_ = 0;

    FULL_STAT statistic_;

    FULL_INST_STAT tmplt_ = {
        {DataType::NO_TYPE, -1}, {DataType::REFERENCE, -1}, {DataType::BOOL, -1},  {DataType::UINT8, -1},
        {DataType::INT8, -1},    {DataType::UINT16, -1},    {DataType::INT16, -1}, {DataType::UINT32, -1},
        {DataType::INT32, -1},   {DataType::UINT64, -1},    {DataType::INT64, -1}, {DataType::FLOAT32, -1},
        {DataType::FLOAT64, -1}, {DataType::ANY, -1},       {DataType::VOID, -1},
    };
};
}  // namespace panda::compiler

#endif  // COMPILER_TESTS_INST_GENERATOR_H
