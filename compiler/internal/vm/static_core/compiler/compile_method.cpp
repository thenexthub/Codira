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

#include "optimizer_run.h"
#include "compile_method.h"
#include "libarkbase/mem/pool_manager.h"
#include "libarkbase/mem/code_allocator.h"
#include "include/class.h"
#include "include/method.h"
#include "optimizer/ir/ir_constructor.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/pass.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "libarkbase/utils/logger.h"
#include "code_info/code_info.h"
#include "libarkbase/events/events.h"
#include "libarkbase/trace/trace.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/code_generator/codegen.h"
#include "libarkbase/utils/utils.h"

namespace ark::compiler {

#ifdef PANDA_COMPILER_DEBUG_INFO
static Span<uint8_t> EmitElf(Graph *graph, CodeAllocator *codeAllocator, ArenaAllocator *gdbDebugInfoAllocator,
                             const std::string &methodName);
#endif

void JITStats::SetCompilationStart()
{
    ASSERT(startTime_ == 0);
    startTime_ = time::GetCurrentTimeInNanos();
}
void JITStats::EndCompilationWithStats(const std::string &methodName, bool isOsr, size_t bcSize, size_t codeSize)
{
    ASSERT(startTime_ != 0);
    auto time = time::GetCurrentTimeInNanos() - startTime_;
    statsList_.push_back(Entry {PandaString(methodName, internalAllocator_->Adapter()), isOsr, bcSize, codeSize, time});
    startTime_ = 0;
}

void JITStats::ResetCompilationStart()
{
    startTime_ = 0;
}

void JITStats::DumpCsv(char sep)
{
    ASSERT(g_options.WasSetCompilerDumpJitStatsCsv());
    std::ofstream csv(g_options.GetCompilerDumpJitStatsCsv(), std::ofstream::trunc);
    for (const auto &i : statsList_) {
        csv << "\"" << i.methodName << "\"" << sep;
        csv << i.isOsr << sep;
        csv << i.bcSize << sep;
        csv << i.codeSize << sep;
        csv << i.time;
        csv << '\n';
    }
}

struct EventCompilationArgs {
    const std::string methodName_;
    bool isOsr;
    size_t bcSize;
    uintptr_t address;
    size_t codeSize;
    size_t infoSize;
    events::CompilationStatus status;
};

static void EndCompilation(const EventCompilationArgs &args, JITStats *jitStats)
{
    [[maybe_unused]] auto [methodName, isOsr, bcSize, address, codeSize, infoSize, status] = args;
    EVENT_COMPILATION(methodName, isOsr, bcSize, address, codeSize, infoSize, status);
    if (jitStats != nullptr) {
        ASSERT((codeSize != 0) == (status == events::CompilationStatus::COMPILED));
        jitStats->EndCompilationWithStats(methodName, isOsr, bcSize, codeSize);
    }
}

Arch ChooseArch(Arch arch)
{
    if (arch != Arch::NONE) {
        return arch;
    }

    arch = RUNTIME_ARCH;
    if (RUNTIME_ARCH == Arch::X86_64 && g_options.WasSetCompilerCrossArch()) {
        arch = GetArchFromString(g_options.GetCompilerCrossArch());
    }

    return arch;
}

static bool CheckSingleImplementation(Graph *graph)
{
    // Check that all methods that were inlined due to its single implementation property, still have this property,
    // otherwise we must drop compiled code.
    // NOTE(compiler): we need to reset hotness counter hereby avoid yet another warmup phase.
    auto cha = graph->GetRuntime()->GetCha();
    ASSERT(cha != nullptr);
    for (auto siMethod : graph->GetSingleImplementationList()) {
        if (!cha->IsSingleImplementation(siMethod)) {
            LOG(WARNING, COMPILER)
                << "Method lost single-implementation property after compilation, so we need to drop "
                   "whole compiled code: "
                << graph->GetRuntime()->GetMethodFullName(siMethod);
            return false;
        }
    }
    return true;
}

static Span<uint8_t> EmitCode(const Graph *graph, CodeAllocator *allocator)
{
    size_t codeOffset = RoundUp(CodePrefix::STRUCT_SIZE, GetCodeAlignment(graph->GetArch()));
    CodePrefix prefix;
    prefix.codeSize = graph->GetCode().size();
    prefix.codeInfoOffset = codeOffset + RoundUp(graph->GetCode().size(), sizeof(uint32_t));
    prefix.codeInfoSize = graph->GetCodeInfoData().size();
    size_t codeSize = prefix.codeInfoOffset + prefix.codeInfoSize;
    auto memRange = allocator->AllocateCodeUnprotected(codeSize);
    if (memRange.GetSize() == 0) {
        return Span<uint8_t> {};
    }

    auto data = reinterpret_cast<uint8_t *>(memRange.GetData());
    MemcpyUnsafe(data, &prefix, sizeof(CodePrefix));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    MemcpyUnsafe(&data[codeOffset], graph->GetCode().data(), graph->GetCode().size());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    MemcpyUnsafe(&data[prefix.codeInfoOffset], graph->GetCodeInfoData().data(), graph->GetCodeInfoData().size());

    allocator->ProtectCode(memRange);

    return Span<uint8_t>(reinterpret_cast<uint8_t *>(memRange.GetData()), codeSize);
}

struct EmitCodeArgs {
    Graph *graph;
    CodeAllocator *codeAllocator;
    ArenaAllocator *gdbDebugInfoAllocator;
    const std::string methodName_;
};

static uint8_t *GetEntryPoint(EmitCodeArgs &&args, [[maybe_unused]] Method *method, [[maybe_unused]] bool isOsr,
                              JITStats *jitStats)
{
    [[maybe_unused]] auto [graph, codeAllocator, gdbDebugInfoAllocator, methodName] = args;
#ifdef PANDA_COMPILER_DEBUG_INFO
    auto generatedData = g_options.IsCompilerEmitDebugInfo()
                             ? EmitElf(graph, codeAllocator, gdbDebugInfoAllocator, methodName)
                             : EmitCode(graph, codeAllocator);
#else
    auto generatedData = EmitCode(graph, codeAllocator);
#endif
    if (generatedData.Empty()) {
        LOG(INFO, COMPILER) << "Compilation failed due to memory allocation fail: " << methodName;
        return nullptr;
    }
    CodeInfo codeInfo(generatedData);
    LOG(INFO, COMPILER) << "Compiled code for '" << methodName << "' has been installed to "
                        << bit_cast<void *>(codeInfo.GetCode()) << ", code size " << codeInfo.GetCodeSize();

    auto entryPoint = const_cast<uint8_t *>(codeInfo.GetCode());
    EndCompilation(EventCompilationArgs {methodName, isOsr, method->GetCodeSize(),
                                         reinterpret_cast<uintptr_t>(entryPoint), codeInfo.GetCodeSize(),
                                         codeInfo.GetInfoSize(), events::CompilationStatus::COMPILED},
                   jitStats);
    return entryPoint;
}

template <TaskRunnerMode RUNNER_MODE>
static void RunOptimizations(CompilerTaskRunner<RUNNER_MODE> taskRunner, JITStats *jitStats)
{
    auto &taskCtx = taskRunner.GetContext();
    taskCtx.GetGraph()->SetLanguage(taskCtx.GetMethod()->GetClass()->GetSourceLang());

    taskRunner.AddCallbackOnSuccess([]([[maybe_unused]] CompilerContext<RUNNER_MODE> &compilerCtx) {
        LOG(DEBUG, COMPILER) << "The method " << compilerCtx.GetMethodName() << " is compiled";
    });
    taskRunner.AddCallbackOnFail([jitStats](CompilerContext<RUNNER_MODE> &compilerCtx) {
        if (!compiler::g_options.IsCompilerIgnoreFailures()) {
            LOG(FATAL, COMPILER) << "RunOptimizations failed!";
        }
        LOG(WARNING, COMPILER) << "RunOptimizations failed!";
        EndCompilation(EventCompilationArgs {compilerCtx.GetMethodName(), compilerCtx.IsOsr(),
                                             compilerCtx.GetMethod()->GetCodeSize(), 0, 0, 0,
                                             events::CompilationStatus::FAILED},
                       jitStats);
    });

    // Run compiler optimizations over created graph
    RunOptimizations<RUNNER_MODE>(std::move(taskRunner));
}

struct CheckCompilationArgs {
    RuntimeInterface *runtime;
    CodeAllocator *codeAllocator;
    ArenaAllocator *gdbDebugInfoAllocator;
    JITStats *jitStats;
    bool isDynamic;
    Arch arch;
};

template <TaskRunnerMode RUNNER_MODE>
static bool CheckCompilation(CheckCompilationArgs &&args, CompilerContext<RUNNER_MODE> &compilerCtx)
{
    auto [runtime, codeAllocator, gdbDebugInfoAllocator, jitStats, isDynamic, arch] = args;
    auto *graph = compilerCtx.GetGraph();

    ASSERT(graph != nullptr && graph->GetCode().data() != nullptr);

    auto &name = compilerCtx.GetMethodName();
    auto isOsr = compilerCtx.IsOsr();
    auto *method = compilerCtx.GetMethod();

    if (!isDynamic && !CheckSingleImplementation(graph)) {
        EndCompilation(EventCompilationArgs {name, isOsr, method->GetCodeSize(), 0, 0, 0,
                                             events::CompilationStatus::FAILED_SINGLE_IMPL},
                       jitStats);
        return false;
    }

    // Drop non-native code in any case
    if (arch != RUNTIME_ARCH) {
        EndCompilation(
            EventCompilationArgs {name, isOsr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::DROPPED},
            jitStats);
        return false;
    }

    auto entryPoint =
        GetEntryPoint(EmitCodeArgs {graph, codeAllocator, gdbDebugInfoAllocator, name}, method, isOsr, jitStats);
    if (entryPoint == nullptr) {
        return false;
    }
    if (isOsr) {
        if (!runtime->TrySetOsrCode(method, entryPoint)) {
            // Compiled code has been deoptimized, so we shouldn't install osr code.
            // NOTE(compiler): release compiled code memory, when CodeAllocator supports freeing the memory.
            return false;
        }
    } else {
        runtime->SetCompiledEntryPoint(method, entryPoint);
    }
    return true;
}

template <TaskRunnerMode RUNNER_MODE>
void JITCompileMethod(RuntimeInterface *runtime, CodeAllocator *codeAllocator, ArenaAllocator *gdbDebugInfoAllocator,
                      JITStats *jitStats, CompilerTaskRunner<RUNNER_MODE> taskRunner)
{
    auto &taskCtx = taskRunner.GetContext();
    auto *taskMethod = taskCtx.GetMethod();
    taskCtx.SetMethodName(runtime->GetMethodFullName(taskMethod, false));
    auto &methodName = taskCtx.GetMethodName();

    SCOPED_TRACE_STREAM << "JIT compiling " << methodName;

    bool regex = g_options.WasSetCompilerRegex();
    bool regexWithSign = g_options.WasSetCompilerRegexWithSignature();
    ASSERT_PRINT(!(regex && regexWithSign),
                 "'--compiler-regex' and '--compiler-regex-with-signature' cannot be used together.");
    if ((regex || regexWithSign) && !g_options.MatchesRegex(runtime->GetMethodFullName(taskMethod, regexWithSign))) {
        LOG(DEBUG, COMPILER) << "Skip the method due to regexp mismatch: "
                             << runtime->GetMethodFullName(taskMethod, true);
        taskCtx.SetCompilationStatus(false);
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(taskRunner), false);
        return;
    }

    if (jitStats != nullptr) {
        jitStats->SetCompilationStart();
    }

    taskRunner.AddFinalize([jitStats](CompilerContext<RUNNER_MODE> &compilerCtx) {
        if (jitStats != nullptr) {
            // Reset compilation start time in all cases for consistency
            jitStats->ResetCompilationStart();
        }
        auto *graph = compilerCtx.GetGraph();
        if (graph != nullptr) {
            graph->~Graph();
        }
    });

    auto arch = ChooseArch(Arch::NONE);
    bool isDynamic = ark::panda_file::IsDynamicLanguage(taskMethod->GetClass()->GetSourceLang());

    taskRunner.AddCallbackOnSuccess([runtime, codeAllocator, gdbDebugInfoAllocator, jitStats, isDynamic,
                                     arch](CompilerContext<RUNNER_MODE> &compilerCtx) {
        bool compilationStatus = CheckCompilation<RUNNER_MODE>(
            CheckCompilationArgs {runtime, codeAllocator, gdbDebugInfoAllocator, jitStats, isDynamic, arch},
            compilerCtx);
        compilerCtx.SetCompilationStatus(compilationStatus);
    });
    taskRunner.AddCallbackOnFail(
        [](CompilerContext<RUNNER_MODE> &compilerCtx) { compilerCtx.SetCompilationStatus(false); });

    CompileInGraph<RUNNER_MODE>(runtime, isDynamic, arch, std::move(taskRunner), jitStats);
}

template <TaskRunnerMode RUNNER_MODE>
void CompileInGraph(RuntimeInterface *runtime, bool isDynamic, Arch arch, CompilerTaskRunner<RUNNER_MODE> taskRunner,
                    JITStats *jitStats)
{
    auto &taskCtx = taskRunner.GetContext();
    auto isOsr = taskCtx.IsOsr();
    auto *method = taskCtx.GetMethod();
    auto &methodName = taskCtx.GetMethodName();

    LOG(INFO, COMPILER) << "Compile method" << (isOsr ? "(OSR)" : "") << ": " << methodName << " ("
                        << runtime->GetFileName(method) << ')';

#ifdef __APPLE__
    LOG(DEBUG, COMPILER) << "Compilation unsupported for this platform!";
    CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(taskRunner), false);
    return;
#else
    if (arch == Arch::NONE || !BackendSupport(arch)) {
        LOG(DEBUG, COMPILER) << "Compilation unsupported for this platform!";
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(taskRunner), false);
        return;
    }
#endif

    auto *allocator = taskCtx.GetAllocator();
    auto *localAllocator = taskCtx.GetLocalAllocator();
    auto *graph = allocator->template New<Graph>(Graph::GraphArgs {allocator, localAllocator, arch, method, runtime},
                                                 nullptr, isOsr, isDynamic);
    taskCtx.SetGraph(graph);
    if (graph == nullptr) {
        LOG(ERROR, COMPILER) << "Creating graph failed!";
        EndCompilation(
            EventCompilationArgs {methodName, isOsr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::FAILED},
            jitStats);
        CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(taskRunner), false);
        return;
    }

    taskRunner.SetTaskOnSuccess([jitStats](CompilerTaskRunner<RUNNER_MODE> nextRunner) {
        RunOptimizations<RUNNER_MODE>(std::move(nextRunner), jitStats);
    });

    bool success = graph->template RunPass<IrBuilder>();
    if (!success) {
        if (!compiler::g_options.IsCompilerIgnoreFailures()) {
            LOG(FATAL, COMPILER) << "IrBuilder failed!";
        }
        LOG(WARNING, COMPILER) << "IrBuilder failed!";
        EndCompilation(
            EventCompilationArgs {methodName, isOsr, method->GetCodeSize(), 0, 0, 0, events::CompilationStatus::FAILED},
            jitStats);
    };
    CompilerTaskRunner<RUNNER_MODE>::EndTask(std::move(taskRunner), success);
}

template void JITCompileMethod<BACKGROUND_MODE>(RuntimeInterface *, CodeAllocator *, ArenaAllocator *, JITStats *,
                                                CompilerTaskRunner<BACKGROUND_MODE>);
template void JITCompileMethod<INPLACE_MODE>(RuntimeInterface *, CodeAllocator *, ArenaAllocator *, JITStats *,
                                             CompilerTaskRunner<INPLACE_MODE>);
template void CompileInGraph<BACKGROUND_MODE>(RuntimeInterface *, bool, Arch, CompilerTaskRunner<BACKGROUND_MODE>,
                                              JITStats *);
template void CompileInGraph<INPLACE_MODE>(RuntimeInterface *, bool, Arch, CompilerTaskRunner<INPLACE_MODE>,
                                           JITStats *);
template void RunOptimizations<BACKGROUND_MODE>(CompilerTaskRunner<BACKGROUND_MODE>, JITStats *);
template void RunOptimizations<INPLACE_MODE>(CompilerTaskRunner<INPLACE_MODE>, JITStats *);
template bool CheckCompilation<BACKGROUND_MODE>(CheckCompilationArgs &&args, CompilerContext<BACKGROUND_MODE> &);
template bool CheckCompilation<INPLACE_MODE>(CheckCompilationArgs &&args, CompilerContext<INPLACE_MODE> &);

}  // namespace ark::compiler

#ifdef PANDA_COMPILER_DEBUG_INFO

#include "optimizer/ir/aot_data.h"
#include "tools/debug/jit_writer.h"

// Next "C"-code need for enable interaction with gdb
// Please read "JIT Compilation Interface" from gdb-documentation for more information
extern "C" {
// Gdb will replace implementation of this function
// NOLINTNEXTLINE(readability-identifier-naming)
void NO_INLINE __jit_debug_register_code(void)
{
    // NOLINTNEXTLINE(hicpp-no-assembler)
    asm("");
}

// Default version for descriptor (may be checked before register code)
// NOLINTNEXTLINE(modernize-use-nullptr, readability-identifier-naming)
jit_descriptor __jit_debug_descriptor = {1, JIT_NOACTION, NULL, NULL};  // CC-OFF(G.EXP.01-CPP) public API
}  // extern "C"

namespace ark::compiler {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static os::memory::Mutex g_jitDebugLock;

// Will register jit-elf description in linked list
static void RegisterJitCode(jit_code_entry *entry)
{
    ASSERT(g_options.IsCompilerEmitDebugInfo());

    os::memory::LockHolder lock(g_jitDebugLock);
    // Re-link list
    entry->nextEntry = __jit_debug_descriptor.firstEntry;
    if (__jit_debug_descriptor.firstEntry != nullptr) {
        __jit_debug_descriptor.firstEntry->prevEntry = entry;
    }
    __jit_debug_descriptor.firstEntry = entry;

    // Fill last entry
    __jit_debug_descriptor.relevantEntry = entry;
    __jit_debug_descriptor.actionFlag = JIT_REGISTER_FN;

    // Call gdb-callback
    __jit_debug_register_code();
    __jit_debug_descriptor.actionFlag = JIT_NOACTION;
    __jit_debug_descriptor.relevantEntry = nullptr;
}

// When code allocator cleaned - also will clean entry
void CleanJitDebugCode()
{
    ASSERT(g_options.IsCompilerEmitDebugInfo());

    os::memory::LockHolder lock(g_jitDebugLock);
    __jit_debug_descriptor.actionFlag = JIT_UNREGISTER_FN;

    while (__jit_debug_descriptor.firstEntry != nullptr) {
        __jit_debug_descriptor.firstEntry->prevEntry = nullptr;
        __jit_debug_descriptor.relevantEntry = __jit_debug_descriptor.firstEntry;
        // Call gdb-callback
        __jit_debug_register_code();

        __jit_debug_descriptor.firstEntry = __jit_debug_descriptor.firstEntry->nextEntry;
    }

    __jit_debug_descriptor.actionFlag = JIT_NOACTION;
    __jit_debug_descriptor.relevantEntry = nullptr;
}

// For each jit code - will generate small elf description and put them in gdb-special linked list.
static Span<uint8_t> EmitElf(Graph *graph, CodeAllocator *codeAllocator, ArenaAllocator *gdbDebugInfoAllocator,
                             const std::string &methodName)
{
    ASSERT(g_options.IsCompilerEmitDebugInfo());

    if (graph->GetCode().Empty()) {
        return {};
    }

    JitDebugWriter jitWriter(graph->GetArch(), graph->GetRuntime(), codeAllocator, methodName);

    jitWriter.Start();

    auto method = reinterpret_cast<Method *>(graph->GetMethod());
    auto klass = reinterpret_cast<Class *>(graph->GetRuntime()->GetClass(method));
    jitWriter.StartClass(*klass);

    CompiledMethod compiledMethod(graph->GetArch(), method, 0);
    compiledMethod.SetCode(graph->GetCode().ToConst());
    compiledMethod.SetCodeInfo(graph->GetCodeInfoData());
    compiledMethod.SetCfiInfo(graph->GetCallingConvention()->GetCfiInfo());

    jitWriter.AddMethod(compiledMethod);
    jitWriter.EndClass();
    jitWriter.End();
    if (!jitWriter.Write()) {
        return {};
    }

    auto gdbEntry {gdbDebugInfoAllocator->New<jit_code_entry>()};
    if (gdbEntry == nullptr) {
        return {};
    }

    auto elfFile {jitWriter.GetElf()};
    // Pointer to Elf-file entry
    gdbEntry->symfileAddr = reinterpret_cast<const char *>(elfFile.Data());
    // Elf-in-memory file size
    gdbEntry->symfileSize = elfFile.Size();
    gdbEntry->prevEntry = nullptr;

    RegisterJitCode(gdbEntry);
    return jitWriter.GetCode();
}

}  // namespace ark::compiler
#endif
