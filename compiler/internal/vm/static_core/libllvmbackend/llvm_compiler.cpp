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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "compiler_options.h"

#include "llvm_compiler.h"
#include "llvm_ark_interface.h"
#include "llvm_logger.h"
#include "llvm_options.h"

#include <llvm/Config/llvm-config.h>
#include <llvm/ExecutionEngine/Interpreter.h>
#include <llvm/InitializePasses.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/PassRegistry.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/StringSaver.h>

namespace ark::llvmbackend {

static llvm::llvm_shutdown_obj g_shutdown {};

LLVMCompiler::LLVMCompiler(Arch arch) : arch_(arch)
{
#ifndef REQUIRED_LLVM_VERSION
#error Internal build error, missing cmake variable
#else
#define STRINGIFY(s) STR(s)  // NOLINT(cppcoreguidelines-macro-usage)
#define STR(s) #s            // NOLINT(cppcoreguidelines-macro-usage)
    // LLVM_VERSION_STRING is defined in llvm-config.h
    // REQUIRED_LLVM_VERSION is defined in libllvmbackend/CMakeLists.txt
    static_assert(std::string_view {LLVM_VERSION_STRING} == STRINGIFY(REQUIRED_LLVM_VERSION));

    const std::string currentLlvmLibVersion = llvm::LLVMContext::getLLVMVersion();
    if (currentLlvmLibVersion != STRINGIFY(REQUIRED_LLVM_VERSION)) {
        llvm::report_fatal_error(llvm::Twine("Incompatible LLVM version " + currentLlvmLibVersion + ". " +
                                             std::string(STRINGIFY(REQUIRED_LLVM_VERSION)) + " is required."),
                                 false); /* gen_crash_diag = false */
    }
#undef STR
#undef STRINGIFY
#endif
    InitializeLLVMTarget(arch);
    InitializeLLVMPasses();
#ifdef NDEBUG
    context_.setDiscardValueNames(true);
#endif
    context_.setOpaquePointers(true);

    LLVMLogger::Init(g_options.GetLlvmLog());
}

bool LLVMCompiler::IsInliningDisabled()
{
    if (ark::compiler::g_options.WasSetCompilerInlining() && !g_options.WasSetLlvmInlining()) {
        return !ark::compiler::g_options.IsCompilerInlining();
    }
    return !g_options.IsLlvmInlining();
}

bool LLVMCompiler::IsInliningDisabled(compiler::Graph *graph)
{
    ASSERT(graph != nullptr);

    // Erased end block means infinite loop in the 'Graph', such method should not be inlined.
    if (graph->GetEndBlock() == nullptr) {
        return true;
    }

    // Since we do not generate code for 'Catch' blocks, LLVM can not inline function with
    // 'Try' blocks. Otherwise, if the inlined code throws, execution is on another frame and
    // it is impossible to find the 'Catch' block. The approach here is same with Ark Compiler.
    // Also, we can not just check 'graph->GetTryBeginBlocks().empty()', because 'TryCatchResolving'
    // pass may remove 'TryBegin' blocks, but keep some 'Try' blocks.
    for (auto block : graph->GetBlocksRPO()) {
        if (block->IsTry()) {
            return true;
        }
    }

    // NB! LLVM does not follow '--compiler-inlining-skip-always-throw-methods=false' option,
    // as it may lead to incorrect 'NoReturn' attribute propagation.
    // Loop below checks if the function exits only using 'Throw' or 'Deoptimize' opcode
    bool alwaysThrowOrDeopt = true;
    for (auto pred : graph->GetEndBlock()->GetPredsBlocks()) {
        if (pred->GetLastInst() == nullptr || !(pred->GetLastInst()->GetOpcode() == compiler::Opcode::Throw ||
                                                pred->GetLastInst()->GetOpcode() == compiler::Opcode::Deoptimize)) {
            alwaysThrowOrDeopt = false;
            break;
        }
    }
    return alwaysThrowOrDeopt || IsInliningDisabled(graph->GetRuntime(), graph->GetMethod());
}

bool LLVMCompiler::IsInliningDisabled(compiler::RuntimeInterface *runtime, compiler::RuntimeInterface::MethodPtr method)
{
    ASSERT(runtime != nullptr);
    ASSERT(method != nullptr);

    if (IsInliningDisabled()) {
        return true;
    }

    auto skipList = ark::compiler::g_options.GetCompilerInliningBlacklist();
    if (!skipList.empty()) {
        std::string methodName = runtime->GetMethodFullName(method);
        if (std::find(skipList.begin(), skipList.end(), methodName) != skipList.end()) {
            return true;
        }
    }

    return (runtime->GetMethodName(method).find("__noinline__") != std::string::npos);
}

ark::llvmbackend::LLVMCompilerOptions LLVMCompiler::InitializeLLVMCompilerOptions()
{
    ark::llvmbackend::LLVMCompilerOptions llvmCompilerOptions {};
    llvmCompilerOptions.optimize = !ark::compiler::g_options.IsCompilerNonOptimizing();
    llvmCompilerOptions.optlevel = llvmCompilerOptions.optimize ? 2U : 0U;
    llvmCompilerOptions.gcIntrusion = g_options.IsLlvmGc();
    llvmCompilerOptions.gcIntrusionChecks = g_options.IsLlvmGc() && g_options.IsLlvmGcCheck();
    llvmCompilerOptions.useSafepoint = ark::compiler::g_options.IsCompilerUseSafepoint();
    llvmCompilerOptions.dumpModuleAfterOptimizations = g_options.IsLlvmDumpAfter();
    llvmCompilerOptions.dumpModuleBeforeOptimizations = g_options.IsLlvmDumpBefore();
    llvmCompilerOptions.inlineModuleFile = g_options.GetLlvmInlineModule();
    llvmCompilerOptions.pipelineFile = g_options.GetLlvmPipeline();

    llvmCompilerOptions.inlining = !IsInliningDisabled();
    llvmCompilerOptions.recursiveInlining = g_options.IsLlvmRecursiveInlining();
    llvmCompilerOptions.doIrtocInline = !llvmCompilerOptions.inlineModuleFile.empty();
    llvmCompilerOptions.doVirtualInline = !ark::compiler::g_options.IsCompilerNoVirtualInlining();

    return llvmCompilerOptions;
}

void LLVMCompiler::InitializeDefaultLLVMOptions()
{
    if (ark::compiler::g_options.IsCompilerNonOptimizing()) {
        SetLLVMOption("fast-isel", "false");
        SetLLVMOption("global-isel", "false");
    } else {
        SetLLVMOption("fixup-allow-gcptr-in-csr", "true");
        SetLLVMOption("max-registers-for-gc-values", std::to_string(std::numeric_limits<int>::max()));
    }
}

void LLVMCompiler::InitializeLLVMOptions()
{
    llvm::cl::ResetAllOptionOccurrences();
    InitializeDefaultLLVMOptions();
    auto llvmOptionsStr = llvmPreOptions_ + " " + g_options.GetLlvmOptions();
    LLVM_LOG(DEBUG, INFRA) << "Using the following llvm options: '" << llvmPreOptions_ << "' (set internally), and '"
                           << g_options.GetLlvmOptions() << "' (from the option to aot compiler)";
    llvm::BumpPtrAllocator alloc;
    llvm::StringSaver stringSaver(alloc);
    llvm::SmallVector<const char *, 0> parsedArgv;
    parsedArgv.emplace_back("");  // First argument is an executable
    llvm::cl::TokenizeGNUCommandLine(llvmOptionsStr, stringSaver, parsedArgv);
    llvm::cl::ParseCommandLineOptions(parsedArgv.size(), parsedArgv.data());
#ifndef NDEBUG
    parsedOptions_ = true;
#endif
}

void LLVMCompiler::SetLLVMOption(const char *option, const std::string &value)
{
#ifndef NDEBUG
    ASSERT_PRINT(!parsedOptions_, "Should call SetLLVMOptions earlier");
#endif
    std::string prefix {llvmPreOptions_.empty() ? "" : " "};
    llvmPreOptions_ += prefix + "--" + option + (value.empty() ? "" : "=") + value;
}

llvm::Triple LLVMCompiler::GetTripleForArch(Arch arch)
{
    std::string error;
    std::string tripleName;
    switch (arch) {
        case Arch::AARCH64:
            tripleName = g_options.GetLlvmTriple();
            break;
        case Arch::X86_64:
#ifdef PANDA_TARGET_LINUX
            tripleName = g_options.WasSetLlvmTriple() ? g_options.GetLlvmTriple() : "x86_64-unknown-linux-gnu";
#elif defined(PANDA_TARGET_MACOS)
            tripleName = g_options.WasSetLlvmTriple() ? g_options.GetLlvmTriple() : "x86_64-apple-darwin-gnu";
#elif defined(PANDA_TARGET_WINDOWS)
            tripleName = g_options.WasSetLlvmTriple() ? g_options.GetLlvmTriple() : "x86_64-unknown-windows-unknown";
#else
            tripleName = g_options.WasSetLlvmTriple() ? g_options.GetLlvmTriple() : "x86_64-unknown-unknown-unknown";
#endif
            break;

        default:
            UNREACHABLE();
    }
    llvm::Triple triple(llvm::Triple::normalize(tripleName));
    [[maybe_unused]] auto target = llvm::TargetRegistry::lookupTarget("", triple, error);
    ASSERT_PRINT(target != nullptr, error);
    return triple;
}

std::string LLVMCompiler::GetCPUForArch(Arch arch)
{
    std::string cpu;
    switch (arch) {
        case Arch::AARCH64:
#if defined(PANDA_TARGET_ARM64) && defined(PANDA_TARGET_LINUX)
            // Avoid specifying default cortex for arm64-linux
            cpu = g_options.WasSetLlvmCpu() ? g_options.GetLlvmCpu() : "";
#else
            cpu = g_options.GetLlvmCpu();
#endif
            break;
        case Arch::X86_64:
            cpu = g_options.WasSetLlvmCpu() ? g_options.GetLlvmCpu() : "";
            break;
        default:
            UNREACHABLE();
    }
    return cpu;
}

void LLVMCompiler::InitializeLLVMTarget(Arch arch)
{
    switch (arch) {
        case Arch::AARCH64: {
            LLVMInitializeAArch64TargetInfo();
            LLVMInitializeAArch64Target();
            LLVMInitializeAArch64TargetMC();
            LLVMInitializeAArch64AsmPrinter();
            LLVMInitializeAArch64AsmParser();
            break;
        }
#ifdef PANDA_TARGET_AMD64
        case Arch::X86_64: {
            LLVMInitializeX86TargetInfo();
            LLVMInitializeX86Target();
            LLVMInitializeX86TargetMC();
            LLVMInitializeX86AsmPrinter();
            LLVMInitializeX86AsmParser();
            break;
        }
#endif
        default:
            LLVM_LOG(FATAL, INFRA) << GetArchString(arch) << std::string(" is unsupported. ");
    }
    LLVM_LOG(DEBUG, INFRA) << "Available targets";
    for (auto target : llvm::TargetRegistry::targets()) {
        LLVM_LOG(DEBUG, INFRA) << "\t" << target.getName();
    }
}

void LLVMCompiler::InitializeLLVMPasses()
{
    llvm::PassRegistry &registry = *llvm::PassRegistry::getPassRegistry();
    initializeCore(registry);
    initializeScalarOpts(registry);
    initializeObjCARCOpts(registry);
    initializeVectorization(registry);
    initializeIPO(registry);
    initializeAnalysis(registry);
    initializeTransformUtils(registry);
    initializeInstCombine(registry);
    initializeAggressiveInstCombine(registry);
    initializeInstrumentation(registry);
    initializeTarget(registry);
    initializeCodeGen(registry);
    initializeGlobalISel(registry);
}

}  // namespace ark::llvmbackend
