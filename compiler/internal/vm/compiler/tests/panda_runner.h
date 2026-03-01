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

#ifndef COMPILER_TESTS_PANDA_RUNNER_H
#define COMPILER_TESTS_PANDA_RUNNER_H

#include "unit_test.h"
#include "assembler/assembly-emitter.h"
#include "assembler/assembly-parser.h"
#include "include/runtime.h"

namespace panda::test {
class PandaRunner {
public:
    using Callback = int (*)(uintptr_t, uintptr_t);

    PandaRunner()
    {
        auto exec_path = panda::os::file::File::GetExecutablePath();

        std::vector<std::string> boot_panda_files = {exec_path.Value() + "/../pandastdlib/arkstdlib.abc"};

        options_.SetBootPandaFiles(boot_panda_files);

        options_.SetShouldLoadBootPandaFiles(true);
        options_.SetShouldInitializeIntrinsics(false);
        options_.SetNoAsyncJit(true);
    }

    void Parse(std::string_view source)
    {
        pandasm::Parser parser;

        auto res = parser.Parse(source.data());
        ASSERT_TRUE(res) << "Parse failed: " << res.Error().message << "\nLine " << res.Error().line_number << ": "
                         << res.Error().whole_line;
        file_ = pandasm::AsmEmitter::Emit(res.Value());
    }

    ~PandaRunner() = default;

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

    void Run(std::string_view source, ssize_t expected_result)
    {
        expected_result_ = expected_result;
        return Run(source, std::vector<std::string> {});
    }

    void Run(std::string_view source, const std::vector<std::string> &args)
    {
        auto finalizer = [](void *) {
            callback_ = nullptr;
            Runtime::Destroy();
        };
        std::unique_ptr<void, decltype(finalizer)> runtime_destroyer(&finalizer, finalizer);

        compiler::CompilerLogger::SetComponents(GetCompilerOptions().GetCompilerLog());

        Run(CreateRuntime(), source, args);
    }

    void Run(Runtime *runtime, std::string_view source, const std::vector<std::string> &args)
    {
        pandasm::Parser parser;
        auto res = parser.Parse(source.data());
        ASSERT_TRUE(res) << "Parse failed: " << res.Error().message << "\nLine " << res.Error().line_number << ": "
                         << res.Error().whole_line;
        auto pf = pandasm::AsmEmitter::Emit(res.Value());
        runtime->GetClassLinker()->AddPandaFile(std::move(pf));

        if (callback_) {
            if (auto method = GetMethod("hook"); method != nullptr) {
                method->SetCompiledEntryPoint(reinterpret_cast<void *>(Hook));
            }
        }

        auto eres = runtime->Execute("_GLOBAL::main", args);
        ASSERT_TRUE(eres) << static_cast<unsigned>(eres.Error());
        if (expected_result_) {
            ASSERT_EQ(eres.Value(), expected_result_.value());
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
        return compiler::options;
    }

    static Method *GetMethod(std::string_view method_name)
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
        return cls->GetDirectMethod(utf::CStringAsMutf8(method_name.data()));
    }

private:
    NO_OPTIMIZE static int Hook()
    {
        ASSERT(callback_);
        if constexpr (RUNTIME_ARCH == Arch::AARCH64) {
            uintptr_t fp;
            uintptr_t lr;
            ManagedThread::GetCurrent()->SetCurrentFrameIsCompiled(true);
            asm("ldr %0, [fp, #0]" : "=r"(fp));
            asm("ldr %0, [fp, #8]" : "=r"(lr));
            ManagedThread::GetCurrent()->SetCurrentFrame(reinterpret_cast<Frame *>(fp));
            return callback_(lr, fp);
        } else if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
            uintptr_t fp;
            uintptr_t lr;
            ManagedThread::GetCurrent()->SetCurrentFrameIsCompiled(true);
#if (defined(__clang__) || defined(PANDA_TARGET_ARM64))
            asm("ldr %0, [fp, #0]" : "=r"(fp));
            asm("ldr %0, [fp, #4]" : "=r"(lr));
#else
            // gcc compile header "push {r4, r11, lr}"
            asm("ldr %0, [fp, #-4]" : "=r"(fp));
            asm("ldr %0, [fp, #0]" : "=r"(lr));
#endif
            ManagedThread::GetCurrent()->SetCurrentFrame(reinterpret_cast<Frame *>(fp));
            return callback_(lr, fp);
        } else if constexpr (RUNTIME_ARCH == Arch::X86_64) {
            uintptr_t fp;
            uintptr_t lr;
            ManagedThread::GetCurrent()->SetCurrentFrameIsCompiled(true);
            asm("movq (%%rbp), %0" : "=r"(fp));
            asm("movq 8(%%rbp), %0" : "=r"(lr));
            ManagedThread::GetCurrent()->SetCurrentFrame(reinterpret_cast<Frame *>(fp));
            return callback_(lr, fp);
        } else {
            return -1;
        }
    }

private:
    RuntimeOptions options_;
    static inline Callback callback_ {nullptr};
    std::unique_ptr<const panda_file::File> file_ {nullptr};
    std::optional<ssize_t> expected_result_;
};
}  // namespace panda::test

#endif  // COMPILER_TESTS_PANDA_RUNNER_H
