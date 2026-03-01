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

#include <gtest/gtest.h>

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "bytecode_optimizer/optimize_bytecode.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/runtime.h"
#include "mangling.h"

namespace ark::bytecodeopt::test {

class BytecodeOptPeepholes : public testing::Test {
public:
    BytecodeOptPeepholes()
    {
        RuntimeOptions options;
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetHeapSizeLimit(128_MB);
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Logger::InitializeDummyLogging();

        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~BytecodeOptPeepholes() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(BytecodeOptPeepholes);
    NO_MOVE_SEMANTIC(BytecodeOptPeepholes);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_;
};

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(BytecodeOptPeepholes, TryBlock)
{
    pandasm::Parser p;

    auto source = R"(
    .record E {}
    .record R {
        u1 field
    }

    .function void R.ctor(R a0) <ctor> {
        newobj v0, E
        throw v0
    }

    .function u64 try_catch() {
    try_begin:
        movi.64 v1, 0x1
        newobj v0, R
        movi.64 v1, 0x2
        call.short R.ctor, v0
    try_end:
        ldai.64 0x0
        return
    catch_all:
        lda.64 v1
        return
    .catchall try_begin, try_end, catch_all
    }
    )";

    auto res = p.Parse(source);
    auto &program = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    std::string fileName = "bc_peepholes";
    auto piece = pandasm::AsmEmitter::Emit(fileName, program, nullptr, &maps);
    ASSERT_NE(piece, false);

    EXPECT_TRUE(OptimizeBytecode(&program, &maps, fileName, false, true));

    // Check if there is initobj instruction in the bytecode
    bool containsInitobj = false;
    const auto sigTryCatch = pandasm::GetFunctionSignatureFromName("try_catch", {});
    for (const auto &inst : program.functionStaticTable.at(sigTryCatch).ins) {
        if (inst.opcode == pandasm::Opcode::INITOBJ) {
            containsInitobj = true;
        }
    }
    EXPECT_FALSE(containsInitobj);

    auto pf = pandasm::AsmEmitter::Emit(program);
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);
    PandaString descriptor;

    auto *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    ASSERT_NE(klass, nullptr);

    Method *method = klass->GetDirectMethod(utf::CStringAsMutf8("try_catch"));
    ASSERT_NE(method, nullptr);

    std::vector<Value> args;
    args.emplace_back(Value(1U));
    Value v = method->Invoke(ManagedThread::GetCurrent(), args.data());
    EXPECT_EQ(v.GetAsLong(), 0x2U);
}

}  // namespace ark::bytecodeopt::test
