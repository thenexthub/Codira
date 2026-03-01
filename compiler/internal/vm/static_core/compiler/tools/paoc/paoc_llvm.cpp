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

#include "paoc_llvm.h"

#include "aot/aot_builder/llvm_aot_builder.h"

#include "optimizer/analysis/monitor_analysis.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_PAOC(level) LOG(level, COMPILER) << "PAOC: "

namespace ark::paoc {

using namespace ark::compiler;  // NOLINT(*-build-using-namespace)

void PaocLLVM::Clear(ark::mem::InternalAllocatorPtr allocator)
{
    llvmAotCompiler_ = nullptr;
    Paoc::Clear(allocator);
}

Paoc::LLVMCompilerStatus PaocLLVM::TryLLVM(CompilingContext *ctx)
{
    auto result = llvmAotCompiler_->TryAddGraph(ctx->graph);
    if (result.HasValue()) {
        if (result.Value()) {
            GetAotBuilder()->AddMethodHeader(ctx->method, ctx->index);
            return LLVMCompilerStatus::COMPILED;
        }  // In result.Value() == false case (e.g. unbalanced monitors), fallthrough to possible fallback below
    } else if (!llvmbackend::g_options.IsLlvmAllowBreakage() || !ShouldIgnoreFailures()) {
        LOG_PAOC(ERROR) << result.Error();
        return LLVMCompilerStatus::ERROR;
    }

    // Check if fallback is actually allowed
    if (llvmbackend::g_options.IsLlvmFallback()) {
        return LLVMCompilerStatus::FALLBACK;
    }
    return LLVMCompilerStatus::SKIP;
}

bool PaocLLVM::EndLLVM()
{
    ASSERT(IsLLVMAotMode());
    llvmAotCompiler_->FinishCompile();
    if (!ShouldIgnoreFailures() && llvmAotCompiler_->IsIrFailed()) {
        return false;
    }
    if (!llvmbackend::g_options.IsLlvmFallback() && !llvmAotCompiler_->HasCompiledCode()) {
        return false;
    }
    return true;
}

void PaocLLVM::PrepareLLVM(const ark::Span<const char *> &args)
{
    ASSERT(IsLLVMAotMode());
    std::string cmdline;
    for (auto arg : args) {
        cmdline += arg;
        cmdline += " ";
    }

    auto outputFile = GetPaocOptions()->GetPaocOutput();
    if (GetPaocOptions()->WasSetPaocBootOutput()) {
        outputFile = GetPaocOptions()->GetPaocBootOutput();
    }
    llvmAotCompiler_ =
        llvmbackend::CreateLLVMAotCompiler(GetRuntime(), GetCodeAllocator(), GetAotBuilder(), cmdline, outputFile);
}

void PaocLLVM::ValidateExtraOptions()
{
    auto llvmOptionsErr = ark::llvmbackend::g_options.Validate();
#ifdef NDEBUG
    if (!llvmbackend::g_options.GetLlvmBreakIrRegex().empty()) {
        LOG_PAOC(FATAL) << "--llvm-break-ir-regex is available only in debug builds";
    }
#endif
    if (llvmOptionsErr) {
        LOG_PAOC(FATAL) << llvmOptionsErr.value().GetMessage();
    }
}

std::unique_ptr<compiler::AotBuilder> PaocLLVM::CreateAotBuilder()
{
    return std::make_unique<LLVMAotBuilder>();
}

compiler::LLVMAotBuilder *PaocLLVM::GetAotBuilder()
{
    return static_cast<LLVMAotBuilder *>(aotBuilder_.get());
}
}  // namespace ark::paoc

#undef LOG_PAOC
