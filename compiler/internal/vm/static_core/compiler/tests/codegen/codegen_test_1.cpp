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

#include "codegen_test.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/ir_constructor.h"
#include "tests/unit_test.h"

namespace ark::compiler {

constexpr uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
constexpr uint64_t ITERATION = 40;
#else
constexpr uint64_t ITERATION = 20000;
#endif
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-msc51-cpp)
static auto g_randomGenerator = std::mt19937_64(SEED);

// NOLINTBEGIN(readability-magic-numbers,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)
TEST_F(CodegenTest, SimpleProgramm)
{
    /*
   .function main()<main>{
       movi.64 v0, 100000000           ##      0 -> 3      ##  bb0
       movi.64 v1, 4294967296          ##      1 -> 4      ##  bb0
       ldai 0                          ##      2 -> 5      ##  bb0
   loop:                               ##                  ##
       jeq v0, loop_exit               ##      6, 7, 8     ##  bb1
                                       ##                  ##
       sta.64 v2                       ##      9           ##  bb2
       and.64 v1                       ##      10          ##  bb2
       sta.64 v1                       ##      11          ##  bb2
       lda.64 v2                       ##      12          ##  bb2
       inc                             ##      13          ##  bb2
       jmp loop                        ##      14          ##  bb2
   loop_exit:                          ##                  ##
       lda.64 v1                       ##      14          ##  bb3
       return.64                       ##      15          ##  bb3
   }
   */

    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10UL);          // r1
        CONSTANT(1U, 4294967296UL);  // r2
        CONSTANT(2U, 0UL);           // r3 -> acc(3)
        CONSTANT(3U, 0x1UL);         // r20 -> 0x1 (for inc constant)

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(16U, Opcode::Phi).Inputs(2U, 13U).s64();  // PHI acc
            INST(17U, Opcode::Phi).Inputs(1U, 10U).s64();  // PHI  v1
            INST(20U, Opcode::Phi).Inputs(2U, 10U).s64();  // result to return

            // NOTE (igorban): support CMP instr
            INST(18U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 16U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(18U);
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(10U, Opcode::And).Inputs(16U, 17U).s64();  // -> acc
            INST(13U, Opcode::Add).Inputs(16U, 3U).s64();   // -> acc
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::Return).Inputs(20U).s64();
        }
    }

    SetNumVirtRegs(0U);
    SetNumArgs(1U);

    RegAlloc(GetGraph());

    // call codegen
    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto entry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto exit = entry + GetGraph()->GetCode().Size();
    ASSERT(entry != nullptr && exit != nullptr);
    GetExecModule().SetInstructions(entry, exit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();

    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 0U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

SRC_GRAPH(CheckStoreArray, Graph *graph, DataType::Type type)
{
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0U, DataType::REFERENCE);
    auto index = graph->AddNewParameter(1U, DataType::INT32);
    auto storeValue = graph->AddNewParameter(2U, type);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));
    storeValue->SetLocationData(graph->GetDataForNativeParam(type));

    auto stArr = graph->CreateInst(Opcode::StoreArray);
    block->AppendInst(stArr);
    stArr->SetType(type);
    stArr->SetInput(0U, array);
    stArr->SetInput(1U, index);
    stArr->SetInput(2U, storeValue);
    auto ret = graph->CreateInst(Opcode::ReturnVoid);
    block->AppendInst(ret);
}

template <typename T>
void CodegenTest::CheckStoreArray()
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);
    src_graph::CheckStoreArray::CREATE(graph, TYPE);
    SetNumVirtRegs(0U);
    SetNumArgs(3U);

    RegAlloc(graph);

    // call codegen
    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    T arrayData[4U];
    auto defaultValue = CutValue<T>(0U, TYPE);
    for (auto &data : arrayData) {
        data = defaultValue;
    }
    auto param1 = GetExecModule().CreateArray(arrayData, 4U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    auto param3 = CutValue<T>(10U, TYPE);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().SetParameter(2U, param3);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param1, arrayData);

    for (size_t i = 0; i < 4U; i++) {
        if (i == 2U) {
            EXPECT_EQ(arrayData[i], param3);
        } else {
            EXPECT_EQ(arrayData[i], defaultValue);
        }
    }
    GetExecModule().FreeArray(param1);
}

SRC_GRAPH(CheckLoadArray, Graph *graph, DataType::Type type)
{
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0U, DataType::REFERENCE);
    auto index = graph->AddNewParameter(1U, DataType::INT32);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));

    auto ldArr = graph->CreateInst(Opcode::LoadArray);
    block->AppendInst(ldArr);
    ldArr->SetType(type);
    ldArr->SetInput(0U, array);
    ldArr->SetInput(1U, index);
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(type);
    ret->SetInput(0U, ldArr);
    block->AppendInst(ret);
}

template <typename T>
void CodegenTest::CheckLoadArray()
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();
    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    src_graph::CheckLoadArray::CREATE(graph, TYPE);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    T arrayData[4U];
    for (size_t i = 0; i < 4U; i++) {
        arrayData[i] = CutValue<T>((-i), TYPE);
    }
    auto param1 = GetExecModule().CreateArray(arrayData, 4U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param1, arrayData);

    GetExecModule().FreeArray(param1);

    auto retData = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(retData, CutValue<T>(-2L, TYPE));
}

SRC_GRAPH(CheckStoreArrayPair, Graph *graph, DataType::Type type, bool imm)
{
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0U, DataType::REFERENCE);
    [[maybe_unused]] auto index = graph->AddNewParameter(1U, DataType::INT32);
    auto val0 = graph->AddNewParameter(2U, type);
    auto val1 = graph->AddNewParameter(3U, type);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));
    val0->SetLocationData(graph->GetDataForNativeParam(type));
    val1->SetLocationData(graph->GetDataForNativeParam(type));

    Inst *stpArr = nullptr;
    if (imm) {
        stpArr = graph->CreateInstStoreArrayPairI(type, INVALID_PC, array, val0, val1, 2U);
        block->AppendInst(stpArr);
    } else {
        stpArr = graph->CreateInstStoreArrayPair(type, INVALID_PC, std::array<Inst *, 4U> {array, index, val0, val1});
        block->AppendInst(stpArr);
    }

    auto ret = graph->CreateInst(Opcode::ReturnVoid);
    block->AppendInst(ret);

    GraphChecker(graph).Check();
}

template <typename T>
void CodegenTest::CheckStoreArrayPair(bool imm)
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);
#ifndef NDEBUG
    // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
    graph->SetLowLevelInstructionsEnabled();
#endif

    src_graph::CheckStoreArrayPair::CREATE(graph, TYPE, imm);

    SetNumVirtRegs(0U);
    SetNumArgs(4U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    T arrayData[6U] = {0U, 0U, 0U, 0U, 0U, 0U};
    auto param1 = GetExecModule().CreateArray(arrayData, 6U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    auto param3 = CutValue<T>(3U, TYPE);
    auto param4 = CutValue<T>(5U, TYPE);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().SetParameter(2U, param3);
    GetExecModule().SetParameter(3U, param4);

    GetExecModule().Execute();
    GetExecModule().CopyArray(param1, arrayData);
    GetExecModule().FreeArray(param1);

    T arrayExpected[6U] = {0U, 0U, 3U, 5U, 0U, 0U};

    for (size_t i = 0; i < 6U; ++i) {
        EXPECT_EQ(arrayData[i], arrayExpected[i]);
    }
}

SRC_GRAPH(CheckLoadArrayPair, Graph *graph, DataType::Type type, bool imm)
{
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto array = graph->AddNewParameter(0U, DataType::REFERENCE);
    [[maybe_unused]] auto index = graph->AddNewParameter(1U, DataType::INT32);

    graph->ResetParameterInfo();
    array->SetLocationData(graph->GetDataForNativeParam(DataType::REFERENCE));
    index->SetLocationData(graph->GetDataForNativeParam(DataType::INT32));

    Inst *ldpArr = nullptr;
    if (imm) {
        ldpArr = graph->CreateInstLoadArrayPairI(type, INVALID_PC, array, 2U);
        block->AppendInst(ldpArr);
    } else {
        ldpArr = graph->CreateInstLoadArrayPair(type, INVALID_PC, array, index);
        block->AppendInst(ldpArr);
    }

    auto loadHigh = graph->CreateInstLoadPairPart(type, INVALID_PC, ldpArr, 0U);
    block->AppendInst(loadHigh);

    auto loadLow = graph->CreateInstLoadPairPart(type, INVALID_PC, ldpArr, 1U);
    block->AppendInst(loadLow);

    auto sum = graph->CreateInst(Opcode::Add);
    block->AppendInst(sum);
    sum->SetType(type);
    sum->SetInput(0U, loadHigh);
    sum->SetInput(1U, loadLow);

    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(type);
    ret->SetInput(0U, sum);
    block->AppendInst(ret);

    GraphChecker(graph).Check();
}

template <typename T>
void CodegenTest::CheckLoadArrayPair(bool imm)
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);
#ifndef NDEBUG
    // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
    graph->SetLowLevelInstructionsEnabled();
#endif

    src_graph::CheckLoadArrayPair::CREATE(graph, TYPE, imm);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    T arrayData[6U];
    // [ 1, 2, 3, 4, 5, 6] -> 7
    for (size_t i = 0; i < 6U; i++) {
        arrayData[i] = CutValue<T>(i + 1U, TYPE);
    }
    auto param1 = GetExecModule().CreateArray(arrayData, 6U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();
    GetExecModule().FreeArray(param1);

    auto retData = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(retData, CutValue<T>(7U, TYPE));
}

SRC_GRAPH(CheckBounds, Graph *graph, DataType::Type type, uint64_t count)
{
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto param = graph->AddNewParameter(0U, type);

    graph->ResetParameterInfo();
    param->SetLocationData(graph->GetDataForNativeParam(type));

    Inst *lastInst = param;
    // instruction_count + parameter + return
    for (uint64_t i = count - 1U; i > 1U; --i) {
        auto addInst = graph->CreateInstAddI(type, 0U, lastInst, 1U);
        block->AppendInst(addInst);
        lastInst = addInst;
    }
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(type);
    ret->SetInput(0U, lastInst);
    block->AppendInst(ret);

#ifndef NDEBUG
    // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
    graph->SetLowLevelInstructionsEnabled();
#endif
    GraphChecker(graph).Check();
}

template <typename T>
void CodegenTest::CheckBounds(uint64_t count)
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();
    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    src_graph::CheckBounds::CREATE(graph, TYPE, count);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    auto instsPerByte = GetGraph()->GetEncoder()->MaxArchInstPerEncoded();
    auto maxBitsInInst = GetInstructionSizeBits(GetGraph()->GetArch());
    if (count * instsPerByte * maxBitsInInst > g_options.GetCompilerMaxGenCodeSize()) {
        EXPECT_FALSE(RunCodegen(graph));
    } else {
        ASSERT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetDump(false);

        T zeroParam = 0;
        GetExecModule().SetParameter(0U, zeroParam);
        GetExecModule().Execute();

        auto retData = GetExecModule().GetRetValue<T>();
        EXPECT_EQ(retData, CutValue<T>(count - 2L, TYPE));
    }
}

SRC_GRAPH(CheckCmp, Graph *graph, DataType::Type type, bool isFcmpg)
{
    auto entry = graph->CreateStartBlock();
    auto exit = graph->CreateEndBlock();
    auto block = graph->CreateEmptyBlock();
    entry->AddSucc(block);
    block->AddSucc(exit);

    auto param1 = graph->AddNewParameter(0U, type);
    auto param2 = graph->AddNewParameter(1U, type);

    graph->ResetParameterInfo();
    param1->SetLocationData(graph->GetDataForNativeParam(type));
    param2->SetLocationData(graph->GetDataForNativeParam(type));

    auto fcmp = graph->CreateInst(Opcode::Cmp);
    block->AppendInst(fcmp);
    fcmp->SetType(DataType::INT32);
    fcmp->SetInput(0U, param1);
    fcmp->SetInput(1U, param2);
    static_cast<CmpInst *>(fcmp)->SetOperandsType(type);
    if (DataType::IsFloatType(type)) {
        static_cast<CmpInst *>(fcmp)->SetFcmpg(isFcmpg);
    }
    auto ret = graph->CreateInst(Opcode::Return);
    ret->SetType(DataType::INT32);
    ret->SetInput(0U, fcmp);
    block->AppendInst(ret);
}

template <typename T>
void CodegenTest::CheckCmp(bool isFcmpg)
{
    constexpr DataType::Type TYPE = VixlExecModule::GetType<T>();

    // Create graph
    auto graph = CreateEmptyGraph();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    src_graph::CheckCmp::CREATE(graph, TYPE, isFcmpg);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);
    T paramData[3U];
    if (TYPE == DataType::FLOAT32) {
        paramData[0U] = std::nanf("0");
    } else if (TYPE == DataType::FLOAT64) {
        paramData[0U] = std::nan("0");
    } else {
        paramData[0U] = std::numeric_limits<T>::max();
        paramData[2U] = std::numeric_limits<T>::min();
    }
    paramData[1U] = CutValue<T>(2U, TYPE);
    if (DataType::IsFloatType(TYPE)) {
        paramData[2U] = -paramData[1U];
    }

    for (size_t i = 0; i < 3U; i++) {
        for (size_t j = 0; j < 3U; j++) {
            auto paramData1 = paramData[i];
            auto paramData2 = paramData[j];
            GetExecModule().SetParameter(0U, paramData1);
            GetExecModule().SetParameter(1U, paramData2);

            GetExecModule().Execute();

            auto retData = GetExecModule().GetRetValue<int32_t>();
            if ((i == 0U || j == 0U) && DataType::IsFloatType(TYPE)) {
                EXPECT_EQ(retData, isFcmpg ? 1U : -1L);
            } else if (i == j) {
                EXPECT_EQ(retData, 0U);
            } else if (i > j) {
                EXPECT_EQ(retData, -1L);
            } else {
                EXPECT_EQ(retData, 1U);
            }
        }
    }
}

TEST_F(CodegenTest, Cmp)
{
    CheckCmp<float>(true);
    CheckCmp<float>(false);
    CheckCmp<double>(true);
    CheckCmp<double>(false);
    CheckCmp<uint8_t>();
    CheckCmp<int8_t>();
    CheckCmp<uint16_t>();
    CheckCmp<int16_t>();
    CheckCmp<uint32_t>();
    CheckCmp<int32_t>();
    CheckCmp<uint64_t>();
    CheckCmp<int64_t>();
}

SRC_GRAPH(StoreArray, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();       // array
        PARAMETER(1U, 1U).u32();       // index
        PARAMETER(2U, 2U).ref_uint();  // store value
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::StoreArray).ref_uint().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(CodegenTest, StoreArray)
{
    CheckStoreArray<bool>();
    CheckStoreArray<int8_t>();
    CheckStoreArray<uint8_t>();
    CheckStoreArray<int16_t>();
    CheckStoreArray<uint16_t>();
    CheckStoreArray<int32_t>();
    CheckStoreArray<uint32_t>();
    CheckStoreArray<int64_t>();
    CheckStoreArray<uint64_t>();
    CheckStoreArray<float>();
    CheckStoreArray<double>();

    src_graph::StoreArray::CREATE(GetGraph());

    auto graph = GetGraph();
    SetNumVirtRegs(0U);
    SetNumArgs(3U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    ObjectPointerType array[4U] = {0U, 0U, 0U, 0U};
    auto param1 = GetExecModule().CreateArray(array, 4U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    auto param3 = CutValue<ObjectPointerType>(10U, DataType::UINT64);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().SetParameter(2U, param3);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param1, array);

    for (size_t i = 0; i < 4U; i++) {
        if (i == 2U) {
            EXPECT_EQ(array[i], 10U) << "value of i: " << i;
        } else {
            EXPECT_EQ(array[i], 0U) << "value of i: " << i;
        }
    }
    GetExecModule().FreeArray(param1);
}

TEST_F(CodegenTest, StoreArrayPair)
{
    CheckStoreArrayPair<uint32_t>(true);
    CheckStoreArrayPair<int32_t>(false);
    CheckStoreArrayPair<uint64_t>(true);
    CheckStoreArrayPair<int64_t>(false);
    CheckStoreArrayPair<float>(true);
    CheckStoreArrayPair<float>(false);
    CheckStoreArrayPair<double>(true);
    CheckStoreArrayPair<double>(false);
}

SRC_GRAPH(Compare, Graph *graph, ConditionCode cc, bool inverse)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(inverse ? CC_EQ : CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Return).b().Inputs(3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
}

TEST_F(CodegenTest, Compare)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        for (auto inverse : {true, false}) {
            auto graph = CreateGraphStartEndBlocks();
            RuntimeInterfaceMock runtime;
            graph->SetRuntime(&runtime);
            src_graph::Compare::CREATE(graph, cc, inverse);

            SetNumVirtRegs(0U);
            SetNumArgs(2U);
            RegAlloc(graph);

            EXPECT_TRUE(RunCodegen(graph));
            auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
            auto codeExit = codeEntry + graph->GetCode().Size();
            ASSERT(codeEntry != nullptr && codeExit != nullptr);
            GetExecModule().SetInstructions(codeEntry, codeExit);

            GetExecModule().SetDump(false);

            auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
            auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param2);

            bool result = (cc == CC_NE || cc == CC_GT || cc == CC_GE || cc == CC_B || cc == CC_BE);
            if (inverse) {
                result = !result;
            }

            GetExecModule().Execute();

            auto retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);

            GetExecModule().SetParameter(0U, param2);
            GetExecModule().SetParameter(1U, param1);
            GetExecModule().Execute();

            result = (cc == CC_NE || cc == CC_LT || cc == CC_LE || cc == CC_A || cc == CC_AE);
            if (inverse) {
                result = !result;
            }

            retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);

            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param1);

            result = (cc == CC_EQ || cc == CC_LE || cc == CC_GE || cc == CC_AE || cc == CC_BE);
            if (inverse) {
                result = !result;
            }

            GetExecModule().Execute();

            retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);
        }
    }
}

SRC_GRAPH(GenIf, Graph *graph, ConditionCode cc)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Return).b().Inputs(3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(2U);
        }
    }
}

TEST_F(CodegenTest, GenIf)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        src_graph::GenIf::CREATE(graph, cc);

        SetNumVirtRegs(0U);
        SetNumArgs(2U);
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif
        EXPECT_TRUE(graph->RunPass<Lowering>());
        ASSERT_EQ(INS(0U).GetUsers().Front().GetInst()->GetOpcode(), Opcode::If);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetDump(false);

        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        bool result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = Compare(cc, param1, param1);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

SRC_GRAPH(GenIfImm, Graph *graph, ConditionCode cc, int32_t value)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::INT32).CC(cc).Imm(value).Inputs(0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::Return).b().Inputs(1U);
        }
    }
}
TEST_F(CodegenTest, GenIfImm)
{
    int32_t values[3U] = {-1L, 0U, 1U};
    for (auto value : values) {
        for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
            auto cc = static_cast<ConditionCode>(ccint);
            auto graph = CreateGraphStartEndBlocks();
            RuntimeInterfaceMock runtime;
            graph->SetRuntime(&runtime);
            if ((cc == CC_TST_EQ || cc == CC_TST_NE) && !graph->GetEncoder()->CanEncodeImmLogical(value, WORD_SIZE)) {
                continue;
            }
            src_graph::GenIfImm::CREATE(graph, cc, value);

            SetNumVirtRegs(0U);
            SetNumArgs(2U);

            RegAlloc(graph);

            EXPECT_TRUE(RunCodegen(graph));
            auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
            auto codeExit = codeEntry + graph->GetCode().Size();
            ASSERT(codeEntry != nullptr && codeExit != nullptr);
            GetExecModule().SetInstructions(codeEntry, codeExit);

            GetExecModule().SetDump(false);

            bool result;
            auto param1 = CutValue<uint64_t>(value, DataType::INT32);
            auto param2 = CutValue<uint64_t>(value + 5U, DataType::INT32);
            auto param3 = CutValue<uint64_t>(value - 5L, DataType::INT32);

            GetExecModule().SetParameter(0U, param1);
            result = Compare(cc, param1, param1);
            GetExecModule().Execute();
            auto retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);

            GetExecModule().SetParameter(0U, param2);
            GetExecModule().Execute();
            result = Compare(cc, param2, param1);
            retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);

            GetExecModule().SetParameter(0U, param3);
            result = Compare(cc, param3, param1);
            GetExecModule().Execute();
            retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result);
        }
    }
}

SRC_GRAPH(If, Graph *graph, ConditionCode cc)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).b().Inputs(3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Return).b().Inputs(2U);
        }
    }
}
TEST_F(CodegenTest, If)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        src_graph::If::CREATE(graph, cc);

        SetNumVirtRegs(0U);
        SetNumArgs(2U);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetDump(false);

        bool result;
        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = Compare(cc, param1, param1);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

SRC_GRAPH(Overflow, Graph *graph, Opcode overflowOpcode)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, overflowOpcode).i32().SrcType(DataType::INT32).CC(CC_EQ).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).b().Inputs(2U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Return).b().Inputs(4U);
        }
    }
}

TEST_F(CodegenTest, AddOverflow)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    src_graph::Overflow::CREATE(graph, Opcode::AddOverflow);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    InPlaceCompilerTaskRunner taskRunner;
    taskRunner.GetContext().SetGraph(graph);
    bool success = true;
    taskRunner.AddCallbackOnFail([&success]([[maybe_unused]] InPlaceCompilerContext &compilerCtx) { success = false; });
    RunOptimizations<INPLACE_MODE>(std::move(taskRunner));
    EXPECT_TRUE(success);

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    int32_t min = std::numeric_limits<int32_t>::min();
    int32_t max = std::numeric_limits<int32_t>::max();
    std::array<int32_t, 7U> values = {0U, 2U, 5U, -7L, -10L, max, min};
    for (uint32_t i = 0; i < values.size(); ++i) {
        for (uint32_t j = 0; j < values.size(); ++j) {
            int32_t a0 = values[i];
            int32_t a1 = values[j];
            int32_t result;
            auto param1 = CutValue<int32_t>(a0, DataType::INT32);
            auto param2 = CutValue<int32_t>(a1, DataType::INT32);

            if ((a0 > 0 && a1 > max - a0) || (a0 < 0 && a1 < min - a0)) {
                result = 0;
            } else {
                result = a0 + a1;
            }
            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param2);
            GetExecModule().Execute();

            auto retData = GetExecModule().GetRetValue<int32_t>();
            EXPECT_EQ(retData, result);
        }
    }
}

TEST_F(CodegenTest, SubOverflow)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    src_graph::Overflow::CREATE(graph, Opcode::SubOverflow);

    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    InPlaceCompilerTaskRunner taskRunner;
    taskRunner.GetContext().SetGraph(graph);
    bool success = true;
    taskRunner.AddCallbackOnFail([&success]([[maybe_unused]] InPlaceCompilerContext &compilerCtx) { success = false; });
    RunOptimizations<INPLACE_MODE>(std::move(taskRunner));
    EXPECT_TRUE(success);

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    int32_t min = std::numeric_limits<int32_t>::min();
    int32_t max = std::numeric_limits<int32_t>::max();
    std::array<int32_t, 7U> values = {0U, 2U, 5U, -7L, -10L, max, min};
    for (uint32_t i = 0; i < values.size(); ++i) {
        for (uint32_t j = 0; j < values.size(); ++j) {
            int32_t a0 = values[i];
            int32_t a1 = values[j];
            int32_t result;
            auto param1 = CutValue<int32_t>(a0, DataType::INT32);
            auto param2 = CutValue<int32_t>(a1, DataType::INT32);

            if ((a1 > 0 && a0 < min + a1) || (a1 < 0 && a0 > max + a1)) {
                result = 0;
            } else {
                result = a0 - a1;
            }
            GetExecModule().SetParameter(0U, param1);
            GetExecModule().SetParameter(1U, param2);
            GetExecModule().Execute();

            auto retData = GetExecModule().GetRetValue<int32_t>();
            EXPECT_EQ(retData, result);
        }
    }
}

SRC_GRAPH(GenSelect, Graph *graph, ConditionCode cc)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::If).SrcType(DataType::INT64).CC(cc).Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::Phi).b().Inputs({{3U, 3U}, {2U, 2U}});
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
}

TEST_F(CodegenTest, GenSelect)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        src_graph::GenSelect::CREATE(graph, cc);

        SetNumVirtRegs(0U);
        SetNumArgs(2U);

        EXPECT_TRUE(graph->RunPass<IfConversion>());
        ASSERT_EQ(INS(6U).GetInput(0U).GetInst()->GetOpcode(), Opcode::Select);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);

        GetExecModule().SetDump(false);

        bool result;
        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = Compare(cc, param1, param1);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

SRC_GRAPH(BoolSelectImm, Graph *graph, ConditionCode cc)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Compare).b().CC(cc).Inputs(0U, 1U);
            INST(5U, Opcode::SelectImm).b().SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U, 2U, 4U);
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
}
TEST_F(CodegenTest, BoolSelectImm)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        src_graph::BoolSelectImm::CREATE(graph, cc);

        SetNumVirtRegs(0U);
        SetNumArgs(2U);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);
        GetExecModule().SetDump(false);

        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        bool result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = Compare(cc, param1, param1);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

SRC_GRAPH(Select, Graph *graph, ConditionCode cc)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::Select).u64().SrcType(DataType::UINT64).CC(cc).Inputs(3U, 2U, 0U, 1U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
}
TEST_F(CodegenTest, Select)
{
    for (int ccint = ConditionCode::CC_FIRST; ccint != ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        auto graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        src_graph::Select::CREATE(graph, cc);

        SetNumVirtRegs(0U);
        SetNumArgs(2U);

        RegAlloc(graph);

        EXPECT_TRUE(RunCodegen(graph));
        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);
        GetExecModule().SetDump(false);

        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        bool result = Compare(cc, param1, param2);
        GetExecModule().Execute();
        auto retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        result = Compare(cc, param2, param1);
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = (cc == CC_EQ || cc == CC_LE || cc == CC_GE || cc == CC_AE || cc == CC_BE);
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

SRC_GRAPH(SelectTransform, Graph *graph, ConditionCode cc, SelectTransformType transform)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SelectTransform)
                .u64()
                .SrcType(DataType::UINT64)
                .CC(cc)
                .SelectTransform(transform)
                .Inputs(2U, 3U, 0U, 1U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
}

TEST_F(CodegenTest, SelectTransform)
{
    if (GetArch() != Arch::AARCH64) {
        return;
    }
    auto testCasePair = std::vector<std::pair<ConditionCode, SelectTransformType>> {};
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        testCasePair.emplace_back(cc, SelectTransformType::INC);
        testCasePair.emplace_back(cc, SelectTransformType::INV);
        testCasePair.emplace_back(cc, SelectTransformType::NEG);
    }
    for (auto [cc, transform] : testCasePair) {
        auto *graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        src_graph::SelectTransform::CREATE(graph, cc, transform);
        SetNumVirtRegs(0U);
        SetNumArgs(2U);
        RegAlloc(graph);
        EXPECT_TRUE(RunCodegen(graph));

        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);
        GetExecModule().SetDump(false);

        auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(-1L, DataType::UINT64);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param2);
        // if param1 op param2 then result = 0 else result = 1
        auto result = static_cast<int64_t>(!Compare(cc, param1, param2));
        if (result != 0) {
            result = Transform(transform, result);
        }
        GetExecModule().Execute();
        int64_t retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param2);
        GetExecModule().SetParameter(1U, param1);
        GetExecModule().Execute();
        // if param2 op param1 then result = 0 else result = 1
        result = static_cast<int64_t>(!Compare(cc, param2, param1));
        if (result != 0) {
            result = Transform(transform, result);
        }
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);

        GetExecModule().SetParameter(0U, param1);
        GetExecModule().SetParameter(1U, param1);
        result = static_cast<int64_t>(!Compare(cc, param1, param1));
        if (result != 0) {
            result = Transform(transform, result);
        }
        GetExecModule().Execute();
        retData = GetExecModule().GetRetValue();
        EXPECT_EQ(retData, result);
    }
}

SRC_GRAPH(SelectImmTransform, Graph *graph, ConditionCode cc, SelectTransformType transform)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SelectImmTransform)
                .u64()
                .SrcType(DataType::UINT64)
                .CC(cc)
                .SelectTransform(transform)
                .Imm(1U)
                .Inputs(1U, 2U, 0U);
            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }
}

TEST_F(CodegenTest, SelectImmTransform)
{
    if (GetArch() != Arch::AARCH64) {
        return;
    }
    auto testCasePair = std::vector<std::pair<ConditionCode, SelectTransformType>> {};
    for (int ccint = ConditionCode::CC_FIRST; ccint <= ConditionCode::CC_LAST; ccint++) {
        auto cc = static_cast<ConditionCode>(ccint);
        testCasePair.emplace_back(cc, SelectTransformType::INC);
        testCasePair.emplace_back(cc, SelectTransformType::INV);
        testCasePair.emplace_back(cc, SelectTransformType::NEG);
    }
    for (auto [cc, transform] : testCasePair) {
        auto *graph = CreateGraphStartEndBlocks();
        RuntimeInterfaceMock runtime;
        graph->SetRuntime(&runtime);

        src_graph::SelectImmTransform::CREATE(graph, cc, transform);
        SetNumVirtRegs(0U);
        SetNumArgs(2U);
        RegAlloc(graph);
        EXPECT_TRUE(RunCodegen(graph));

        auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
        auto codeExit = codeEntry + graph->GetCode().Size();
        ASSERT(codeEntry != nullptr && codeExit != nullptr);
        GetExecModule().SetInstructions(codeEntry, codeExit);
        GetExecModule().SetDump(false);

        auto param1 = CutValue<uint64_t>(-1L, DataType::UINT64);
        auto param2 = CutValue<uint64_t>(1U, DataType::UINT64);  // Equal to imm (see above)

        for (uint64_t curParam : {param1, param2}) {
            GetExecModule().SetParameter(0U, curParam);
            // If param op imm then result = 0 else result = 1 (see above)
            auto result = static_cast<int64_t>(!Compare(cc, curParam, param2));
            if (result != 0) {
                result = Transform(transform, result);
            }
            GetExecModule().Execute();
            int64_t retData = GetExecModule().GetRetValue();
            EXPECT_EQ(retData, result) << "Incorrect result with condition code = " << GetConditionCodeString(cc)
                                       << ", transformation type = " << GetSelectTransformTypeString(transform)
                                       << ", input param = " << curParam;
        }
    }
}

SRC_GRAPH(CompareObj, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_NE).Inputs(0U, 1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
}

TEST_F(CodegenTest, CompareObj)
{
    auto graph = CreateGraphStartEndBlocks();
    RuntimeInterfaceMock runtime;
    graph->SetRuntime(&runtime);

    src_graph::CompareObj::CREATE(graph);
    SetNumVirtRegs(0U);
    SetNumArgs(1U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    auto param1 = CutValue<uint64_t>(1U, DataType::UINT64);
    auto param2 = CutValue<uint64_t>(0U, DataType::UINT64);

    GetExecModule().SetParameter(0U, param1);

    GetExecModule().Execute();

    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 1U);

    GetExecModule().SetParameter(0U, param2);

    GetExecModule().Execute();

    retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 0U);
}

SRC_GRAPH(LoadArray, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u32();  // index
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::LoadArray).ref_uint().Inputs(0U, 1U);
            INST(3U, Opcode::Return).ref_uint().Inputs(2U);
        }
    }
}

TEST_F(CodegenTest, LoadArray)
{
    CheckLoadArray<bool>();
    CheckLoadArray<int8_t>();
    CheckLoadArray<uint8_t>();
    CheckLoadArray<int16_t>();
    CheckLoadArray<uint16_t>();
    CheckLoadArray<int32_t>();
    CheckLoadArray<uint32_t>();
    CheckLoadArray<int64_t>();
    CheckLoadArray<uint64_t>();
    CheckLoadArray<float>();
    CheckLoadArray<double>();

    src_graph::LoadArray::CREATE(GetGraph());
    auto graph = GetGraph();
    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    ObjectPointerType array[4U] = {0xffffaaaaU, 0xffffbbbbU, 0xffffccccU, 0xffffddddU};
    auto param1 = GetExecModule().CreateArray(array, 4U, GetObjectAllocator());
    auto param2 = CutValue<int32_t>(2U, DataType::INT32);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();

    GetExecModule().CopyArray(param1, array);

    GetExecModule().FreeArray(param1);
    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, array[2U]);
}

TEST_F(CodegenTest, LoadArrayPair)
{
    CheckLoadArrayPair<uint32_t>(true);
    CheckLoadArrayPair<int32_t>(false);
    CheckLoadArrayPair<uint64_t>(true);
    CheckLoadArrayPair<int64_t>(false);
    CheckLoadArrayPair<float>(true);
    CheckLoadArrayPair<float>(false);
    CheckLoadArrayPair<double>(true);
    CheckLoadArrayPair<double>(false);
}

#ifndef USE_ADDRESS_SANITIZER
TEST_F(CodegenTest, CheckCodegenBounds)
{
    // Do not try to encode too large graph
    uint64_t instsPerByte = GetGraph()->GetEncoder()->MaxArchInstPerEncoded();
    uint64_t maxBitsInInst = GetInstructionSizeBits(GetGraph()->GetArch());
    uint64_t instCount = g_options.GetCompilerMaxGenCodeSize() / (instsPerByte * maxBitsInInst);

    CheckBounds<uint32_t>(instCount - 1L);
    CheckBounds<uint32_t>(instCount + 1U);

    CheckBounds<uint32_t>(instCount / 2U);
}
#endif

TEST_F(CodegenTest, LenArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LenArray).s32().Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }
    auto graph = GetGraph();
    SetNumVirtRegs(0U);
    SetNumArgs(1U);

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    uint64_t array[4U] = {0U, 0U, 0U, 0U};
    auto param1 = GetExecModule().CreateArray(array, 4U, GetObjectAllocator());
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));

    GetExecModule().Execute();
    GetExecModule().FreeArray(param1);

    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 4U);
}

TEST_F(CodegenTest, Parameter)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).s16();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(8U, Opcode::Add).s16().Inputs(1U, 1U);
            INST(15U, Opcode::Return).u64().Inputs(6U);
            // Return parameter_0 + parameter_0
        }
    }
    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    auto graph = GetGraph();

    RegAlloc(graph);

    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    auto param1 = CutValue<uint64_t>(1234U, DataType::UINT64);
    auto param2 = CutValue<int16_t>(-1234L, DataType::INT16);
    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();

    auto retData = GetExecModule().GetRetValue();
    EXPECT_EQ(retData, 1234U + 1234U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

SRC_GRAPH(RegallocTwoFreeRegs, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);   // const a = 1
        CONSTANT(1U, 10U);  // const b = 10
        CONSTANT(2U, 20U);  // const c = 20

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{0U, 0U}, {3U, 7U}});                      // var a' = a
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {3U, 8U}});                      // var b' = b
            INST(5U, Opcode::Compare).b().CC(CC_NE).Inputs(4U, 0U);                        // b' == 1 ?
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);  // if == (b' == 1) -> exit
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(3U, 4U);  // a' = a' * b'
            INST(8U, Opcode::Sub).u64().Inputs(4U, 0U);  // b' = b' - 1
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);  // a' = c + a'
            INST(11U, Opcode::Return).u64().Inputs(10U);  // return a'
        }
    }
}

TEST_F(CodegenTest, RegallocTwoFreeRegs)
{
    src_graph::RegallocTwoFreeRegs::CREATE(GetGraph());
    // Create reg_mask with 5 available general registers,
    // 3 of them will be reserved by Reg Alloc.
    {
        RegAllocLinearScan ra(GetGraph());
        ra.SetRegMask(RegMask {0xFFFFF07FU});
        ra.SetVRegMask(VRegMask {0U});
        EXPECT_TRUE(ra.Run());
    }
    GraphChecker(GetGraph()).Check();
    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    auto param1 = CutValue<uint64_t>(0x0U, DataType::UINT64);
    auto param2 = CutValue<uint16_t>(0x0U, DataType::INT32);

    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();

    auto retData = GetExecModule().GetRetValue();
    EXPECT_TRUE(retData == 10U * 9U * 8U * 7U * 6U * 5U * 4U * 3U * 2U * 1U + 20U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

SRC_GRAPH(TwoFreeRegsAdditionSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(11U, 0U).f64();
        PARAMETER(12U, 0U).f32();
        CONSTANT(1U, 12U);
        CONSTANT(2U, -1L);
        CONSTANT(3U, 100000000U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(6U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(7U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(8U, Opcode::Sub).u64().Inputs(0U, 2U);
            INST(9U, Opcode::Sub).u64().Inputs(0U, 3U);
            INST(17U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(18U, Opcode::Sub).u64().Inputs(0U, 0U);
            INST(19U, Opcode::Add).u16().Inputs(0U, 1U);
            INST(20U, Opcode::Add).u16().Inputs(0U, 2U);
            INST(10U, Opcode::SaveState)
                .Inputs(0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 11U, 12U, 17U, 18U, 19U, 20U)
                .SrcVregs({0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U, 11U, 12U, 13U, 14U});
        }
    }
}

// NOTE (igorban): Update FillSaveStates() with filling SaveState from SpillFill
TEST_F(CodegenTest, DISABLED_TwoFreeRegsAdditionSaveState)
{
    src_graph::TwoFreeRegsAdditionSaveState::CREATE(GetGraph());
    // NO RETURN value - will droped down to SaveState block!

    SetNumVirtRegs(0U);
    SetNumArgs(3U);
    // Create reg_mask with 5 available general registers,
    // 3 of them will be reserved by Reg Alloc.
    {
        RegAllocLinearScan ra(GetGraph());
        ra.SetRegMask(RegMask {0xFFFFF07FU});
        ra.SetVRegMask(VRegMask {0U});
        EXPECT_TRUE(ra.Run());
    }
    GraphChecker(GetGraph()).Check();

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    GetExecModule().SetDump(false);

    auto param1 = CutValue<uint64_t>(0x12345U, DataType::UINT64);
    auto param2 = CutValue<float>(0x12345U, DataType::FLOAT32);

    GetExecModule().SetParameter(0U, param1);
    GetExecModule().SetParameter(1U, param2);
    GetExecModule().SetParameter(2U, param2);

    GetExecModule().Execute();

    // Main check - return value get from SaveState return DEOPTIMIZATION
    EXPECT_EQ(GetExecModule().GetRetValue(), 1U);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
}

SRC_GRAPH(SaveState, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).u64();  // index
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Add).u64().Inputs(6U, 6U);
            INST(8U, Opcode::StoreArray).u64().Inputs(3U, 5U, 7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(7U, 7U);  // Some return value
            INST(11U, Opcode::Return).u64().Inputs(10U);
        }
    }
}

TEST_F(CodegenTest, SaveState)
{
    src_graph::SaveState::CREATE(GetGraph());
    SetNumVirtRegs(0U);
    SetNumArgs(2U);

    auto graph = GetGraph();

    RegAlloc(graph);

    // Run codegen
    EXPECT_TRUE(RunCodegen(graph));
    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    auto codeExit = codeEntry + graph->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // Enable dumping
    GetExecModule().SetDump(false);

    uint64_t arrayData[4U];
    for (size_t i = 0; i < 4U; i++) {
        arrayData[i] = i + 0x20U;
    }
    auto param1 = GetExecModule().CreateArray(arrayData, 4U, GetObjectAllocator());
    auto param2 = CutValue<uint64_t>(1U, DataType::UINT64);
    GetExecModule().SetParameter(0U, reinterpret_cast<uint64_t>(param1));
    GetExecModule().SetParameter(1U, param2);

    GetExecModule().Execute();
    GetExecModule().SetDump(false);
    // End dump

    auto retData = GetExecModule().GetRetValue();
    // NOTE (igorban) : really need to check array changes
    EXPECT_EQ(retData, 4U * 0x21);

    // Clear data for next execution
    while (auto current = GetGraph()->GetFirstConstInst()) {
        GetGraph()->RemoveConstFromList(current);
    }
    GetExecModule().FreeArray(param1);
}  // namespace ark::compiler

TEST_F(CodegenTest, DeoptimizeIf)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).b();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::DeoptimizeIf).Inputs(0U, 2U);
            INST(4U, Opcode::Return).b().Inputs(0U);
        }
    }
    RegAlloc(GetGraph());

    EXPECT_TRUE(RunCodegen(GetGraph()));
    auto codeEntry = reinterpret_cast<char *>(GetGraph()->GetCode().Data());
    auto codeExit = codeEntry + GetGraph()->GetCode().Size();
    ASSERT(codeEntry != nullptr && codeExit != nullptr);
    GetExecModule().SetInstructions(codeEntry, codeExit);

    // param == false [OK]
    auto param = false;
    GetExecModule().SetParameter(0U, param);
    GetExecModule().Execute();
    EXPECT_EQ(GetExecModule().GetRetValue(), param);

    // param == true [INTERPRET]
}

static ArenaVector<std::pair<uint8_t, uint8_t>> ResoveParameterSequence(
    ArenaVector<std::pair<uint8_t, uint8_t>> *movedRegisters, uint8_t tmp, ArenaAllocator *allocator)
{
    constexpr uint8_t INVALID_FIRST = -1;
    constexpr uint8_t INVALID_SECOND = -2;

    movedRegisters->emplace_back(std::pair<uint8_t, uint8_t>(INVALID_FIRST, INVALID_SECOND));
    /*
        Example:
        1. mov x0 <- x3
        2. mov x1 <- x0
        3. mov x2 <- x3
        4. mov x3 <- x2
        Agreement - dst-s can't hold same registers multiple times (double move to one register)
                  - src for movs can hold same register multiply times

        Algorithm:
            1. Find handing edges (x1 - just in dst)
                emit "2. mov x1 <- x0"
                goto 1.
                emit "1. mov x0 <- x3"
            2. Assert all registers used just one time (loop from registers sequence)
               All multiply-definitions must be resolved on previous step
                emit ".. mov xtmp <- x2" (strore xtmp == x3)
                emit "3. mov x2 <- x3"
                emit "4. mov x3 <- xtmp" (ASSERT(4->GetReg == x3) - there is no other possible situations here)
    */
    // Calculate weigth
    ArenaVector<std::pair<uint8_t, uint8_t>> result(allocator->Adapter());
    // --moved_registers->end() - for remove marker-element
    for (auto pair = movedRegisters->begin(); pair != --movedRegisters->end();) {
        auto conflict = std::find_if(movedRegisters->begin(), movedRegisters->end(), [pair](auto inPair) {
            return (inPair.second == pair->first && (inPair != *pair));
        });
        if (conflict == movedRegisters->end()) {
            // emit immediate - there are no another possible combinations
            result.emplace_back(*pair);
            movedRegisters->erase(pair);
            pair = movedRegisters->begin();
        } else {
            ++pair;
        }
    }
    // Here just loops
    auto currPair = movedRegisters->begin();
    while (!(currPair->first == INVALID_FIRST && currPair->second == INVALID_SECOND)) {
        /* Need support single mov x1 <- x1:
           ASSERT(moved_registers->size() != 1);
        */

        auto savedReg = currPair->first;
        result.emplace_back(std::pair<uint8_t, uint8_t>(tmp, currPair->first));
        result.emplace_back(*currPair);  // we already save dst_register

        // Remove current instruction
        auto currReg = currPair->second;
        movedRegisters->erase(currPair);

        while (currPair != movedRegisters->end()) {
            currPair = std::find_if(movedRegisters->begin(), movedRegisters->end(),
                                    [currReg](auto inPair) { return inPair.first == currReg; });
            ASSERT(currPair != movedRegisters->end());
            if (currPair->second == savedReg) {
                result.emplace_back(std::pair<uint8_t, uint8_t>(currPair->first, tmp));
                movedRegisters->erase(currPair);
                break;
            };
            result.emplace_back(*currPair);
            currReg = currPair->second;
            movedRegisters->erase(currPair);
        }
        currPair = movedRegisters->begin();
    }
    movedRegisters->erase(currPair);

    return result;
}

TEST_F(CodegenTest, ResolveParamSequence1)
{
    ArenaVector<std::pair<uint8_t, uint8_t>> someSequence(GetAllocator()->Adapter());
    someSequence.emplace_back(std::pair<uint8_t, uint8_t>(0U, 3U));
    someSequence.emplace_back(std::pair<uint8_t, uint8_t>(1U, 0U));
    someSequence.emplace_back(std::pair<uint8_t, uint8_t>(2U, 3U));
    someSequence.emplace_back(std::pair<uint8_t, uint8_t>(3U, 2U));

    auto result = ResoveParameterSequence(&someSequence, 13U, GetAllocator());
    EXPECT_TRUE(someSequence.empty());
    ArenaVector<std::pair<uint8_t, uint8_t>> resultSequence(GetAllocator()->Adapter());
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(1U, 0U));
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(0U, 3U));
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(13U, 2U));
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(2U, 3U));
    resultSequence.emplace_back(std::pair<uint8_t, uint8_t>(3U, 13U));

    EXPECT_EQ(result, resultSequence);
}
TEST_F(CodegenTest, ResolveParamSequence2)
{
    // Special loop-only case
    ArenaVector<std::pair<uint8_t, uint8_t>> someSequenceLoop(GetAllocator()->Adapter());
    someSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(2U, 3U));
    someSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(1U, 2U));
    someSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(4U, 1U));
    someSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(0U, 4U));
    someSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(3U, 0U));

    auto resultLoop = ResoveParameterSequence(&someSequenceLoop, 13U, GetAllocator());
    EXPECT_TRUE(someSequenceLoop.empty());
    ArenaVector<std::pair<uint8_t, uint8_t>> resultSequenceLoop(GetAllocator()->Adapter());
    resultSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(13U, 2U));
    resultSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(2U, 3U));
    resultSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(3U, 0U));
    resultSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(0U, 4U));
    resultSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(4U, 1U));
    resultSequenceLoop.emplace_back(std::pair<uint8_t, uint8_t>(1U, 13U));

    EXPECT_EQ(resultLoop, resultSequenceLoop);
}

template <uint8_t REG_SIZE, uint8_t TMP_REG, typename T>
void CheckParamSequence(uint8_t j, const T &resultVect, const T &origVector)
{
    auto dst = resultVect[j].first;

    for (uint8_t k = j + 1U; k < REG_SIZE; ++k) {
        if (resultVect[k].second == dst && resultVect[k].second != TMP_REG) {
            std::cerr << " first = " << resultVect[k].first << " tmp = " << TMP_REG << "\n";
            std::cerr << " Before:\n";
            for (const auto &it : origVector) {
                std::cerr << " " << (size_t)it.first << "<-" << (size_t)it.second << "\n";
            }
            std::cerr << " After:\n";
            for (const auto &it : resultVect) {
                std::cerr << " " << (size_t)it.first << "<-" << (size_t)it.second << "\n";
            }
            std::cerr << "Fault on " << (size_t)j << " and " << (size_t)k << "\n";
            EXPECT_NE(resultVect[k].second, dst);
        }
    }
}

TEST_F(CodegenTest, ResolveParamSequence3)
{
    ArenaVector<std::pair<uint8_t, uint8_t>> someSequence(GetAllocator()->Adapter());
    ArenaVector<std::pair<uint8_t, uint8_t>> resultSequence(GetAllocator()->Adapter());
    constexpr uint8_t REG_SIZE = 30;
    constexpr uint8_t TMP_REG = REG_SIZE + 5U;
    for (uint64_t i = 0; i < ITERATION; ++i) {
        EXPECT_TRUE(someSequence.empty());

        std::vector<uint8_t> iters;
        for (uint8_t j = 0; j < REG_SIZE; ++j) {
            iters.push_back(j);
        }
        std::shuffle(iters.begin(), iters.end(), g_randomGenerator);
        std::vector<std::pair<uint8_t, uint8_t>> origVector;
        for (uint8_t j = 0; j < REG_SIZE; ++j) {
            auto gen {g_randomGenerator()};
            auto randomValue = gen % REG_SIZE;
            origVector.emplace_back(std::pair<uint8_t, uint8_t>(iters[j], randomValue));
        }
        for (auto &pair : origVector) {
            someSequence.emplace_back(pair);
        }
        resultSequence = ResoveParameterSequence(&someSequence, TMP_REG, GetAllocator());
        std::vector<std::pair<uint8_t, uint8_t>> resultVect;
        for (auto &pair : resultSequence) {
            resultVect.emplace_back(pair);
        }

        // First analysis - there are no dst before src
        for (uint8_t j = 0; j < REG_SIZE; ++j) {
            CheckParamSequence<REG_SIZE, TMP_REG>(j, resultVect, origVector);
        }

        // Second analysis - if remove all same moves - there will be
        // only " move tmp <- reg " & " mov reg <- tmp"
    }
}
// NOLINTEND(readability-magic-numbers,modernize-avoid-c-arrays,cppcoreguidelines-pro-bounds-pointer-arithmetic)

}  // namespace ark::compiler
