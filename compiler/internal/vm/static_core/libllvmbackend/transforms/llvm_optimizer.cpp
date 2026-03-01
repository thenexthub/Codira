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

#include "llvm_optimizer.h"

#include "passes/aarch64_fixup_sdiv.h"
#include "passes/ark_inlining.h"
#include "passes/ark_gvn.h"
#include "passes/ark_speculation.h"
#include "passes/insert_safepoints.h"
#include "passes/gc_intrusion.h"
#include "passes/gc_intrusion_check.h"
#include "passes/intrinsics_lowering.h"
#include "passes/mem_barriers.h"
#include "passes/gep_propagation.h"
#include "passes/panda_runtime_lowering.h"
#include "passes/prune_deopt.h"
#include "passes/fixup_poisons.h"
#include "passes/expand_atomics.h"
#include "passes/devirt.h"
#include "passes/infer_flags.h"
#include "passes/inline_devirt.h"
#include "passes/loop_peeling.h"
#include "passes/propagate_lenarray.h"
#include "passes/check_external.h"

#include "passes/inline_ir/cleanup_inline_module.h"
#include "passes/inline_ir/discard_inline_module.h"
#include "passes/inline_ir/mark_always_inline.h"
#include "passes/inline_ir/mark_inline_module.h"
#include "passes/inline_ir/remove_unused_functions.h"

#include "llvm_ark_interface.h"

#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/GlobalsModRef.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/ProfileSummaryInfo.h>
#include <llvm/Transforms/Utils/CanonicalizeAliases.h>
#include <llvm/Transforms/Utils/NameAnonGlobals.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Target/TargetMachine.h>

#include <fstream>
#include <streambuf>
#include <type_traits>

#include "transforms/transform_utils.h"

namespace {

template <typename PassManagerT, typename PassT>
void AddPassIf(PassManagerT &passManager, PassT &&pass, bool needInsert = true)
{
    if (!needInsert) {
        return;
    }
    passManager.addPass(std::forward<PassT>(pass));
#ifndef NDEBUG
    // VerifierPass can be insterted only in ModulePassManager or FunctionPassManager
    constexpr auto IS_MODULE_PM = std::is_same_v<llvm::ModulePassManager, PassManagerT>;
    constexpr auto IS_FUNCTION_PM = std::is_same_v<llvm::FunctionPassManager, PassManagerT>;
    // Disable checks due to clang-tidy bug https://bugs.llvm.org/show_bug.cgi?id=32203
    // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
    if constexpr (IS_MODULE_PM || IS_FUNCTION_PM) {  // NOLINT(bugprone-suspicious-semicolon)
        passManager.addPass(llvm::VerifierPass());
    }
#endif
}

std::string PreprocessPipelineFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        llvm::report_fatal_error(llvm::Twine("Cant open pipeline file: `") + filename + "`",
                                 false); /* gen_crash_diag = false */
    }
    std::string rawData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Remove comments
    size_t insertPos = 0;
    size_t copyPos = 0;
    constexpr auto COMMENT_SYMBOL = '#';
    constexpr auto ENDLINE_SYMBOL = '\n';
    while (copyPos < rawData.size()) {
        while (copyPos < rawData.size() && rawData[copyPos] != COMMENT_SYMBOL) {
            rawData[insertPos++] = rawData[copyPos++];
        }
        while (copyPos < rawData.size() && rawData[copyPos] != ENDLINE_SYMBOL) {
            copyPos++;
        }
    }
    rawData.resize(insertPos);

    // Remove space symbols
    auto predicate = [](char sym) { return std::isspace(sym); };
    rawData.erase(std::remove_if(rawData.begin(), rawData.end(), predicate), rawData.end());

    return rawData;
}

#include <pipeline_irtoc_gen.inc>
#include <pipeline_gen.inc>

std::string GetOptimizationPipeline(const std::string &filename, bool isIrtoc)
{
    std::string pipeline;
    if (!filename.empty()) {
        pipeline = PreprocessPipelineFile(filename);
    } else if (isIrtoc) {
        /* PIPELINE_IRTOC variable is defined in pipeline_irtoc_gen.inc */
        pipeline = std::string {PIPELINE_IRTOC};
    } else {
        /* PIPELINE variable is defined in pipeline_gen.inc */
        pipeline = std::string {PIPELINE};
    }
    return pipeline;
}

}  // namespace

#include <llvm_passes.inl>

namespace ark::llvmbackend {

LLVMOptimizer::LLVMOptimizer(ark::llvmbackend::LLVMCompilerOptions options, LLVMArkInterface *arkInterface,
                             const std::unique_ptr<llvm::TargetMachine> &targetMachine)
    : options_(std::move(options)), arkInterface_(arkInterface), targetMachine_(std::move(targetMachine))
{
}

void LLVMOptimizer::ProcessInlineModule(llvm::Module *inlineModule) const
{
    namespace pass = ark::llvmbackend::passes;
    llvm::ModulePassManager modulePm;
    llvm::LoopAnalysisManager loopAm;
    llvm::FunctionAnalysisManager functionAm;
    llvm::CGSCCAnalysisManager cgsccAm;
    llvm::ModuleAnalysisManager moduleAm;

    llvm::PassBuilder passBuilder(targetMachine_.get());
    passBuilder.registerModuleAnalyses(moduleAm);
    passBuilder.registerCGSCCAnalyses(cgsccAm);
    passBuilder.registerFunctionAnalyses(functionAm);
    passBuilder.registerLoopAnalyses(loopAm);
    passBuilder.crossRegisterProxies(loopAm, functionAm, cgsccAm, moduleAm);

    AddPassIf(modulePm, llvm::CanonicalizeAliasesPass(), true);
    // Sanitizers create unnamed globals
    // 1. https://github.com/rust-lang/rust/issues/45220
    // 2. https://github.com/rust-lang/rust/pull/50684/files
    AddPassIf(modulePm, llvm::NameAnonGlobalPass(), true);
    AddPassIf(modulePm, pass::MarkInlineModule(), true);
    AddPassIf(modulePm, pass::CleanupInlineModule(), true);

    modulePm.run(*inlineModule, moduleAm);
}

void LLVMOptimizer::DumpModuleBefore(llvm::Module *module) const
{
    if (options_.dumpModuleBeforeOptimizations) {
        llvm::errs() << "; =========================================\n";
        llvm::errs() << "; LLVM IR module BEFORE LLVM optimizations:\n";
        llvm::errs() << "; =========================================\n";
        llvm::errs() << *module << '\n';
    }
}

void LLVMOptimizer::DumpModuleAfter(llvm::Module *module) const
{
    if (options_.dumpModuleAfterOptimizations) {
        llvm::errs() << "; =========================================\n";
        llvm::errs() << "; LLVM IR module AFTER LLVM optimizations: \n";
        llvm::errs() << "; =========================================\n";
        llvm::errs() << *module << '\n';
    }
}

void LLVMOptimizer::OptimizeModule(llvm::Module *module) const
{
    ASSERT(arkInterface_ != nullptr);
    // Create the analysis managers.
    llvm::LoopAnalysisManager loopAm;
    llvm::FunctionAnalysisManager functionAm;
    llvm::CGSCCAnalysisManager cgsccAm;
    llvm::ModuleAnalysisManager moduleAm;

    llvm::StandardInstrumentations instrumentation(false);
    llvm::PassInstrumentationCallbacks passInstrumentation;
    instrumentation.registerCallbacks(passInstrumentation);
    ark::libllvmbackend::RegisterPasses(passInstrumentation);

    llvm::PassBuilder passBuilder(targetMachine_.get(), llvm::PipelineTuningOptions(), llvm::None,
                                  &passInstrumentation);

    // Register the AA manager first so that our version is the one used.
    functionAm.registerPass([&passBuilder] { return passBuilder.buildDefaultAAPipeline(); });
    // Register the target library analysis directly.
    functionAm.registerPass(
        [&] { return llvm::TargetLibraryAnalysis(llvm::TargetLibraryInfoImpl(targetMachine_->getTargetTriple())); });
    // Register all the basic analyses with the managers.
    passBuilder.registerModuleAnalyses(moduleAm);
    passBuilder.registerCGSCCAnalyses(cgsccAm);
    passBuilder.registerFunctionAnalyses(functionAm);
    passBuilder.registerLoopAnalyses(loopAm);
    passBuilder.crossRegisterProxies(loopAm, functionAm, cgsccAm, moduleAm);

    auto isIrtocMode = arkInterface_->IsIrtocMode();

    ark::libllvmbackend::PassParser passParser(arkInterface_);
    passParser.RegisterParserCallbacks(passBuilder, options_);

    llvm::ModulePassManager modulePm;
    if (options_.optimize) {
        cantFail(passBuilder.parsePassPipeline(modulePm, GetOptimizationPipeline(options_.pipelineFile, isIrtocMode)));
    } else {
        llvm::FunctionPassManager functionPm;
        if (!isIrtocMode) {
            AddPassIf(functionPm, passes::PruneDeopt());
            AddPassIf(functionPm, passes::ArkGVN(arkInterface_));
            AddPassIf(functionPm, passes::MemBarriers(arkInterface_, options_.optimize));
            AddPassIf(functionPm, passes::IntrinsicsLowering(arkInterface_));
            AddPassIf(functionPm, passes::PandaRuntimeLowering(arkInterface_));
            AddPassIf(functionPm, passes::InsertSafepoints(), options_.useSafepoint);
            AddPassIf(functionPm, passes::GepPropagation());
            AddPassIf(functionPm, passes::GcIntrusion(), options_.gcIntrusion);
            AddPassIf(functionPm, passes::GcIntrusionCheck(), options_.gcIntrusionChecks);
        }
        AddPassIf(functionPm, passes::ExpandAtomics());
        modulePm.addPass(createModuleToFunctionPassAdaptor(std::move(functionPm)));
    }
    modulePm.run(*module, moduleAm);
}

}  // namespace ark::llvmbackend
