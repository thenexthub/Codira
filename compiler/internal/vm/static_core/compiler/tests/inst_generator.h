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

#include "libarkbase/macros.h"
#include "unit_test.h"

namespace ark::compiler {
class GraphCreator {
public:
    GraphCreator() = delete;
    explicit GraphCreator(ArenaAllocator &allocator, ArenaAllocator &localAllocator)
        : allocator_(allocator), localAllocator_(localAllocator)
    {
    }

    Graph *GenerateGraph(Inst *inst);

    void SetRuntimeTargetArch(Arch arch)
    {
        arch_ = arch;
    }

    Arch GetRuntimeTargetArch() const
    {
        return arch_;
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
        return &localAllocator_;
    }

private:
    Graph *CreateGraph();

    Inst *PopulateLoadArrayPair(Graph *graph, BasicBlock *block, Inst *inst, Opcode opc);
    void PopulateStoreArrayPair(Graph *graph, Inst *inst, Opcode opc);
    void PopulateReturnInlined(Graph *graph, BasicBlock *block, Inst *inst, int32_t n);
    void PopulateCall(Graph *graph, BasicBlock *block, Inst *inst, DataType::Type type, int32_t n);
    void PopulateLoadStoreArray(Graph *graph, Inst *inst, DataType::Type type, int32_t n);
    void PopulateLoadStoreArrayI(Graph *graph, Inst *inst, DataType::Type type, int32_t n);
    void PopulateSelect(Graph *graph, Inst *inst, DataType::Type type, int32_t n);
    void PopulateSelectI(Graph *graph, Inst *inst, DataType::Type type, int32_t n);
    void PopulateStoreStatic(Graph *graph, BasicBlock *block, Inst *inst, DataType::Type type);
    void PopulateLoadStatic(Graph *graph, BasicBlock *block, Inst *inst);
    void PopulateMonitor(Graph *graph, BasicBlock *block, Inst *inst);
    void PopulateLoadType(Graph *graph, BasicBlock *block, Inst *inst);
    void PopulateIsInstance(Graph *graph, BasicBlock *block, Inst *inst);
    void PopulateNewArray(Graph *graph, BasicBlock *block, Inst *inst, int32_t n);
    void PopulateNewObject(Graph *graph, BasicBlock *block, Inst *inst, int32_t n);
    void PopulateDefault(Graph *graph, Inst *inst, DataType::Type type, int32_t n);
    void PopulateGraph(Graph *graph, Inst *inst, int32_t n);
    void Finalize(Graph *graph, BasicBlock *block, Inst *inst);

    Graph *GenerateOperation(Inst *inst, int32_t n);
    Graph *GenerateCheckOperation(Inst *inst);
    Graph *GenerateSSOperation(Inst *inst);
    Graph *GenerateBoundaryCheckOperation(Inst *inst);
    Graph *GenerateThrowOperation(Inst *inst);
    Graph *GenerateMultiArrayOperation(Inst *inst);
    Graph *GeneratePhiOperation(Inst *inst);

#include "generate_operations_intrinsic_graph.inl"

    Graph *CreateGraphWithOneBasicBlock();
    Graph *CreateGraphWithTwoBasicBlock();
    Graph *CreateGraphWithThreeBasicBlock();
    Graph *CreateGraphWithFourBasicBlock();
    ParameterInst *CreateParamInst(Graph *graph, DataType::Type type, uint8_t slot);

    struct PackArgsForCkeckInst {
        Opcode opcode;
        DataType::Type type;
        Inst *inst;
        ParameterInst *param1;
        ParameterInst *param2;

        SaveStateInst *saveState;
        BasicBlock *block;
        Graph *graph;
    };

    Inst *CreateCheckInstByPackArgs(const PackArgsForCkeckInst &pack);

private:
    // need to create graphs
    ArenaAllocator &allocator_;
    ArenaAllocator &localAllocator_;
    RuntimeInterfaceMock runtime_;
    Arch arch_ {Arch::AARCH64};

public:
    void SetNumVRegsArgs(size_t regs, size_t args)
    {
        runtime_.vregsCount_ = regs;
        runtime_.argsCount_ = args;
    }
};

class InstGenerator {
public:
    InstGenerator() = delete;
    explicit InstGenerator(ArenaAllocator &allocator) : allocator_(allocator) {}

    std::vector<Inst *> &Generate(Opcode opCode);

    int GetAllPossibleInstToGenerateNumber()
    {
        int result = 0;
        for (auto &it : opcodeXPossibleTypes_) {
            result += it.second.size();
        }
        return result;
    }

    int GetPossibleInstToGenerateNumber(Opcode opCode)
    {
        return opcodeXPossibleTypes_[opCode].size();
    }

    std::map<Opcode, std::vector<DataType::Type>> &GetMap()
    {
        return opcodeXPossibleTypes_;
    }

    ArenaAllocator *GetAllocator()
    {
        return &allocator_;
    }

private:
    template <class T>
    std::vector<Inst *> &GenerateOperations(Opcode opCode);

    template <class T>
    std::vector<Inst *> &GenerateOperationsImm(Opcode opCode);

    template <class T>
    std::vector<Inst *> &GenerateOperationsShiftedRegister(Opcode opCode);

    void GenerateIntrinsic(DataType::Type type, RuntimeInterface::IntrinsicId intrinsicId)
    {
        auto inst = Inst::New<IntrinsicInst>(&allocator_, Opcode::Intrinsic);
        inst->SetType(type);
        inst->SetIntrinsicId(intrinsicId);
        insts_.push_back(inst);
    }

    void SetFlagsNoCseNoHoistIfReference(Inst *inst, DataType::Type dstType);

    std::vector<DataType::Type> integerTypes_ {DataType::UINT8,  DataType::INT8,  DataType::UINT16, DataType::INT16,
                                               DataType::UINT32, DataType::INT32, DataType::UINT64, DataType::INT64};

    std::vector<DataType::Type> numericTypes_ {DataType::BOOL,  DataType::UINT8,   DataType::INT8,   DataType::UINT16,
                                               DataType::INT16, DataType::UINT32,  DataType::INT32,  DataType::UINT64,
                                               DataType::INT64, DataType::FLOAT32, DataType::FLOAT64};

    std::vector<DataType::Type> refNumTypes_ {
        DataType::REFERENCE, DataType::BOOL,  DataType::UINT8,  DataType::INT8,  DataType::UINT16,  DataType::INT16,
        DataType::UINT32,    DataType::INT32, DataType::UINT64, DataType::INT64, DataType::FLOAT32, DataType::FLOAT64};

    std::vector<DataType::Type> refIntTypes_ {DataType::REFERENCE, DataType::BOOL,  DataType::UINT8,  DataType::INT8,
                                              DataType::UINT16,    DataType::INT16, DataType::UINT32, DataType::INT32,
                                              DataType::UINT64,    DataType::INT64};

    std::vector<DataType::Type> allTypes_ {DataType::REFERENCE, DataType::BOOL,  DataType::UINT8,   DataType::INT8,
                                           DataType::UINT16,    DataType::INT16, DataType::UINT32,  DataType::INT32,
                                           DataType::UINT64,    DataType::INT64, DataType::FLOAT32, DataType::FLOAT64,
                                           DataType::VOID};

    std::vector<DataType::Type> floatsTypes_ {DataType::FLOAT32, DataType::FLOAT64};

    std::map<Opcode, std::vector<DataType::Type>> opcodeXPossibleTypes_ = {
        {Opcode::Neg, numericTypes_},
        {Opcode::Abs, numericTypes_},
        {Opcode::Not, integerTypes_},
        {Opcode::Add, numericTypes_},
        {Opcode::Sub, numericTypes_},
        {Opcode::Mul, numericTypes_},
        {Opcode::Div, numericTypes_},
        {Opcode::Min, numericTypes_},
        {Opcode::Max, numericTypes_},
        {Opcode::Shl, integerTypes_},
        {Opcode::Shr, integerTypes_},
        {Opcode::AShr, integerTypes_},
        {Opcode::Mod, numericTypes_},
        {Opcode::And, integerTypes_},
        {Opcode::Or, integerTypes_},
        {Opcode::Xor, integerTypes_},
        {Opcode::Compare, refNumTypes_},
        {Opcode::If, refIntTypes_},
        {Opcode::Cmp, {DataType::INT32}},
        {Opcode::Constant, {DataType::INT64, DataType::FLOAT32, DataType::FLOAT64}},
        {Opcode::Phi, refNumTypes_},
        {Opcode::IfImm, refIntTypes_},
        {Opcode::Cast, numericTypes_},
        {Opcode::Parameter, refNumTypes_},
        {Opcode::IsInstance, {DataType::BOOL}},
        {Opcode::LenArray, {DataType::INT32}},
        {Opcode::LoadArray, refNumTypes_},
        {Opcode::StoreArray, refNumTypes_},
        {Opcode::LoadArrayI, refNumTypes_},
        {Opcode::StoreArrayI, refNumTypes_},
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
        {Opcode::Return, refNumTypes_},
        {Opcode::ReturnI, numericTypes_},
        {Opcode::CallStatic, allTypes_},
        {Opcode::CallVirtual, allTypes_},
        {Opcode::AddI, integerTypes_},
        {Opcode::SubI, integerTypes_},
        {Opcode::AndI, integerTypes_},
        {Opcode::OrI, integerTypes_},
        {Opcode::XorI, integerTypes_},
        {Opcode::ShrI, integerTypes_},
        {Opcode::ShlI, integerTypes_},
        {Opcode::AShrI, integerTypes_},
        {Opcode::SpillFill, {DataType::NO_TYPE}},
        {Opcode::NewObject, {DataType::REFERENCE}},
        {Opcode::LoadObject, refNumTypes_},
        {Opcode::LoadStatic, refNumTypes_},
        {Opcode::StoreObject, refNumTypes_},
        {Opcode::StoreStatic, refNumTypes_},
        {Opcode::LoadString, {DataType::REFERENCE}},
        {Opcode::LoadType, {DataType::REFERENCE}},
        {Opcode::SafePoint, {DataType::NO_TYPE}},
        {Opcode::ReturnInlined, {DataType::NO_TYPE}},
        {Opcode::Monitor, {DataType::VOID}},
        {Opcode::Intrinsic, {}},
        {Opcode::Select, refIntTypes_},
        {Opcode::SelectImm, refIntTypes_},
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
        {Opcode::AndNot, integerTypes_},
        {Opcode::OrNot, integerTypes_},
        {Opcode::XorNot, integerTypes_},
        {Opcode::MNeg, numericTypes_},
        {Opcode::MAdd, numericTypes_},
        {Opcode::MSub, numericTypes_},
        {Opcode::AddSR, integerTypes_},
        {Opcode::SubSR, integerTypes_},
        {Opcode::AndSR, integerTypes_},
        {Opcode::OrSR, integerTypes_},
        {Opcode::XorSR, integerTypes_},
        {Opcode::AndNotSR, integerTypes_},
        {Opcode::OrNotSR, integerTypes_},
        {Opcode::XorNotSR, integerTypes_},
        {Opcode::NegSR, integerTypes_},
    };

    std::vector<ShiftType> onlyShifts_ = {ShiftType::LSL, ShiftType::LSR, ShiftType::ASR};
    std::vector<ShiftType> shiftsAndRotation_ = {ShiftType::LSL, ShiftType::LSR, ShiftType::ASR, ShiftType::ROR};
    std::map<Opcode, std::vector<ShiftType>> opcodeXPossibleShiftTypes_ = {
        {Opcode::AddSR, onlyShifts_},          {Opcode::SubSR, onlyShifts_},
        {Opcode::AndSR, shiftsAndRotation_},   {Opcode::OrSR, shiftsAndRotation_},
        {Opcode::XorSR, shiftsAndRotation_},   {Opcode::AndNotSR, shiftsAndRotation_},
        {Opcode::OrNotSR, shiftsAndRotation_}, {Opcode::XorNotSR, shiftsAndRotation_},
        {Opcode::NegSR, onlyShifts_}};
    std::vector<Inst *> insts_;

    // need to create graphs
    ArenaAllocator &allocator_;
};

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class StatisticGenerator {
public:
    StatisticGenerator(InstGenerator &instGenerator, GraphCreator &graphCreator)
        : instGenerator_(instGenerator), graphCreator_(graphCreator)
    {
    }

    virtual ~StatisticGenerator() = default;

    NO_COPY_SEMANTIC(StatisticGenerator);
    NO_MOVE_SEMANTIC(StatisticGenerator);

    using FullInstStat = std::map<DataType::Type, int8_t>;
    using FullIntrinsicStat = std::map<RuntimeInterface::IntrinsicId, bool>;
    using FullStat = std::pair<std::map<Opcode, FullInstStat>, FullIntrinsicStat>;

    virtual void Generate() = 0;

    FullStat &GetStatistic()
    {
        return statistic_;
    }

    void GenerateHTMLPage(const std::string &fileName);

private:
    void FillHTMLPageHeadPart(std::ofstream &htmlPage);
    void FillHTMLPageOpcodeStatistic(std::ofstream &htmlPage, Opcode opc);

protected:
    InstGenerator &instGenerator_;
    GraphCreator &graphCreator_;

    int allInstNumber_ = 0;
    int positiveInstNumber_ = 0;

    int allOpcodeNumber_ = 0;
    int implementedOpcodeNumber_ = 0;

    FullStat statistic_;

    FullInstStat tmplt_ = {
        {DataType::NO_TYPE, -1}, {DataType::REFERENCE, -1}, {DataType::BOOL, -1},  {DataType::UINT8, -1},
        {DataType::INT8, -1},    {DataType::UINT16, -1},    {DataType::INT16, -1}, {DataType::UINT32, -1},
        {DataType::INT32, -1},   {DataType::UINT64, -1},    {DataType::INT64, -1}, {DataType::FLOAT32, -1},
        {DataType::FLOAT64, -1}, {DataType::ANY, -1},       {DataType::VOID, -1},
    };
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark::compiler

#endif  // COMPILER_TESTS_INST_GENERATOR_H
