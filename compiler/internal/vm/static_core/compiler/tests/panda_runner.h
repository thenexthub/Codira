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

#ifndef PANDA_PANDA_RUNNER_H
#define PANDA_PANDA_RUNNER_H

#include "libarkbase/macros.h"
#include "unit_test.h"
#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "include/runtime.h"

namespace ark::test {

extern "C" int PandaRunnerHookAArch64();
extern "C" int PandaRunnerHook(uintptr_t lr, uintptr_t fp);
extern "C" int StackWalkerHookBridge();

class PandaRunner {
public:
    using Callback = int (*)(uintptr_t, uintptr_t);

    PandaRunner()
    {
        auto execPath = ark::os::file::File::GetExecutablePath();

        std::vector<std::string> bootPandaFiles = {execPath.Value() + "/../pandastdlib/arkstdlib.abc"};

        options_.SetBootPandaFiles(bootPandaFiles);

        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetNoAsyncJit(true);
    }

    void Parse(std::string_view source)
    {
        pandasm::Parser parser;

        auto res = parser.Parse(source.data());
        ASSERT_TRUE(res) << "Parse failed: " << res.Error().message << "\nLine " << res.Error().lineNumber << ": "
                         << res.Error().wholeLine;
        file_ = pandasm::AsmEmitter::Emit(res.Value());
    }

    ~PandaRunner() = default;

    NO_MOVE_SEMANTIC(PandaRunner);
    NO_COPY_SEMANTIC(PandaRunner);

    void SetHook(Callback hook)
    {
        callback_ = hook;
    }

    void Run(std::string_view source)
    {
        return Run(source, std::vector<std::string> {});
    }

    void Run(std::string_view source, Callback hook)
    {
        callback_ = hook;
        return Run(source, ssize_t(0));
    }

    void Run(std::string_view source, ssize_t expectedResult)
    {
        expectedResult_ = expectedResult;
        return Run(source, std::vector<std::string> {});
    }

    void Run(std::string_view source, const std::vector<std::string> &args)
    {
        auto finalizer = [](void *) {
            callback_ = nullptr;
            Runtime::Destroy();
        };
        std::unique_ptr<void, decltype(finalizer)> runtimeDestroyer(&finalizer, finalizer);

        compiler::CompilerLogger::SetComponents(GetCompilerOptions().GetCompilerLog());

        Run(CreateRuntime(), source, args);
    }

    void Run(Runtime *runtime, std::string_view source, const std::vector<std::string> &args)
    {
        pandasm::Parser parser;
        auto res = parser.Parse(source.data());
        ASSERT_TRUE(res) << "Parse failed: " << res.Error().message << "\nLine " << res.Error().lineNumber << ": "
                         << res.Error().wholeLine;
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        runtime->GetClassLinker()->AddPandaFile(std::move(pf));

        if (callback_ != nullptr) {
            if (auto method = GetMethod("hook"); method != nullptr) {
                method->SetCompiledEntryPoint(reinterpret_cast<void *>(StackWalkerHookBridge));
            }
        }

        auto eres = runtime->Execute("_GLOBAL::main", args);
        ASSERT_TRUE(eres) << static_cast<unsigned>(eres.Error());
        if (expectedResult_) {
            ASSERT_EQ(eres.Value(), expectedResult_.value());
        }
    }

    Runtime *CreateRuntime()
    {
        Runtime::Create(options_);
        return Runtime::GetCurrent();
    }

    RuntimeOptions &GetRuntimeOptions()
    {
        return options_;
    }

    compiler::CompilerOptions &GetCompilerOptions()
    {
        return compiler::g_options;
    }

    static Method *GetMethod(std::string_view methodName)
    {
        PandaString descriptor;
        auto *thread = MTManagedThread::GetCurrent();
        thread->ManagedCodeBegin();
        auto cls = Runtime::GetCurrent()
                       ->GetClassLinker()
                       ->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
        thread->ManagedCodeEnd();
        ASSERT(cls);
        return cls->GetDirectMethod(utf::CStringAsMutf8(methodName.data()));
    }

private:
    friend int PandaRunnerHookAArch64();
    friend int PandaRunnerHook(uintptr_t lr, uintptr_t fp);

private:
    RuntimeOptions options_;
    static inline Callback callback_ {nullptr};
    std::unique_ptr<const panda_file::File> file_ {nullptr};
    std::optional<ssize_t> expectedResult_;
};
}  // namespace ark::test

#endif  // PANDA_PANDA_RUNNER_H
