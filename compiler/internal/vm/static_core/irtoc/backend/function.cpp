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

#include "function.h"
#include "compiler/codegen_boundary.h"
#include "compiler/codegen_fastpath.h"
#include "compiler/codegen_nativeplus.h"
#include "compiler/codegen_interpreter.h"
#include "compiler/optimizer_run.h"
#include "compiler/dangling_pointers_checker.h"
#include "compiler/optimizer/code_generator/target_info.h"
#include "compiler/optimizer/optimizations/balance_expressions.h"
#include "compiler/optimizer/optimizations/branch_elimination.h"
#include "compiler/optimizer/optimizations/checks_elimination.h"
#include "compiler/optimizer/optimizations/code_sink.h"
#include "compiler/optimizer/optimizations/cse.h"
#include "compiler/optimizer/optimizations/deoptimize_elimination.h"
#include "compiler/optimizer/optimizations/if_conversion.h"
#include "compiler/optimizer/optimizations/if_merging.h"
#include "compiler/optimizer/optimizations/licm.h"
#include "compiler/optimizer/optimizations/loop_peeling.h"
#include "compiler/optimizer/optimizations/loop_unroll.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/lse.h"
#include "compiler/optimizer/optimizations/memory_barriers.h"
#include "compiler/optimizer/optimizations/memory_coalescing.h"
#include "compiler/optimizer/optimizations/move_constants.h"
#include "compiler/optimizer/optimizations/peepholes.h"
#include "compiler/optimizer/optimizations/redundant_loop_elimination.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc.h"
#include "compiler/optimizer/optimizations/scheduler.h"
#include "compiler/optimizer/optimizations/try_catch_resolving.h"
#include "compiler/optimizer/optimizations/vn.h"
#include "elfio.hpp"
#include "irtoc_runtime.h"

namespace ark::irtoc {

using compiler::Graph;

static bool RunIrtocOptimizations(Graph *graph);
static bool RunIrtocInterpreterOptimizations(Graph *graph);

Function::Result Function::Compile(Arch arch, ArenaAllocator *allocator, ArenaAllocator *localAllocator)
{
    IrtocRuntimeInterface runtime;
    graph_ = allocator->New<Graph>(Graph::GraphArgs {allocator, localAllocator, arch, this, &runtime}, false);
    ASSERT(graph_ != nullptr);
    builder_ = std::make_unique<compiler::IrConstructor>();

    MakeGraphImpl();

    if (GetGraph()->GetMode().IsNative()) {
        compiler::InPlaceCompilerTaskRunner taskRunner;
        taskRunner.GetContext().SetGraph(GetGraph());
        bool success = true;
        taskRunner.AddCallbackOnFail(
            [&success]([[maybe_unused]] compiler::InPlaceCompilerContext &compilerCtx) { success = false; });
        compiler::RunOptimizations<compiler::INPLACE_MODE>(std::move(taskRunner));
        if (!success) {
            return Unexpected("RunOptimizations failed!");
        }
        compilationResult_ = CompilationResult::ARK;
    } else {
        auto result = RunOptimizations();
        if (!result) {
            return result;
        }
        compilationResult_ = result.Value();
    }
    builder_.reset(nullptr);

    auto code = GetGraph()->GetCode();
    SetCode(code);

    ASSERT(compilationResult_ != CompilationResult::INVALID);
    return compilationResult_;
}

Function::Result Function::RunOptimizations()
{
    auto result = CompilationResult::INVALID;
    bool interpreter = GetGraph()->GetMode().IsInterpreter() || GetGraph()->GetMode().IsInterpreterEntry();
    bool shouldTryLlvm = false;
#ifdef PANDA_LLVM_IRTOC
    bool withLlvmSuffix = std::string_view {GetName()}.find(LLVM_SUFFIX) != std::string_view::npos;
    if (withLlvmSuffix) {
#ifdef PANDA_LLVM_INTERPRETER
        shouldTryLlvm = true;
        // Functions with "_LLVM" suffix must be interpreter handlers from the irtoc_code_llvm.cpp
        LOG_IF(!interpreter, FATAL, IRTOC)
            << "'" << GetName() << "' must not contain '" << LLVM_SUFFIX << "', only interpreter handlers can";
#else
        LOG(FATAL, IRTOC) << "'" << GetName() << "' handler can not exist with PANDA_LLVM_INTERPRETER disabled";
#endif
    }
#ifdef PANDA_LLVM_FASTPATH
    shouldTryLlvm |= !interpreter;
#endif
#endif  // PANDA_LLVM_IRTOC
    if (shouldTryLlvm) {
        result = CompileByLLVM();
        ASSERT(result == CompilationResult::LLVM || result == CompilationResult::ARK_BECAUSE_SKIP ||
               result == CompilationResult::ARK_BECAUSE_FALLBACK);
    } else {
        result = CompilationResult::ARK;
    }
    if (result == CompilationResult::LLVM) {
        return result;
    }
    ASSERT(GetGraph()->GetCode().Empty());
    if (interpreter) {
        if (!RunIrtocInterpreterOptimizations(GetGraph())) {
            return Unexpected {"RunIrtocInterpreterOptimizations failed"};
        }
    } else {
        if (!RunIrtocOptimizations(GetGraph())) {
            return Unexpected {"RunIrtocOptimizations failed"};
        }
    }
    return result;
}

void Function::SetCode(Span<uint8_t> code)
{
    std::copy(code.begin(), code.end(), std::back_inserter(code_));
}

CompilationResult Function::CompileByLLVM()
{
#ifndef PANDA_LLVM_IRTOC
    UNREACHABLE();
#else
    if (SkippedByLLVM()) {
        return CompilationResult::ARK_BECAUSE_SKIP;
    }
    ASSERT(llvmCompiler_ != nullptr);
    auto result = llvmCompiler_->TryAddGraph(GetGraph());
    if (!result.HasValue()) {
        LOG(FATAL, IRTOC) << "LLVM IRTOC compilation failed for function '" << GetName() << "', error: '"
                          << result.Error() << "'";
    }
    return result.Value() ? CompilationResult::LLVM : CompilationResult::ARK_BECAUSE_FALLBACK;
#endif  // ifndef PANDA_LLVM_IRTOC
}

#ifdef PANDA_LLVM_IRTOC
void Function::ReportCompilationStatistic(std::ostream *out)
{
    ASSERT(out != nullptr);
    ASSERT(!g_options.Validate());
    const auto &statsLevel = g_options.GetIrtocLlvmStats();
    if (statsLevel == "none") {
        return;
    }
    if (statsLevel != "full" && GetGraph()->IsDynamicMethod()) {
        return;
    }
    if (statsLevel == "short") {
        if (compilationResult_ == CompilationResult::LLVM || compilationResult_ == CompilationResult::ARK) {
            return;
        }
    }

    static constexpr auto HANDLER_WIDTH = 96;
    static constexpr auto RESULT_WIDTH = 8;

    *out << "LLVM " << GraphModeToString() << std::setw(HANDLER_WIDTH) << GetName() << " --- "
         << std::setw(RESULT_WIDTH) << LLVMCompilationResultToString() << "(size " << GetCodeSize() << ")" << std::endl;
}

std::string_view Function::LLVMCompilationResultToString() const
{
    if (compilationResult_ == CompilationResult::ARK) {
        return "ark";
    }
    if (compilationResult_ == CompilationResult::LLVM) {
        return "ok";
    }
    if (compilationResult_ == CompilationResult::ARK_BECAUSE_FALLBACK) {
        return "fallback";
    }
    if (compilationResult_ == CompilationResult::ARK_BECAUSE_SKIP) {
        return "skip";
    }
    UNREACHABLE();
}

std::string_view Function::GraphModeToString()
{
    if (GetGraph()->GetMode().IsFastPath()) {
        return "fastpath";
    }
    if (GetGraph()->GetMode().IsBoundary()) {
        return "boundary";
    }
    if (GetGraph()->GetMode().IsInterpreter() || GetGraph()->GetMode().IsInterpreterEntry()) {
        return "interp";
    }
    if (GetGraph()->GetMode().IsNative()) {
        return "native";
    }
    if (GetGraph()->GetMode().IsNativePlus()) {
        return "native_plus";
    }
    UNREACHABLE();
}
#endif

void Function::AddRelocation(const compiler::RelocationInfo &info)
{
    relocationEntries_.emplace_back(info);
}

#ifdef PANDA_LLVM_IRTOC
bool Function::SkippedByLLVM()
{
    auto name = std::string(GetName());
#ifdef PANDA_LLVM_INTERPRETER
    if (GetGraph()->GetMode().IsInterpreterEntry() || GetGraph()->GetMode().IsInterpreter()) {
        ASSERT(name.find(LLVM_SUFFIX) == name.size() - LLVM_SUFFIX.size());
        name = name.substr(0, name.size() - LLVM_SUFFIX.size());
        return std::find(SKIPPED_HANDLERS.begin(), SKIPPED_HANDLERS.end(), name) != SKIPPED_HANDLERS.end();
    }
#endif

#ifdef PANDA_LLVM_FASTPATH
    if (GetGraph()->GetMode().IsFastPath()) {
        ASSERT(GetArch() == Arch::AARCH64);
        return std::find(SKIPPED_FASTPATHS.begin(), SKIPPED_FASTPATHS.end(), name) != SKIPPED_FASTPATHS.end() ||
               (compiler::g_options.IsCpuFeatureEnabled(compiler::CpuFeature::JSCVT) &&
                std::find(SKIPPED_FASTPATHS_JSCVT.begin(), SKIPPED_FASTPATHS_JSCVT.end(), name) !=
                    SKIPPED_FASTPATHS_JSCVT.end());
    }
#endif
    return true;
}
#endif

static bool RunIrtocInterpreterOptimizations(Graph *graph)
{
    compiler::g_options.SetCompilerChecksElimination(false);
    // aantipina: re-enable Lse
    compiler::g_options.SetCompilerLse(false);
#ifdef PANDA_COMPILER_TARGET_AARCH64
    compiler::g_options.SetCompilerMemoryCoalescing(false);
#endif
    if (!compiler::g_options.IsCompilerNonOptimizing()) {
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::BranchElimination>();
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::IfMerging>();
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Cse>();
        graph->RunPass<compiler::Licm>(compiler::g_options.GetCompilerLicmHoistLimit());
        graph->RunPass<compiler::RedundantLoopElimination>();
        graph->RunPass<compiler::LoopPeeling>();
        graph->RunPass<compiler::Lse>();
        graph->RunPass<compiler::ValNum>();
        if (graph->RunPass<compiler::Peepholes>() && graph->RunPass<compiler::BranchElimination>()) {
            graph->RunPass<compiler::Peepholes>();
        }
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Cse>();
        graph->RunPass<compiler::ChecksElimination>();
        graph->RunPass<compiler::LoopUnroll>(compiler::g_options.GetCompilerLoopUnrollInstLimit(),
                                             compiler::g_options.GetCompilerLoopUnrollFactor());
        graph->RunPass<compiler::BalanceExpressions>();
        if (graph->RunPass<compiler::Peepholes>()) {
            graph->RunPass<compiler::BranchElimination>();
        }
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::Cse>();

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif  // NDEBUG
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Lowering>();
        graph->RunPass<compiler::CodeSink>();
        graph->RunPass<compiler::MemoryCoalescing>(compiler::g_options.IsCompilerMemoryCoalescingAligned());
        graph->RunPass<compiler::IfConversion>(compiler::g_options.GetCompilerIfConversionLimit());
        graph->RunPass<compiler::MoveConstants>();
    }

    graph->RunPass<compiler::Cleanup>();
    if (!compiler::RegAlloc(graph) || !graph->RunPass<compiler::DanglingPointersChecker>()) {
        return false;
    }

    return graph->RunPass<compiler::CodegenInterpreter>();
}

static bool RunIrtocOptimizations(Graph *graph)
{
    if (!compiler::g_options.IsCompilerNonOptimizing()) {
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::Cse>();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif  // NDEBUG
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Lowering>();
        graph->RunPass<compiler::CodeSink>();
        graph->RunPass<compiler::MemoryCoalescing>(compiler::g_options.IsCompilerMemoryCoalescingAligned());
        graph->RunPass<compiler::IfConversion>(compiler::g_options.GetCompilerIfConversionLimit());
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::Scheduler>();
        // Perform MoveConstants after Scheduler because Scheduler can rearrange constants
        // and cause spillfill in reg alloc
        graph->RunPass<compiler::MoveConstants>();
    }

    graph->RunPass<compiler::Cleanup>();
    if (!compiler::RegAlloc(graph)) {
        LOG(FATAL, IRTOC) << "RunOptimizations failed: register allocation error";
        return false;
    }

    if (graph->GetMode().IsFastPath()) {
        if (!graph->RunPass<compiler::CodegenFastPath>()) {
            LOG(FATAL, IRTOC) << "RunOptimizations failed: code generation error";
            return false;
        }
    } else if (graph->GetMode().IsBoundary()) {
        if (!graph->RunPass<compiler::CodegenBoundary>()) {
            LOG(FATAL, IRTOC) << "RunOptimizations failed: code generation error";
            return false;
        }
    } else if (graph->GetMode().IsNativePlus()) {
        if (!graph->RunPass<compiler::CodegenNativePlus>()) {
            LOG(FATAL, IRTOC) << "RunOptimizations failed: code generation error";
            return false;
        }
    } else {
        UNREACHABLE();
    }

    return true;
}

}  // namespace ark::irtoc
