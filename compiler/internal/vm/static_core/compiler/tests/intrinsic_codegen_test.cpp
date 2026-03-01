/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "unit_test.h"
#include "inst_generator.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "target/amd64/target.h"
#include "target/aarch64/target.h"
#include "target/aarch32/target.h"

namespace ark::compiler {

class IntrinsicCodegenTest : public GraphTest {
public:
    IntrinsicCodegenTest()
        : alloc_ {SpaceType::SPACE_TYPE_COMPILER},
          local_alloc_ {SpaceType::SPACE_TYPE_COMPILER},
          graph_creator_ {alloc_, local_alloc_},
          aarch64_encoder_ {&alloc_},
          aarch32_encoder_ {&alloc_},
          amd64_encoder_ {&alloc_}
    {
    }

    bool TryEncode(DataType::Type type, RuntimeInterface::IntrinsicId intrinsic_id, Arch arch, bool prepare_graph)
    {
        graph_creator_.SetRuntimeTargetArch(arch);
        auto inst = GenerateIntrinsic(&alloc_, type, intrinsic_id);
        auto graph = graph_creator_.GenerateGraph(inst);
        Codegen codegen(graph);
        codegen.GetCodeBuilder()->BeginMethod(0U, 0U);
        if (prepare_graph) {
            graph->GetAnalysis<LoopAnalyzer>().Run();
            GraphChecker(graph).Check();
            RegAlloc(graph);
            codegen.Initialize();
        }
        codegen.CreateBuiltinIntrinsic(inst);
        return codegen.GetEncoder()->GetResult();
    }

private:
    ArenaAllocator alloc_;
    ArenaAllocator local_alloc_;
    GraphCreator graph_creator_;
    aarch64::Aarch64Encoder aarch64_encoder_;
    aarch32::Aarch32Encoder aarch32_encoder_;
    amd64::Amd64Encoder amd64_encoder_;

    IntrinsicInst *GenerateIntrinsic(ArenaAllocator *allocator, DataType::Type type,
                                     RuntimeInterface::IntrinsicId intrinsic_id)
    {
        auto inst = Inst::New<IntrinsicInst>(allocator, Opcode::Intrinsic, intrinsic_id);
        inst->SetType(type);
        return inst;
    }
};

#ifdef INTRINSIC_CODEGEN_TEST_ARM64
TEST_F(IntrinsicCodegenTest, EncodingARM64)
{
    std::pair<Arch, std::string> arch = std::pair {Arch::AARCH64, "arm64"};
#include "intrinsic_codegen_test.inl"
}
#endif

#ifdef INTRINSIC_CODEGEN_TEST_AMD64
TEST_F(IntrinsicCodegenTest, EncodingAMD64)
{
    std::pair<Arch, std::string> arch = std::pair {Arch::X86_64, "amd64"};
#include "intrinsic_codegen_test.inl"
}
#endif

#ifdef INTRINSIC_CODEGEN_TEST_ARM32
TEST_F(IntrinsicCodegenTest, EncodingARM32)
{
    std::pair<Arch, std::string> arch = std::pair {Arch::AARCH32, "arm32"};
#include "intrinsic_codegen_test.inl"
}
#endif
}  // namespace ark::compiler
