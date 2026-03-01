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

#ifndef BYTECODE_OPTIMIZER_TESTS_COMMON_H
#define BYTECODE_OPTIMIZER_TESTS_COMMON_H

#include <gtest/gtest.h>
#include <cstdint>
#include <string>
#include <string_view>

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "assembler/assembly-program.h"
#include "assembler/extensions/extensions.h"
#include "canonicalization.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "codegen.h"
#include "compiler/compiler_options.h"
#include "compiler/optimizer/analysis/rpo.h"
#include "compiler/optimizer/ir/datatype.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/ir/ir_constructor.h"
#include "compiler/optimizer/ir_builder/ir_builder.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "libarkfile/file_items.h"
#include "ir_interface.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/utils/utils.h"
#include "libarkbase/mem/arena_allocator.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkfile/method_data_accessor-inl.h"
#include "optimize_bytecode.h"
#include "reg_encoder.h"
#include "runtime_adapter.h"

namespace ark::bytecodeopt {

using compiler::BasicBlock;
using compiler::Graph;
using compiler::Input;
using compiler::Inst;
using compiler::Opcode;

struct RuntimeInterfaceMock : public compiler::RuntimeInterface {
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    size_t argumentCount {0};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    bool isConstructor {true};

    explicit RuntimeInterfaceMock(size_t argCount) : RuntimeInterfaceMock(argCount, true) {}

    RuntimeInterfaceMock(size_t argCount, bool isCtor) : argumentCount(argCount), isConstructor(isCtor) {}

    size_t GetMethodTotalArgumentsCount([[maybe_unused]] MethodPtr method) const override
    {
        return argumentCount;
    }

    bool IsConstructor([[maybe_unused]] MethodPtr method, [[maybe_unused]] SourceLanguage lang) override
    {
        return isConstructor;
    }
};

class IrInterfaceTest : public BytecodeOptIrInterface {
public:
    explicit IrInterfaceTest(pandasm::Program *prog = nullptr,
                             const pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps = nullptr)
        : BytecodeOptIrInterface(maps, prog)
    {
    }

    std::string GetFieldIdByOffset([[maybe_unused]] uint32_t offset) const override
    {
        return "";
    }

    std::string GetTypeIdByOffset([[maybe_unused]] uint32_t offset) const override
    {
        return IsMapsSet() ? BytecodeOptIrInterface::GetTypeIdByOffset(offset) : "";
    }

    std::string GetMethodIdByOffset([[maybe_unused]] uint32_t offset) const override
    {
        return "";
    }

    std::string GetStringIdByOffset([[maybe_unused]] uint32_t offset) const override
    {
        return "";
    }

    bool IsMethodStatic(uint32_t offset) const override
    {
        return IsMapsSet() && BytecodeOptIrInterface::IsMethodStatic(offset);
    }
};

namespace test {

extern std::string g_globArgV0;

}  // namespace test

class CommonTest : public ::testing::Test {
public:
    CommonTest()
    {
        compiler::g_options.SetCompilerUseSafepoint(false);
        compiler::g_options.SetCompilerSupportInitObjectInst(true);

        // NOLINTNEXTLINE(readability-magic-numbers)
        mem::MemConfig::Initialize(128_MB, 64_MB, 64_MB, 32_MB, 0, 0);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_INTERNAL);
        localAllocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_INTERNAL);
        builder_ = new compiler::IrConstructor();

        Logger::InitializeStdLogging(Logger::Level::ERROR,
                                     ark::Logger::ComponentMask().set(Logger::Component::BYTECODE_OPTIMIZER));
    }
    ~CommonTest() override
    {
        delete allocator_;
        delete localAllocator_;
        delete builder_;
        PoolManager::Finalize();
        mem::MemConfig::Finalize();

        Logger::Destroy();
    }

    NO_COPY_SEMANTIC(CommonTest);
    NO_MOVE_SEMANTIC(CommonTest);

    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }
    ArenaAllocator *GetLocalAllocator()
    {
        return localAllocator_;
    }

    compiler::Graph *CreateEmptyGraph(bool isDynamic = false)
    {
        auto *graph =
            GetAllocator()->New<compiler::Graph>(GetAllocator(), GetLocalAllocator(), Arch::NONE, isDynamic, true);
        return graph;
    }

    compiler::Graph *GetGraph()
    {
        return graph_;
    }

    void SetGraph(compiler::Graph *graph)
    {
        graph_ = graph;
    }

    bool FuncHasInst(pandasm::Function *func, pandasm::Opcode opcode)
    {
        for (const auto &inst : func->ins) {
            if (inst.opcode == opcode) {
                return true;
            }
        }
        return false;
    }

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    compiler::IrConstructor *builder_;

private:
    ArenaAllocator *allocator_;
    ArenaAllocator *localAllocator_;
    compiler::Graph *graph_ {nullptr};
};

class AsmTest : public CommonTest {
public:
    bool ParseToGraph(const std::string &source, const std::string &funcName, const char *fileName = "test.pb")
    {
        ark::pandasm::Parser parser;
        auto res = parser.Parse(source, fileName);
        if (parser.ShowError().err != pandasm::Error::ErrorType::ERR_NONE) {
            std::cerr << "Parse failed: " << parser.ShowError().message << std::endl
                      << parser.ShowError().wholeLine << std::endl;
            ADD_FAILURE();
            return false;
        }
        auto &prog = res.Value();
        return ParseToGraph(&prog, funcName);
    }

    virtual bool ParseToGraph(pandasm::Program *prog, const std::string &funcName)
    {
        pfile_ = pandasm::AsmEmitter::Emit(*prog, &maps_);
        irInterface_ = std::make_unique<bytecodeopt::BytecodeOptIrInterface>(&maps_, prog);

        if (pfile_ == nullptr) {
            ADD_FAILURE();
            return false;
        }

        auto ptrFile = pfile_.get();
        if (ptrFile == nullptr) {
            ADD_FAILURE();
            return false;
        }

        compiler::Graph *tempGraph = nullptr;

        for (uint32_t id : ptrFile->GetClasses()) {
            panda_file::File::EntityId recordId {id};

            if (ptrFile->IsExternal(recordId)) {
                continue;
            }

            panda_file::ClassDataAccessor cda {*ptrFile, recordId};
            cda.EnumerateMethods([&tempGraph, ptrFile, funcName, this](panda_file::MethodDataAccessor &mda) {
                auto nameId = mda.GetNameId();
                auto str = ptrFile->GetStringData(nameId).data;
                bool isEqual = (std::string(funcName) == std::string(reinterpret_cast<const char *>(str)));
                auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());

                if (!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative() && isEqual) {
                    auto adapter = allocator_.New<BytecodeOptimizerRuntimeAdapter>(mda.GetPandaFile());
                    tempGraph = allocator_.New<compiler::Graph>(
                        compiler::Graph::GraphArgs {&allocator_, &localAllocator_, Arch::NONE, methodPtr, adapter},
                        nullptr, false, false, true);
                    ASSERT_NE(tempGraph, nullptr);
                    ASSERT_TRUE(tempGraph->RunPass<compiler::IrBuilder>());
                }
            });
        }

        if (tempGraph != nullptr) {
            SetGraph(tempGraph);
            return true;
        }
        return false;
    }

    bytecodeopt::BytecodeOptIrInterface *GetIrInterface()
    {
        return irInterface_.get();
    }

    pandasm::AsmEmitter::PandaFileToPandaAsmMaps *GetMaps()
    {
        return &maps_;
    }

    auto GetFile()
    {
        return pfile_.get();
    }

private:
    std::unique_ptr<BytecodeOptIrInterface> irInterface_;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps_;
    ArenaAllocator allocator_ {SpaceType::SPACE_TYPE_COMPILER};
    ArenaAllocator localAllocator_ {SpaceType::SPACE_TYPE_COMPILER};
    std::unique_ptr<const panda_file::File> pfile_ {nullptr};
};

class GraphComparator {
public:
    bool Compare(Graph *graph1, Graph *graph2)
    {
        graph1->InvalidateAnalysis<compiler::Rpo>();
        graph2->InvalidateAnalysis<compiler::Rpo>();
        if (graph1->GetBlocksRPO().size() != graph2->GetBlocksRPO().size()) {
            std::cerr << "Different number of blocks: " << graph1->GetBlocksRPO().size() << " and "
                      << graph2->GetBlocksRPO().size() << std::endl;
            return false;
        }
        return std::equal(graph1->GetBlocksRPO().begin(), graph1->GetBlocksRPO().end(), graph2->GetBlocksRPO().begin(),
                          graph2->GetBlocksRPO().end(), [this](auto bb1, auto bb2) { return Compare(bb1, bb2); });
    }

    bool Compare(BasicBlock *block1, BasicBlock *block2)
    {
        if (block1->GetPredsBlocks().size() != block2->GetPredsBlocks().size()) {
            std::cerr << "Different number of preds blocks\n";
            block1->Dump(&std::cout);
            block2->Dump(&std::cout);
            return false;
        }
        if (block1->GetSuccsBlocks().size() != block2->GetSuccsBlocks().size()) {
            std::cerr << "Different number of succs blocks\n";
            block1->Dump(&std::cout);
            block2->Dump(&std::cout);
            return false;
        }

        auto comparator = [this](auto inst1, auto inst2) {
            ASSERT(inst2 != nullptr);
            bool t = Compare(inst1, inst2);
            if (!t) {
                std::cerr << "Different instructions:\n";
                inst1->Dump(&std::cout);
                inst2->Dump(&std::cout);
            }
            return t;
        };
        return std::equal(block1->AllInsts().begin(), block1->AllInsts().end(), block2->AllInsts().begin(),
                          block2->AllInsts().end(), comparator);
    }

    // CC-OFFNXT(huge_method[C], G.FUN.01-CPP) solid logic
    bool Compare(Inst *inst1, Inst *inst2)
    {
        if (auto it = instCompareMap_.insert({inst1, inst2}); !it.second) {
            if (inst2 == it.first->second) {
                return true;
            }
            instCompareMap_.erase(inst1);
            return false;
        }

        if (inst1->GetOpcode() != inst2->GetOpcode() || inst1->GetType() != inst2->GetType() ||
            inst1->GetInputsCount() != inst2->GetInputsCount()) {
            instCompareMap_.erase(inst1);
            return false;
        }

        if (inst1->GetOpcode() == Opcode::Intrinsic || inst1->GetOpcode() == Opcode::Builtin) {
            if (inst1->CastToIntrinsic()->GetIntrinsicId() != inst2->CastToIntrinsic()->GetIntrinsicId()) {
                instCompareMap_.erase(inst1);
                return false;
            }
        }

        if (inst1->GetOpcode() != Opcode::Phi) {
            bool equal =
                std::equal(inst1->GetInputs().begin(), inst1->GetInputs().end(), inst2->GetInputs().begin(),
                           [this](Input input1, Input input2) { return Compare(input1.GetInst(), input2.GetInst()); });
            if (!equal) {
                instCompareMap_.erase(inst1);
                return false;
            }
        } else {
            for (auto input1 : inst1->GetInputs()) {
                auto it =
                    std::find_if(inst2->GetInputs().begin(), inst2->GetInputs().end(),
                                 [this, &input1](Input input2) { return Compare(input1.GetInst(), input2.GetInst()); });
                if (it == inst2->GetInputs().end()) {
                    instCompareMap_.erase(inst1);
                    return false;
                }
            }
        }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CAST(Opc) CastTo##Opc()

// CC-OFFNXT(G.PRE.02-CPP) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_OPC(Opc, Getter)                                                                               \
    do {                                                                                                     \
        /* CC-OFFNXT(G.PRE.02) false positive */                                                             \
        if (inst1->GetOpcode() == Opcode::Opc && inst1->CAST(Opc)->Getter() != inst2->CAST(Opc)->Getter()) { \
            instCompareMap_.erase(inst1);                                                                    \
            return false; /* CC-OFF(G.PRE.05) function gen */                                                \
        }                                                                                                    \
    } while (0)

        CHECK_OPC(Constant, GetRawValue);

        CHECK_OPC(Cast, GetOperandsType);
        CHECK_OPC(Cmp, GetOperandsType);

        CHECK_OPC(Compare, GetCc);
        CHECK_OPC(Compare, GetOperandsType);

        CHECK_OPC(If, GetCc);
        CHECK_OPC(If, GetOperandsType);

        CHECK_OPC(IfImm, GetCc);
        CHECK_OPC(IfImm, GetImm);
        CHECK_OPC(IfImm, GetOperandsType);

        CHECK_OPC(LoadArrayI, GetImm);
        CHECK_OPC(LoadArrayPairI, GetImm);
        CHECK_OPC(LoadPairPart, GetImm);
        CHECK_OPC(StoreArrayI, GetImm);
        CHECK_OPC(StoreArrayPairI, GetImm);
        CHECK_OPC(BoundsCheckI, GetImm);
        CHECK_OPC(ReturnI, GetImm);
        CHECK_OPC(AddI, GetImm);
        CHECK_OPC(SubI, GetImm);
        CHECK_OPC(ShlI, GetImm);
        CHECK_OPC(ShrI, GetImm);
        CHECK_OPC(AShrI, GetImm);
        CHECK_OPC(AndI, GetImm);
        CHECK_OPC(OrI, GetImm);
        CHECK_OPC(XorI, GetImm);

        CHECK_OPC(LoadStatic, GetVolatile);
        CHECK_OPC(StoreStatic, GetVolatile);
        CHECK_OPC(LoadObject, GetVolatile);
        CHECK_OPC(StoreObject, GetVolatile);
#undef CHECK_OPC
#undef CAST

        if (inst1->GetOpcode() == Opcode::Cmp && IsFloatType(inst1->GetInput(0).GetInst()->GetType())) {
            auto cmp1 = static_cast<compiler::CmpInst *>(inst1);
            auto cmp2 = static_cast<compiler::CmpInst *>(inst2);
            if (cmp1->IsFcmpg() != cmp2->IsFcmpg()) {
                instCompareMap_.erase(inst1);
                return false;
            }
        }
        for (uint32_t i = 0; i < inst2->GetInputsCount(); i++) {
            if (inst1->GetInputType(i) != inst2->GetInputType(i)) {
                instCompareMap_.erase(inst1);
                return false;
            }
        }
        return true;
    }

private:
    std::unordered_map<Inst *, Inst *> instCompareMap_;
};

// CC-OFFNXT(G.FUD.06) big switch case
inline std::string JccMnemonic(compiler::ConditionCode cc)
{
    switch (cc) {
        case compiler::ConditionCode::CC_EQ:
            return "jeq";
        case compiler::ConditionCode::CC_NE:
            return "jne";
        case compiler::ConditionCode::CC_LT:
            return "jlt";
        case compiler::ConditionCode::CC_GT:
            return "jgt";
        case compiler::ConditionCode::CC_LE:
            return "jle";
        case compiler::ConditionCode::CC_GE:
            return "jge";
        default:
            UNREACHABLE();
    }
}

inline std::string JcczMnemonic(compiler::ConditionCode cc)
{
    return JccMnemonic(cc) + "z";
}

class IrBuilderTest : public AsmTest {
public:
    void CheckSimple(const std::string &instName, compiler::DataType::Type dataType, const std::string &instType)
    {
        ASSERT(instName == "mov" || instName == "lda" || instName == "sta");
        std::string currType;
        if (dataType == compiler::DataType::Type::REFERENCE) {
            currType = "i64[]";
        } else {
            currType = ToString(dataType);
        }

        std::string source = ".function " + currType + " main(";
        source += currType + " a0){\n";
        if (instName == "mov") {
            source += "mov" + instType + " v0, a0\n";
            source += "lda" + instType + " v0\n";
        } else if (instName == "lda") {
            source += "lda" + instType + " a0\n";
        } else if (instName == "sta") {
            source += "lda" + instType + " a0\n";
            source += "sta" + instType + " v0\n";
            source += "lda" + instType + " v0\n";
        } else {
            UNREACHABLE();
        }
        source += "return" + instType + "\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(dataType);

            BASIC_BLOCK(2U, -1)
            {
                INST(1, Opcode::Return).Inputs(0);
                INS(1).SetType(dataType);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckSimpleWithImm(const std::string &instName, compiler::DataType::Type dataType, const std::string &instType)
    {
        ASSERT(instName == "mov" || instName == "fmov" || instName == "lda" || instName == "flda");
        std::string currType = ToString(dataType);

        std::string source = ".function " + currType + " main(){\n";
        if (instName == "mov") {
            source += "movi" + instType + " v0, 0\n";
            source += "lda" + instType + " v0\n";
        } else if (instName == "fmov") {
            source += "fmovi" + instType + " v0, 0.\n";
            source += "lda" + instType + " v0\n";
        } else if (instName == "lda") {
            source += "ldai" + instType + " 0\n";
        } else if (instName == "flda") {
            source += "fldai" + instType + " 0.\n";
        } else {
            UNREACHABLE();
        }
        source += "return" + instType + "\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();

        GRAPH(graph)
        {
            CONSTANT(0, 0);
            INS(0).SetType(dataType);

            BASIC_BLOCK(2U, -1)
            {
                INST(1, Opcode::Return).Inputs(0);
                INS(1).SetType(dataType);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckCmp(const std::string &instName, compiler::DataType::Type dataType, const std::string &instType)
    {
        ASSERT(instName == "ucmp" || instName == "fcmpl" || instName == "fcmpg");
        std::string currType;
        if (dataType == compiler::DataType::Type::REFERENCE) {
            currType = "i64[]";
        } else {
            currType = ToString(dataType);
        }
        std::string source = ".function i32 main(";
        source += currType + " a0, ";
        source += currType + " a1){\n";
        source += "lda" + instType + " a0\n";
        source += instName + instType + " a1\n";
        source += "return\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(dataType);
            PARAMETER(1, 1);
            INS(1).SetType(dataType);

            BASIC_BLOCK(2U, -1)
            {
                INST(2U, Opcode::Cmp).s32().Inputs(0, 1);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckFloatCmp(const std::string &instName, compiler::DataType::Type dataType, const std::string &instType,
                       bool fcmpg)
    {
        ASSERT(instName == "fcmpl" || instName == "fcmpg");
        std::string currType = ToString(dataType);

        std::string source = ".function i32 main(";
        source += currType + " a0, ";
        source += currType + " a1){\n";
        source += "lda" + instType + " a0\n";
        source += instName + instType + " a1\n";
        source += "return\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(dataType);
            PARAMETER(1, 1);
            INS(1).SetType(dataType);

            BASIC_BLOCK(2U, -1)
            {
                INST(2U, Opcode::Cmp).s32().SrcType(dataType).Fcmpg(fcmpg).Inputs(0, 1);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    template <bool IS_OBJ>
    void CheckCondJumpWithZero(compiler::ConditionCode cc)
    {
        std::string cmd = JcczMnemonic(cc);
        std::string instPostfix {};
        std::string paramType = "i32";
        auto type = compiler::DataType::INT32;
        if constexpr (IS_OBJ) {
            instPostfix = ".obj";
            paramType = "i64[]";
            type = compiler::DataType::REFERENCE;
        }

        std::string source = ".function void main(";
        source += paramType + " a0) {\n";
        source += "lda" + instPostfix + " a0\n";
        source += cmd + instPostfix + " label\n";
        source += "label: ";
        source += "return.void\n}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(2U, 0).s64();

            BASIC_BLOCK(2U, 3_I, 4_I)
            {
                INST(1U, Opcode::Compare).b().CC(cc).Inputs(0, 2_I);
                INST(3U, Opcode::IfImm)
                    .SrcType(compiler::DataType::BOOL)
                    .CC(compiler::ConditionCode::CC_NE)
                    .Inputs(1)
                    .Imm(0);
            }
            BASIC_BLOCK(3U, 4_I) {}
            BASIC_BLOCK(4U, -1)
            {
                INST(4U, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    template <bool IS_OBJ>
    void CheckCondJump(compiler::ConditionCode cc)
    {
        std::string cmd = JccMnemonic(cc);
        std::string instPostfix {};
        std::string paramType = "i32";
        auto type = compiler::DataType::INT32;
        if constexpr (IS_OBJ) {
            instPostfix = ".obj";
            paramType = "i64[]";
            type = compiler::DataType::REFERENCE;
        }

        std::string source = ".function void main(";
        source += paramType + " a0, " + paramType + " a1) {\n";
        source += "lda" + instPostfix + " a0\n";
        source += cmd + instPostfix + " a1, label\n";
        source += "label: ";
        source += "return.void\n}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            PARAMETER(1, 1);
            INS(1).SetType(type);

            BASIC_BLOCK(2U, 3_I, 4_I)
            {
                INST(2U, Opcode::Compare).b().CC(cc).Inputs(0, 1);
                INST(3U, Opcode::IfImm)
                    .SrcType(compiler::DataType::BOOL)
                    .CC(compiler::ConditionCode::CC_NE)
                    .Imm(0)
                    .Inputs(2U);
            }
            BASIC_BLOCK(3U, 4_I) {}
            BASIC_BLOCK(4U, -1)
            {
                INST(4U, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckOtherPasses(ark::pandasm::Program *prog, const std::string &funcName, bool isStatic)
    {
        GetGraph()->RunPass<compiler::Cleanup>();
        GetGraph()->RunPass<Canonicalization>();
#ifndef NDEBUG
        GetGraph()->SetLowLevelInstructionsEnabled();
#endif
        GetGraph()->RunPass<compiler::Cleanup>();
        GetGraph()->RunPass<compiler::Lowering>();
        GetGraph()->RunPass<compiler::Cleanup>();
        EXPECT_TRUE(GetGraph()->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        GetGraph()->RunPass<compiler::Cleanup>();
        EXPECT_TRUE(GetGraph()->RunPass<RegEncoder>());

        auto &functionTable = isStatic ? prog->functionStaticTable : prog->functionInstanceTable;
        ASSERT_TRUE(functionTable.find(funcName) != functionTable.end());
        auto function = const_cast<pandasm::Function *>(&functionTable.at(funcName));
        GetGraph()->RunPass<compiler::Cleanup>();
        EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(function, GetIrInterface()));
        auto pf = pandasm::AsmEmitter::Emit(*prog);
        ASSERT_NE(pf, nullptr);
    }

    void CheckConstArrayFilling(ark::pandasm::Program *prog, [[maybe_unused]] const std::string &className,
                                const std::string &funcName, bool isStatic)
    {
        if (prog->literalarrayTable.size() == 1) {
            EXPECT_TRUE(prog->literalarrayTable["0"].literals[0U].tag == panda_file::LiteralTag::TAGVALUE);
            EXPECT_TRUE(prog->literalarrayTable["0"].literals[1U].tag == panda_file::LiteralTag::INTEGER);
            EXPECT_TRUE(prog->literalarrayTable["0"].literals[2U].tag == panda_file::LiteralTag::ARRAY_I32);
            return;
        }
        EXPECT_TRUE(prog->literalarrayTable.size() == 8U);
        for (const auto &elem : prog->literalarrayTable) {
            EXPECT_TRUE(elem.second.literals.size() == 5U);
            EXPECT_TRUE(elem.second.literals[0U].tag == panda_file::LiteralTag::TAGVALUE);
            EXPECT_TRUE(elem.second.literals[1U].tag == panda_file::LiteralTag::INTEGER);
        }
        EXPECT_TRUE(prog->literalarrayTable["7"].literals[2U].tag == panda_file::LiteralTag::ARRAY_U1);
        EXPECT_TRUE(prog->literalarrayTable["6"].literals[2U].tag == panda_file::LiteralTag::ARRAY_I8);
        EXPECT_TRUE(prog->literalarrayTable["5"].literals[2U].tag == panda_file::LiteralTag::ARRAY_I16);
        EXPECT_TRUE(prog->literalarrayTable["4"].literals[2U].tag == panda_file::LiteralTag::ARRAY_I32);
        EXPECT_TRUE(prog->literalarrayTable["3"].literals[2U].tag == panda_file::LiteralTag::ARRAY_I64);
        EXPECT_TRUE(prog->literalarrayTable["2"].literals[2U].tag == panda_file::LiteralTag::ARRAY_F32);
        EXPECT_TRUE(prog->literalarrayTable["1"].literals[2U].tag == panda_file::LiteralTag::ARRAY_F64);
        EXPECT_TRUE(prog->literalarrayTable["0"].literals[2U].tag == panda_file::LiteralTag::ARRAY_STRING);

        EXPECT_TRUE(GetGraph()->RunPass<RegEncoder>());

        auto &functionTable = isStatic ? prog->functionStaticTable : prog->functionInstanceTable;
        ASSERT_TRUE(functionTable.find(funcName) != functionTable.end());
        auto function = const_cast<pandasm::Function *>(&functionTable.at(funcName));
        EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(function, GetIrInterface()));
        ASSERT(pandasm::AsmEmitter::Emit(className + ".panda", *prog, nullptr, nullptr, false));
    }

    enum CheckConstArrayTypes { ACCESS, SKIP_MULTIDIM_ARRAYS };

    static compiler::Inst *CheckAllInsts(const BasicBlock *bb, CheckConstArrayTypes type,
                                         compiler::Inst *constArrayDefInst)
    {
        for (auto inst : bb->AllInsts()) {
            switch (type) {
                case CheckConstArrayTypes::ACCESS: {
                    if (inst->GetOpcode() == Opcode::LoadConstArray) {
                        constArrayDefInst = inst;
                        continue;
                    }
                    if (inst->GetOpcode() == Opcode::LoadArray) {
                        EXPECT_TRUE(constArrayDefInst != nullptr);
                        EXPECT_TRUE(inst->CastToLoadArray()->GetArray() == constArrayDefInst);
                    }
                    continue;
                }
                case CheckConstArrayTypes::SKIP_MULTIDIM_ARRAYS: {
                    EXPECT_TRUE(inst->GetOpcode() != Opcode::LoadConstArray);
                    continue;
                }
                default:
                    UNREACHABLE();
            }
        }

        return constArrayDefInst;
    }

    void CheckConstArray(ark::pandasm::Program *prog, const char *className, const std::string &funcName,
                         CheckConstArrayTypes type, bool isStatic)
    {
        g_options.SetConstArrayResolver(true);

        ark::pandasm::AsmEmitter::Emit(std::string(className) + ".panda", *prog, nullptr, nullptr, false);
        auto tempName = funcName.substr(funcName.find('.') + 1);
        EXPECT_TRUE(ParseToGraph(prog, tempName.substr(0, tempName.find(':'))));
        EXPECT_TRUE(RunOptimizations(GetGraph(), GetIrInterface()));

        compiler::Inst *constArrayDefInst {nullptr};
        for (auto bb : GetGraph()->GetBlocksRPO()) {
            constArrayDefInst = CheckAllInsts(bb, type, constArrayDefInst);
        }

        EXPECT_TRUE(GetGraph()->RunPass<RegEncoder>());

        auto &functionTable = isStatic ? prog->functionStaticTable : prog->functionInstanceTable;
        ASSERT_TRUE(functionTable.find(funcName) != functionTable.end());
        auto function = const_cast<pandasm::Function *>(&functionTable.at(funcName));
        EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(function, GetIrInterface()));
        ASSERT(pandasm::AsmEmitter::Emit("LiteralArrayIntAccess.panda", *prog, nullptr, nullptr, false));
    }

    void CheckVirtualCallsNoExist(const int expStaticCallsCount)
    {
        uint16_t staticCallCount = 0;
        for (auto block : GetGraph()->GetVectorBlocks()) {
            for (auto inst : block->AllInsts()) {
                auto u = inst->GetFirstUser();
                if (u != nullptr) {
                    auto intsOpcode = u->GetInst()->GetOpcode();
                    ASSERT_FALSE(intsOpcode == Opcode::CallVirtual);
                    staticCallCount = (intsOpcode == Opcode::CallStatic) ? (staticCallCount + 1) : staticCallCount;
                }
            }
        }
        ASSERT_TRUE(staticCallCount == expStaticCallsCount);
    }
};

}  // namespace ark::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_TESTS_COMMON_H
