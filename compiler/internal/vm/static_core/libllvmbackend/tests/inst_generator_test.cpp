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

#include "tests/unit_test.h"
#include "tests/inst_generator.h"
#include "llvm_aot_compiler.h"
#include "llvm_options.h"
#include "libarkfile/file_item_container.h"

#include "aot/aot_builder/llvm_aot_builder.h"

using ark::compiler::Opcode;

namespace ark::llvmbackend {
class LLVMCodegenTest : public ark::compiler::GraphTest {};

class LLVMCodegenStatisticGenerator : public ark::compiler::StatisticGenerator {
public:
    using StatisticGenerator::StatisticGenerator;

    void GenerateOp(Opcode op)
    {
        ASSERT(op != Opcode::Builtin);
        if (op == Opcode::Intrinsic || op == Opcode::SpillFill) {
            return;
        }
        auto it = instGenerator_.Generate(op);
        ASSERT(it.begin() != it.end());
        if ((*(it.begin()))->IsLowLevel()) {
            return;
        }
        FullInstStat fullInstStat = tmplt_;
        for (auto &i : it) {
            auto graph = graphCreator_.GenerateGraph(i);
            ark::compiler::LLVMAotBuilder aotBuilder;
            graph->SetRuntime(graphCreator_.GetRuntime());
            aotBuilder.SetArch(graph->GetArch());
            aotBuilder.SetRuntime(graph->GetRuntime());

            LLVMAotCompiler llvm(graph->GetRuntime(), graphCreator_.GetAllocator(), &aotBuilder, "", "inst-gen.abc");

            if (graph->GetAotData() == nullptr) {
                uintptr_t codeAddress = aotBuilder.GetCurrentCodeAddress();
                auto aotData = graph->GetAllocator()->New<compiler::AotData>(
                    compiler::AotData::AotDataArgs {nullptr,
                                                    graph,
                                                    nullptr,
                                                    codeAddress,
                                                    aotBuilder.GetIntfInlineCacheIndex(),
                                                    {aotBuilder.GetGotPlt(), aotBuilder.GetGotVirtIndexes(),
                                                     aotBuilder.GetGotClass(), aotBuilder.GetGotString()},
                                                    {aotBuilder.GetGotIntfInlineCache(), aotBuilder.GetGotCommon()}});
                aotData->SetUseCha(true);
                graph->SetAotData(aotData);
            }
            auto res = llvm.TryAddGraph(graph);
            ASSERT(res.HasValue() && !llvm.IsIrFailed());
            bool status = res.Value();

            // NOLINTNEXTLINE(hicpp-signed-bitwise)
            fullInstStat[(i)->GetType()] &= static_cast<int>(status);
            allInstNumber_++;
            positiveInstNumber_ += static_cast<int>(status);
            // To consume less memory
            graph->~Graph();
            graphCreator_.GetAllocator()->Resize(0);
        }
        statistic_.first.insert({op, fullInstStat});
    }

    void GenerateIntrinsic(compiler::Inst *i)
    {
        ASSERT(graphCreator_.GetAllocator()->GetAllocatedSize() == 0);

        auto intrinsicsId = (i)->CastToIntrinsic()->GetIntrinsicId();
        auto graph = graphCreator_.GenerateGraph(i);
        ark::compiler::LLVMAotBuilder aotBuilder;
        aotBuilder.SetArch(graph->GetArch());
        graph->SetRuntime(graphCreator_.GetRuntime());
        aotBuilder.SetRuntime(graph->GetRuntime());
        ASSERT(compiler::g_options.IsCompilerEncodeIntrinsics());
        bool encodes = EncodesBuiltin(graph->GetRuntime(), intrinsicsId, graph->GetArch());
        if (!encodes) {
            compiler::g_options.SetCompilerEncodeIntrinsics(false);
        }

        LLVMAotCompiler llvm(graph->GetRuntime(), graphCreator_.GetAllocator(), &aotBuilder, "", "inst-gen.abc");

        if (graph->GetAotData() == nullptr) {
            uintptr_t codeAddress = aotBuilder.GetCurrentCodeAddress();
            auto aotData = graph->GetAllocator()->New<compiler::AotData>(
                compiler::AotData::AotDataArgs {nullptr,
                                                graph,
                                                nullptr,
                                                codeAddress,
                                                aotBuilder.GetIntfInlineCacheIndex(),
                                                {aotBuilder.GetGotPlt(), aotBuilder.GetGotVirtIndexes(),
                                                 aotBuilder.GetGotClass(), aotBuilder.GetGotString()},
                                                {aotBuilder.GetGotIntfInlineCache(), aotBuilder.GetGotCommon()}});
            aotData->SetUseCha(true);
            graph->SetAotData(aotData);
        }
        auto res = llvm.TryAddGraph(graph);
        ASSERT(!llvm.IsIrFailed());
        bool status = res.HasValue() && res.Value();

        if (!encodes) {
            compiler::g_options.SetCompilerEncodeIntrinsics(true);
        }

        statistic_.second[intrinsicsId] = status;
        allInstNumber_++;
        positiveInstNumber_ += static_cast<int>(status);
        graph->~Graph();
        graphCreator_.GetAllocator()->Resize(0);
    }

    void Generate() override
    {
        compiler::g_options.SetCompilerNonOptimizing(true);
        for (const auto &op : instGenerator_.GetMap()) {
            GenerateOp(op.first);
        }
        auto intrinsics = instGenerator_.Generate(Opcode::Intrinsic);
        for (auto &i : intrinsics) {
            GenerateIntrinsic(i);
        }
        for (auto i = 0; i != static_cast<int>(Opcode::NUM_OPCODES); ++i) {
            auto opc = static_cast<Opcode>(i);
            if (opc == Opcode::NOP || opc == Opcode::Intrinsic || opc == Opcode::Builtin) {
                continue;
            }
            allOpcodeNumber_++;
            implementedOpcodeNumber_ += static_cast<int>(statistic_.first.find(opc) != statistic_.first.end());
        }
    }
};

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
TEST_F(LLVMCodegenTest, AllInstTest)
{
    ArenaAllocator instAlloc {SpaceType::SPACE_TYPE_COMPILER};
    ark::compiler::InstGenerator instGen(instAlloc);

    ArenaAllocator localAlloc {SpaceType::SPACE_TYPE_COMPILER};
    ArenaAllocator graphAlloc {SpaceType::SPACE_TYPE_COMPILER};
    ark::compiler::GraphCreator graphCreator {graphAlloc, localAlloc};

    // ARM64
    LLVMCodegenStatisticGenerator statGenArm64(instGen, graphCreator);
    statGenArm64.Generate();
    statGenArm64.GenerateHTMLPage("LLVMCodegenStatisticARM64.html");

    // AMD64
    graphCreator.SetRuntimeTargetArch(Arch::X86_64);
    LLVMCodegenStatisticGenerator statGenAmd64(instGen, graphCreator);
    statGenAmd64.Generate();
    statGenAmd64.GenerateHTMLPage("LLVMCodegenStatisticAMD64.html");
}

}  // namespace ark::llvmbackend
