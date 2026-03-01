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
#include <string>
#include <string_view>

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "assembler/assembly-program.h"
#include "assembler/extensions/extensions.h"
#include "canonicalization.h"
#include "class_data_accessor-inl.h"
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
#include "file_items.h"
#include "ir_interface.h"
#include "libpandabase/utils/logger.h"
#include "mem/arena_allocator.h"
#include "mem/pool_manager.h"
#include "method_data_accessor-inl.h"
#include "optimize_bytecode.h"
#include "reg_encoder.h"
#include "runtime_adapter.h"

namespace panda::bytecodeopt {

using compiler::BasicBlock;
using compiler::Graph;
using compiler::Input;
using compiler::Inst;
using compiler::Opcode;

struct RuntimeInterfaceMock : public compiler::RuntimeInterface {
    size_t argument_count {0};
    bool is_constructor {true};

    explicit RuntimeInterfaceMock(size_t arg_count) : RuntimeInterfaceMock(arg_count, true) {}

    RuntimeInterfaceMock(size_t arg_count, bool is_ctor) : argument_count(arg_count), is_constructor(is_ctor) {}

    size_t GetMethodTotalArgumentsCount([[maybe_unused]] MethodPtr method) const override
    {
        return argument_count;
    }

    bool IsConstructor([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t class_id) override
    {
        return is_constructor;
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
};

namespace test {

extern std::string glob_argv0;

}  // namespace test

class CommonTest : public ::testing::Test {
public:
    CommonTest()
    {
        compiler::options.SetCompilerUseSafepoint(false);
        compiler::options.SetCompilerSupportInitObjectInst(true);

        mem::MemConfig::Initialize(128_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
        allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_INTERNAL);
        local_allocator_ = new ArenaAllocator(SpaceType::SPACE_TYPE_INTERNAL);
        builder_ = new compiler::IrConstructor();

        Logger::InitializeStdLogging(Logger::Level::ERROR,
                                     panda::Logger::ComponentMask().set(Logger::Component::BYTECODE_OPTIMIZER));
    }
    virtual ~CommonTest()
    {
        delete allocator_;
        delete local_allocator_;
        delete builder_;
        PoolManager::Finalize();
        mem::MemConfig::Finalize();

        Logger::Destroy();
    }
    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }
    ArenaAllocator *GetLocalAllocator()
    {
        return local_allocator_;
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
    compiler::IrConstructor *builder_;

private:
    ArenaAllocator *allocator_;
    ArenaAllocator *local_allocator_;
    compiler::Graph *graph_ {nullptr};
};

class AsmTest : public CommonTest {
public:
    bool ParseToGraph(const std::string &source, const std::string &func_name, const char *file_name = "test.pb")
    {
        panda::pandasm::Parser parser;
        auto res = parser.Parse(source, file_name);
        if (parser.ShowError().err != pandasm::Error::ErrorType::ERR_NONE) {
            std::cerr << "Parse failed: " << parser.ShowError().message << std::endl
                      << parser.ShowError().whole_line << std::endl;
            ADD_FAILURE();
            return false;
        }
        auto &prog = res.Value();
        return ParseToGraph(&prog, func_name);
    }

    bool ParseToGraph(pandasm::Program *prog, const std::string &func_name)
    {
        pfile_ = pandasm::AsmEmitter::Emit(*prog, &maps_);
        ir_interface_ = std::make_unique<bytecodeopt::BytecodeOptIrInterface>(&maps_, prog);

        if (pfile_ == nullptr) {
            ADD_FAILURE();
            return false;
        }

        auto ptr_file = pfile_.get();
        if (ptr_file == nullptr) {
            ADD_FAILURE();
            return false;
        }

        compiler::Graph *temp_graph = nullptr;

        for (uint32_t id : ptr_file->GetClasses()) {
            panda_file::File::EntityId record_id {id};

            if (ptr_file->IsExternal(record_id)) {
                continue;
            }

            panda_file::ClassDataAccessor cda {*ptr_file, record_id};
            cda.EnumerateMethods([&temp_graph, ptr_file, func_name, this](panda_file::MethodDataAccessor &mda) {
                auto name_id = mda.GetNameId();
                auto str = ptr_file->GetStringData(name_id).data;
                bool is_equal = (std::string(func_name) == std::string(reinterpret_cast<const char *>(str)));
                auto method_ptr =
                    reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(mda.GetMethodId().GetOffset());

                if (!mda.IsExternal() && !mda.IsAbstract() && !mda.IsNative() && is_equal) {
                    auto adapter = allocator_.New<BytecodeOptimizerRuntimeAdapter>(mda.GetPandaFile());
                    temp_graph = allocator_.New<compiler::Graph>(&allocator_, &local_allocator_, Arch::NONE, method_ptr,
                                                                 adapter, false, nullptr, false, true);
                    ASSERT_NE(temp_graph, nullptr);
                    ASSERT_TRUE(temp_graph->RunPass<compiler::IrBuilder>());
                }
            });
        }

        if (temp_graph != nullptr) {
            SetGraph(temp_graph);
            return true;
        }
        return false;
    }

    bytecodeopt::BytecodeOptIrInterface *GetIrInterface()
    {
        return ir_interface_.get();
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
    std::unique_ptr<BytecodeOptIrInterface> ir_interface_;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps_;
    ArenaAllocator allocator_ {SpaceType::SPACE_TYPE_COMPILER};
    ArenaAllocator local_allocator_ {SpaceType::SPACE_TYPE_COMPILER};
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
        return std::equal(block1->AllInsts().begin(), block1->AllInsts().end(), block2->AllInsts().begin(),
                          block2->AllInsts().end(), [this](auto inst1, auto inst2) {
                              ASSERT(inst2 != nullptr);
                              bool t = Compare(inst1, inst2);
                              if (!t) {
                                  std::cerr << "Different instructions:\n";
                                  inst1->Dump(&std::cout);
                                  inst2->Dump(&std::cout);
                              }
                              return t;
                          });
    }

    bool Compare(Inst *inst1, Inst *inst2)
    {
        if (auto it = inst_compare_map_.insert({inst1, inst2}); !it.second) {
            if (inst2 == it.first->second) {
                return true;
            }
            inst_compare_map_.erase(inst1);
            return false;
        }

        if (inst1->GetOpcode() != inst2->GetOpcode() || inst1->GetType() != inst2->GetType() ||
            inst1->GetInputsCount() != inst2->GetInputsCount()) {
            inst_compare_map_.erase(inst1);
            return false;
        }

        if (inst1->GetOpcode() == Opcode::Intrinsic || inst1->GetOpcode() == Opcode::Builtin) {
            if (inst1->CastToIntrinsic()->GetIntrinsicId() != inst2->CastToIntrinsic()->GetIntrinsicId()) {
                inst_compare_map_.erase(inst1);
                return false;
            }
        }

        if (inst1->GetOpcode() != Opcode::Phi) {
            if (!std::equal(
                    inst1->GetInputs().begin(), inst1->GetInputs().end(), inst2->GetInputs().begin(),
                    [this](Input input1, Input input2) { return Compare(input1.GetInst(), input2.GetInst()); })) {
                inst_compare_map_.erase(inst1);
                return false;
            }
        } else {
            for (auto input1 : inst1->GetInputs()) {
                auto it =
                    std::find_if(inst2->GetInputs().begin(), inst2->GetInputs().end(),
                                 [this, &input1](Input input2) { return Compare(input1.GetInst(), input2.GetInst()); });
                if (it == inst2->GetInputs().end()) {
                    inst_compare_map_.erase(inst1);
                    return false;
                }
            }
        }

#define CAST(Opc) CastTo##Opc()

#define CHECK(Opc, Getter)                                                                               \
    if (inst1->GetOpcode() == Opcode::Opc && inst1->CAST(Opc)->Getter() != inst2->CAST(Opc)->Getter()) { \
        inst_compare_map_.erase(inst1);                                                                  \
        return false;                                                                                    \
    }

        CHECK(Constant, GetRawValue)

        CHECK(Cast, GetOperandsType)
        CHECK(Cmp, GetOperandsType)

        CHECK(Compare, GetCc)
        CHECK(Compare, GetOperandsType)

        CHECK(If, GetCc)
        CHECK(If, GetOperandsType)

        CHECK(IfImm, GetCc)
        CHECK(IfImm, GetImm)
        CHECK(IfImm, GetOperandsType)

        CHECK(LoadArrayI, GetImm)
        CHECK(LoadArrayPairI, GetImm)
        CHECK(LoadPairPart, GetImm)
        CHECK(StoreArrayI, GetImm)
        CHECK(StoreArrayPairI, GetImm)
        CHECK(BoundsCheckI, GetImm)
        CHECK(ReturnI, GetImm)
        CHECK(AddI, GetImm)
        CHECK(SubI, GetImm)
        CHECK(ShlI, GetImm)
        CHECK(ShrI, GetImm)
        CHECK(AShrI, GetImm)
        CHECK(AndI, GetImm)
        CHECK(OrI, GetImm)
        CHECK(XorI, GetImm)

        CHECK(LoadStatic, GetVolatile)
        CHECK(StoreStatic, GetVolatile)
        CHECK(LoadObject, GetVolatile)
        CHECK(StoreObject, GetVolatile)
#undef CHECK
#undef CAST

        if (inst1->GetOpcode() == Opcode::Cmp && IsFloatType(inst1->GetInput(0).GetInst()->GetType())) {
            auto cmp1 = static_cast<compiler::CmpInst *>(inst1);
            auto cmp2 = static_cast<compiler::CmpInst *>(inst2);
            if (cmp1->IsFcmpg() != cmp2->IsFcmpg()) {
                inst_compare_map_.erase(inst1);
                return false;
            }
        }
        for (uint32_t i = 0; i < inst2->GetInputsCount(); i++) {
            if (inst1->GetInputType(i) != inst2->GetInputType(i)) {
                inst_compare_map_.erase(inst1);
                return false;
            }
        }
        return true;
    }

private:
    std::unordered_map<Inst *, Inst *> inst_compare_map_;
};

class IrBuilderTest : public AsmTest {
public:
    void CheckSimple(std::string inst_name, compiler::DataType::Type data_type, std::string inst_type)
    {
        ASSERT(inst_name == "mov" || inst_name == "lda" || inst_name == "sta");
        std::string curr_type;
        if (data_type == compiler::DataType::Type::REFERENCE) {
            curr_type = "i64[]";
        } else {
            curr_type = ToString(data_type);
        }

        std::string source = ".function " + curr_type + " main(";
        source += curr_type + " a0){\n";
        if (inst_name == "mov") {
            source += "mov" + inst_type + " v0, a0\n";
            source += "lda" + inst_type + " v0\n";
        } else if (inst_name == "lda") {
            source += "lda" + inst_type + " a0\n";
        } else if (inst_name == "sta") {
            source += "lda" + inst_type + " a0\n";
            source += "sta" + inst_type + " v0\n";
            source += "lda" + inst_type + " v0\n";
        } else {
            UNREACHABLE();
        }
        source += "return" + inst_type + "\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(data_type);

            BASIC_BLOCK(2, -1)
            {
                INST(1, Opcode::Return).Inputs(0);
                INS(1).SetType(data_type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckSimpleWithImm(std::string inst_name, compiler::DataType::Type data_type, std::string inst_type)
    {
        ASSERT(inst_name == "mov" || inst_name == "fmov" || inst_name == "lda" || inst_name == "flda");
        std::string curr_type = ToString(data_type);

        std::string source = ".function " + curr_type + " main(){\n";
        if (inst_name == "mov") {
            source += "movi" + inst_type + " v0, 0\n";
            source += "lda" + inst_type + " v0\n";
        } else if (inst_name == "fmov") {
            source += "fmovi" + inst_type + " v0, 0.\n";
            source += "lda" + inst_type + " v0\n";
        } else if (inst_name == "lda") {
            source += "ldai" + inst_type + " 0\n";
        } else if (inst_name == "flda") {
            source += "fldai" + inst_type + " 0.\n";
        } else {
            UNREACHABLE();
        }
        source += "return" + inst_type + "\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();

        GRAPH(graph)
        {
            CONSTANT(0, 0);
            INS(0).SetType(data_type);

            BASIC_BLOCK(2, -1)
            {
                INST(1, Opcode::Return).Inputs(0);
                INS(1).SetType(data_type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckCmp(std::string inst_name, compiler::DataType::Type data_type, std::string inst_type)
    {
        ASSERT(inst_name == "ucmp" || inst_name == "fcmpl" || inst_name == "fcmpg");
        std::string curr_type;
        if (data_type == compiler::DataType::Type::REFERENCE) {
            curr_type = "i64[]";
        } else {
            curr_type = ToString(data_type);
        }
        std::string source = ".function i32 main(";
        source += curr_type + " a0, ";
        source += curr_type + " a1){\n";
        source += "lda" + inst_type + " a0\n";
        source += inst_name + inst_type + " a1\n";
        source += "return\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(data_type);
            PARAMETER(1, 1);
            INS(1).SetType(data_type);

            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Cmp).s32().Inputs(0, 1);
                INST(3, Opcode::Return).s32().Inputs(2);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckFloatCmp(std::string inst_name, compiler::DataType::Type data_type, std::string inst_type, bool fcmpg)
    {
        ASSERT(inst_name == "fcmpl" || inst_name == "fcmpg");
        std::string curr_type = ToString(data_type);

        std::string source = ".function i32 main(";
        source += curr_type + " a0, ";
        source += curr_type + " a1){\n";
        source += "lda" + inst_type + " a0\n";
        source += inst_name + inst_type + " a1\n";
        source += "return\n";
        source += "}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(data_type);
            PARAMETER(1, 1);
            INS(1).SetType(data_type);

            BASIC_BLOCK(2, -1)
            {
                INST(2, Opcode::Cmp).s32().SrcType(data_type).Fcmpg(fcmpg).Inputs(0, 1);
                INST(3, Opcode::Return).s32().Inputs(2);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    template <bool is_obj>
    void CheckCondJumpWithZero(compiler::ConditionCode CC)
    {
        std::string cmd;
        switch (CC) {
            case compiler::ConditionCode::CC_EQ:
                cmd = "jeqz";
                break;
            case compiler::ConditionCode::CC_NE:
                cmd = "jnez";
                break;
            case compiler::ConditionCode::CC_LT:
                cmd = "jltz";
                break;
            case compiler::ConditionCode::CC_GT:
                cmd = "jgtz";
                break;
            case compiler::ConditionCode::CC_LE:
                cmd = "jlez";
                break;
            case compiler::ConditionCode::CC_GE:
                cmd = "jgez";
                break;
            default:
                UNREACHABLE();
        }

        std::string inst_postfix = "";
        std::string param_type = "i32";
        auto type = compiler::DataType::INT32;
        if constexpr (is_obj) {
            inst_postfix = ".obj";
            param_type = "i64[]";
            type = compiler::DataType::REFERENCE;
        }

        std::string source = ".function void main(";
        source += param_type + " a0) {\n";
        source += "lda" + inst_postfix + " a0\n";
        source += cmd + inst_postfix + " label\n";
        source += "label: ";
        source += "return.void\n}";

        ASSERT_TRUE(ParseToGraph(source, "main"));

        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0, 0);
            INS(0).SetType(type);
            CONSTANT(2, 0).s64();

            BASIC_BLOCK(2, 3, 4)
            {
                INST(1, Opcode::Compare).b().CC(CC).Inputs(0, 2);
                INST(3, Opcode::IfImm)
                    .SrcType(compiler::DataType::BOOL)
                    .CC(compiler::ConditionCode::CC_NE)
                    .Inputs(1)
                    .Imm(0);
            }
            BASIC_BLOCK(3, 4) {}
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    template <bool is_obj>
    void CheckCondJump(compiler::ConditionCode CC)
    {
        std::string cmd;
        switch (CC) {
            case compiler::ConditionCode::CC_EQ:
                cmd = "jeq";
                break;
            case compiler::ConditionCode::CC_NE:
                cmd = "jne";
                break;
            case compiler::ConditionCode::CC_LT:
                cmd = "jlt";
                break;
            case compiler::ConditionCode::CC_GT:
                cmd = "jgt";
                break;
            case compiler::ConditionCode::CC_LE:
                cmd = "jle";
                break;
            case compiler::ConditionCode::CC_GE:
                cmd = "jge";
                break;
            default:
                UNREACHABLE();
        }
        std::string inst_postfix = "";
        std::string param_type = "i32";
        auto type = compiler::DataType::INT32;
        if constexpr (is_obj) {
            inst_postfix = ".obj";
            param_type = "i64[]";
            type = compiler::DataType::REFERENCE;
        }

        std::string source = ".function void main(";
        source += param_type + " a0, " + param_type + " a1) {\n";
        source += "lda" + inst_postfix + " a0\n";
        source += cmd + inst_postfix + " a1, label\n";
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

            BASIC_BLOCK(2, 3, 4)
            {
                INST(2, Opcode::Compare).b().CC(CC).Inputs(0, 1);
                INST(3, Opcode::IfImm)
                    .SrcType(compiler::DataType::BOOL)
                    .CC(compiler::ConditionCode::CC_NE)
                    .Imm(0)
                    .Inputs(2);
            }
            BASIC_BLOCK(3, 4) {}
            BASIC_BLOCK(4, -1)
            {
                INST(4, Opcode::ReturnVoid).v0id();
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
    }

    void CheckOtherPasses(panda::pandasm::Program *prog, std::string fun_name)
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
        ASSERT_TRUE(prog->function_table.find(fun_name) != prog->function_table.end());
        auto &function = prog->function_table.at(fun_name);
        GetGraph()->RunPass<compiler::Cleanup>();
        EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, GetIrInterface()));
        auto pf = pandasm::AsmEmitter::Emit(*prog);
        ASSERT_NE(pf, nullptr);
    }

    void CheckConstArrayFilling(panda::pandasm::Program *prog, [[maybe_unused]] std::string class_name,
                                std::string func_name)
    {
        if (prog->literalarray_table.size() == 1) {
            EXPECT_TRUE(prog->literalarray_table["0"].literals_[0].tag_ == panda_file::LiteralTag::TAGVALUE);
            EXPECT_TRUE(prog->literalarray_table["0"].literals_[1].tag_ == panda_file::LiteralTag::INTEGER);
            EXPECT_TRUE(prog->literalarray_table["0"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_I32);
            return;
        }
        EXPECT_TRUE(prog->literalarray_table.size() == 8);
        for (const auto &elem : prog->literalarray_table) {
            EXPECT_TRUE(elem.second.literals_.size() == 5);
            EXPECT_TRUE(elem.second.literals_[0].tag_ == panda_file::LiteralTag::TAGVALUE);
            EXPECT_TRUE(elem.second.literals_[1].tag_ == panda_file::LiteralTag::INTEGER);
        }
        EXPECT_TRUE(prog->literalarray_table["7"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_U1);
        EXPECT_TRUE(prog->literalarray_table["6"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_I8);
        EXPECT_TRUE(prog->literalarray_table["5"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_I16);
        EXPECT_TRUE(prog->literalarray_table["4"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_I32);
        EXPECT_TRUE(prog->literalarray_table["3"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_I64);
        EXPECT_TRUE(prog->literalarray_table["2"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_F32);
        EXPECT_TRUE(prog->literalarray_table["1"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_F64);
        EXPECT_TRUE(prog->literalarray_table["0"].literals_[2].tag_ == panda_file::LiteralTag::ARRAY_STRING);

        EXPECT_TRUE(GetGraph()->RunPass<RegEncoder>());
        ASSERT_TRUE(prog->function_table.find(func_name) != prog->function_table.end());
        auto &function = prog->function_table.at(func_name);
        EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, GetIrInterface()));
        ASSERT(pandasm::AsmEmitter::Emit(class_name + ".panda", *prog, nullptr, nullptr, false));
    }

    enum CheckConstArrayTypes { ACCESS, SKIP_MULTIDIM_ARRAYS };

    void CheckConstArray(panda::pandasm::Program *prog, const char *class_name, std::string func_name,
                         CheckConstArrayTypes type)
    {
        options.SetConstArrayResolver(true);

        panda::pandasm::AsmEmitter::Emit(std::string(class_name) + ".panda", *prog, nullptr, nullptr, false);
        auto temp_name = func_name.substr(func_name.find(".") + 1);
        EXPECT_TRUE(ParseToGraph(prog, temp_name.substr(0, temp_name.find(":"))));
        EXPECT_TRUE(RunOptimizations(GetGraph(), GetIrInterface()));

        compiler::Inst *const_array_def_inst {nullptr};
        for (auto bb : GetGraph()->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                switch (type) {
                    case CheckConstArrayTypes::ACCESS: {
                        if (inst->GetOpcode() == Opcode::LoadConstArray) {
                            const_array_def_inst = inst;
                            continue;
                        }
                        if (inst->GetOpcode() == Opcode::LoadArray) {
                            EXPECT_TRUE(const_array_def_inst != nullptr);
                            EXPECT_TRUE(inst->CastToLoadArray()->GetArray() == const_array_def_inst);
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
        }

        EXPECT_TRUE(GetGraph()->RunPass<RegEncoder>());
        ASSERT_TRUE(prog->function_table.find(func_name) != prog->function_table.end());
        auto &function = prog->function_table.at(func_name);
        EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, GetIrInterface()));
        ASSERT(pandasm::AsmEmitter::Emit("LiteralArrayIntAccess.panda", *prog, nullptr, nullptr, false));
    }
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_TESTS_COMMON_H
