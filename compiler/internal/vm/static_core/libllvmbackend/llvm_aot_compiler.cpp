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

#include "compiler/aot/compiled_method.h"
#include "compiler/code_info/code_info.h"
#include "compiler/generated/pipeline_includes.h"
#include "libarkbase/events/events.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir_builder/ir_builder.h"
#include "optimizer/optimizations/balance_expressions.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/checks_elimination.h"
#include "optimizer/optimizations/code_sink.h"
#include "optimizer/optimizations/escape.h"
#include "optimizer/optimizations/if_merging.h"
#include "optimizer/optimizations/inlining.h"
#include "optimizer/optimizations/licm.h"
#include "optimizer/optimizations/licm_conditions.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/loop_idioms.h"
#include "optimizer/optimizations/loop_peeling.h"
#include "optimizer/optimizations/loop_unroll.h"
#include "optimizer/optimizations/loop_unswitch.h"
#include "optimizer/optimizations/lse.h"
#include "optimizer/optimizations/memory_barriers.h"
#include "optimizer/optimizations/object_type_check_elimination.h"
#include "optimizer/optimizations/optimize_string_concat.h"
#include "optimizer/optimizations/peepholes.h"
#include "optimizer/optimizations/redundant_loop_elimination.h"
#include "optimizer/optimizations/reserve_string_builder_buffer.h"
#include "optimizer/optimizations/savestate_optimization.h"
#include "optimizer/optimizations/simplify_string_builder.h"
#include "optimizer/optimizations/try_catch_resolving.h"
#include "optimizer/optimizations/vn.h"
#include "optimizer/optimizations/string_flat_check.h"
#include "optimizer/analysis/monitor_analysis.h"
#include "optimizer/optimizations/cleanup.h"
#include "runtime/include/method.h"
#include "runtime/include/thread.h"
#include "compiler_options.h"

#include "llvm_aot_compiler.h"
#include "llvm_logger.h"
#include "llvm_options.h"
#include "mir_compiler.h"
#include "target_machine_builder.h"

#include "lowering/llvm_ir_constructor.h"
#include "object_code/code_info_producer.h"
#include "transforms/passes/ark_frame_lowering/frame_lowering.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
// Suppress warning about forward Pass declaration defined in another namespace
#include <llvm/Pass.h>
#include <llvm/Support/Debug.h>

/* Be careful this file also includes elf.h, which has a lot of defines */
#include "aot/aot_builder/llvm_aot_builder.h"

#define DEBUG_TYPE "llvm-aot-compiler"

static constexpr unsigned LIMIT_MULTIPLIER = 2U;

static constexpr int32_t MAX_DEPTH = 32;

namespace {

class ScopedCompilerThread {
public:
    ScopedCompilerThread(ark::compiler::RuntimeInterface::ThreadPtr parent,
                         ark::compiler::RuntimeInterface *runtimeInterface)
        : runtimeInterface_ {runtimeInterface}
    {
        ASSERT(parent != nullptr);
        old_ = runtimeInterface->GetCurrentThread();
        runtimeInterface->SetCurrentThread(parent);
        compiler_ = runtimeInterface->CreateCompilerThread();
        runtimeInterface->SetCurrentThread(compiler_);
    }

    ScopedCompilerThread(ScopedCompilerThread &) = delete;
    ScopedCompilerThread &operator=(ScopedCompilerThread &) = delete;
    ScopedCompilerThread(ScopedCompilerThread &&) = delete;
    ScopedCompilerThread &operator=(ScopedCompilerThread &&) = delete;

    ~ScopedCompilerThread()
    {
        ASSERT(runtimeInterface_->GetCurrentThread() == compiler_);
        runtimeInterface_->DestroyCompilerThread(compiler_);
        runtimeInterface_->SetCurrentThread(old_);
    }

private:
    ark::compiler::RuntimeInterface::ThreadPtr old_ {nullptr};
    ark::compiler::RuntimeInterface::ThreadPtr compiler_ {nullptr};
    ark::compiler::RuntimeInterface *runtimeInterface_;
};

void MarkAsInlineObject(llvm::GlobalObject *globalObject)
{
    globalObject->addMetadata(ark::llvmbackend::LLVMArkInterface::FUNCTION_MD_INLINE_MODULE,
                              *llvm::MDNode::get(globalObject->getContext(), {}));
}

void UnmarkAsInlineObject(llvm::GlobalObject *globalObject)
{
    ASSERT_PRINT(globalObject->hasMetadata(ark::llvmbackend::LLVMArkInterface::FUNCTION_MD_INLINE_MODULE),
                 "Must be marked to unmark");

    globalObject->setMetadata(ark::llvmbackend::LLVMArkInterface::FUNCTION_MD_INLINE_MODULE, nullptr);
}

constexpr unsigned GetFrameSlotSize(ark::Arch arch)
{
    constexpr size_t SPILLS = 0;
    ark::CFrameLayout layout {arch, SPILLS};
    return layout.GetSlotSize();
}

bool MonitorsCorrect(ark::compiler::Graph *graph, bool nonCatchOnly)
{
    if (!nonCatchOnly) {
        return graph->RunPass<ark::compiler::MonitorAnalysis>();
    }
    graph->GetAnalysis<ark::compiler::MonitorAnalysis>().SetCheckNonCatchOnly(true);
    bool ok = graph->RunPass<ark::compiler::MonitorAnalysis>();
    graph->InvalidateAnalysis<ark::compiler::MonitorAnalysis>();
    graph->GetAnalysis<ark::compiler::MonitorAnalysis>().SetCheckNonCatchOnly(false);
    return ok;
}

}  // namespace

namespace ark::llvmbackend {

class FixedCountSpreader : public Spreader {
public:
    using MethodCount = uint64_t;
    using ModuleFactory = std::function<std::shared_ptr<WrappedModule>(uint32_t moduleId)>;

    explicit FixedCountSpreader(compiler::RuntimeInterface *runtime, MethodCount limit, ModuleFactory moduleFactory)
        : moduleFactory_(std::move(moduleFactory)), runtime_(runtime), limit_(limit)
    {
        // To avoid overflow in HardLimit()
        ASSERT(limit_ < std::numeric_limits<MethodCount>::max() / LIMIT_MULTIPLIER);
        ASSERT(limit_ > 0);
    }

    std::shared_ptr<WrappedModule> GetModuleForMethod(compiler::RuntimeInterface::MethodPtr method) override
    {
        auto &module = modules_[method];
        if (module != nullptr) {
            return module;
        }
        ASSERT(!IsOrderViolated(method));
        if (currentModule_ == nullptr) {
            currentModule_ = moduleFactory_(moduleCount_++);
        }
        if (currentModuleSize_ + 1 >= limit_) {
            bool sizeExceeds = currentModuleSize_ + 1 >= HardLimit();
            if (sizeExceeds || runtime_->GetClass(method) != lastClass_) {
                LLVM_LOG(DEBUG, INFRA) << "Creating new module, since "
                                       << (sizeExceeds ? "the module's size would exceed HardLimit" : "class changed")
                                       << ", class of current method = '" << runtime_->GetClassNameFromMethod(method)
                                       << "', lastClass_ = '"
                                       << (lastClass_ == nullptr ? "nullptr" : runtime_->GetClassName(lastClass_))
                                       << "', limit_ = " << limit_ << ", HardLimit = " << HardLimit()
                                       << ", currentModuleSize_ = " << currentModuleSize_;
                currentModuleSize_ = 0;
                currentModule_ = moduleFactory_(moduleCount_++);
                moduleCount_++;
            }
        }

        currentModuleSize_++;
        lastClass_ = runtime_->GetClass(method);
        module = currentModule_;
        LLVM_LOG(DEBUG, INFRA) << "Getting module for graph (" << runtime_->GetMethodFullName(method, true) << "), "
                               << currentModuleSize_ << " / " << limit_ << ", HardLimit = " << HardLimit();

        ASSERT(module != nullptr);
        ASSERT(!module->IsCompiled());
        ASSERT(currentModuleSize_ < HardLimit());
        return module;
    }

    std::unordered_set<std::shared_ptr<WrappedModule>> GetModules() override
    {
        std::unordered_set<std::shared_ptr<WrappedModule>> modules;
        for (auto &entry : modules_) {
            modules.insert(entry.second);
        }
        return modules;
    }

private:
    constexpr uint64_t HardLimit()
    {
        return limit_ * LIMIT_MULTIPLIER;
    }
#ifndef NDEBUG
    bool IsOrderViolated(compiler::RuntimeInterface::MethodPtr method)
    {
        auto clazz = runtime_->GetClass(method);
        auto notSeen = seenClasses_.insert(clazz).second;
        if (notSeen) {
            return false;
        }
        return clazz != lastClass_;
    }
#endif

private:
#ifndef NDEBUG
    std::unordered_set<compiler::RuntimeInterface::ClassPtr> seenClasses_;
#endif
    ModuleFactory moduleFactory_;
    compiler::RuntimeInterface *runtime_;
    const uint64_t limit_;
    uint64_t currentModuleSize_ {0};
    uint32_t moduleCount_ {0};
    compiler::RuntimeInterface::ClassPtr lastClass_ {nullptr};
    std::shared_ptr<WrappedModule> currentModule_ {nullptr};
    std::unordered_map<compiler::RuntimeInterface::MethodPtr, std::shared_ptr<WrappedModule>> modules_;
};

Expected<bool, std::string> LLVMAotCompiler::TryAddGraph(compiler::Graph *graph)
{
    ASSERT(graph != nullptr && graph->GetArch() == GetArch() && graph->GetAotData()->GetUseCha());
    ASSERT(graph->GetRuntime()->IsCompressedStringsEnabled() && graph->SupportManagedCode());

    if (graph->IsDynamicMethod()) {
        return false;
    }

    auto method = static_cast<Method *>(graph->GetMethod());
    auto module = spreader_->GetModuleForMethod(method);
    if (currentModule_ == nullptr) {
        currentModule_ = module;
    }
    if (threadPool_ == nullptr && currentModule_ != module) {
        // Compile finished module immediately before emitting the next one
        CompileModule(*currentModule_);
    }
    if (threadPool_ != nullptr && currentModule_ != module) {
        // Launch compilation on parallel worker
        auto thread = runtime_->GetCurrentThread();
        threadPool_->async(
            [this, thread](auto it) -> void {
                ScopedCompilerThread scopedCompilerThread {thread, runtime_};
                CompileModule(*it);
                {
                    std::lock_guard<decltype(mutex_)> lock {mutex_};
                    semaphore_++;
                }
                cv_.notify_all();
            },
            currentModule_);
        // If all workers busy, wait here before emitting next module
        {
            std::unique_lock<decltype(mutex_)> lock {mutex_};
            cv_.wait(lock, [&]() { return semaphore_ > 0; });
            --semaphore_;
        }
    }
    currentModule_ = module;

    // Proceed with the Graph
    auto result = AddGraphToModule(graph, *module, AddGraphMode::PRIMARY_FUNCTION);
    if (result.HasValue() && result.Value()) {
        module->AddMethod(method);
        methods_.push_back(method);
        if (!compiler::g_options.IsCompilerNonOptimizing()) {
            AddInlineFunctionsByDepth(*module, graph, 0);
        }
    }
    return result;
}

bool LLVMAotCompiler::RunArkPasses(compiler::Graph *graph)
{
    auto llvmPreOpt = g_options.GetLlvmPreOpt();
    ASSERT(llvmPreOpt <= 2U);
    graph->RunPass<compiler::Cleanup>(false);
    if (!compiler::g_options.IsCompilerNonOptimizing()) {
        // Same logic as in ark::compiler::Pipeline::RunOptimizations
        if (!(compiler::g_options.WasSetCompilerLoopUnroll() && compiler::g_options.IsCompilerLoopUnroll())) {
            graph->SetUnrollComplete();
        }
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::BranchElimination>();
        graph->RunPass<compiler::OptimizeStringConcat>();
        graph->RunPass<compiler::SimplifyStringBuilder>();
        graph->RunPass<compiler::Inlining>(llvmPreOpt == 0);
        graph->RunPass<compiler::Cleanup>();
        graph->RunPass<compiler::CatchInputs>();
        graph->RunPass<compiler::TryCatchResolving>();
        if (!MonitorsCorrect(graph, false)) {
            return false;
        }
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::BranchElimination>();
        graph->RunPass<compiler::ValNum>();
        graph->RunPass<compiler::IfMerging>();
        graph->RunPass<compiler::Cleanup>(false);
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::ChecksElimination>();
        if (llvmPreOpt == 2U) {
            PreOpt2(graph);
        }
        // Run after LSE, LICM etc to avoid redundant StringFlatCheck instruction insertions
        // and before SaveStateOptimization since it might optimize save states inserted here
        graph->RunPass<compiler::StringFlatCheck>();
        if (llvmPreOpt == 2U) {
            PreOpt2Epilog(graph);
        }
    }
#ifndef NDEBUG
    if (compiler::g_options.IsCompilerCheckGraph() && compiler::g_options.IsCompilerCheckFinal()) {
        compiler::GraphChecker(graph, "LLVM").Check();
    }
#endif

    return !compiler::g_options.IsCompilerNonOptimizing() || MonitorsCorrect(graph, true);
}

void LLVMAotCompiler::PreOpt2(compiler::Graph *graph)
{
    graph->RunPass<compiler::Licm>(compiler::g_options.GetCompilerLicmHoistLimit());
    graph->RunPass<compiler::LicmConditions>();
    graph->RunPass<compiler::RedundantLoopElimination>();
    graph->RunPass<compiler::LoopPeeling>();
    graph->RunPass<compiler::LoopUnswitch>(compiler::g_options.GetCompilerLoopUnswitchMaxLevel(),
                                           compiler::g_options.GetCompilerLoopUnswitchMaxInsts());
    graph->RunPass<compiler::Lse>();
    graph->RunPass<compiler::ValNum>();
    if (graph->RunPass<compiler::Peepholes>() && graph->RunPass<compiler::BranchElimination>()) {
        graph->RunPass<compiler::Peepholes>();
        graph->RunPass<compiler::ValNum>();
    }
    graph->RunPass<compiler::Cleanup>();

    graph->RunPass<compiler::LoopIdioms>();
    graph->RunPass<compiler::ChecksElimination>();
    if (graph->RunPass<compiler::DeoptimizeElimination>()) {
        graph->RunPass<compiler::Peepholes>();
    }
    if (compiler::g_options.WasSetCompilerLoopUnroll()) {
        graph->RunPass<compiler::LoopUnroll>(compiler::g_options.GetCompilerLoopUnrollInstLimit(),
                                             compiler::g_options.GetCompilerLoopUnrollFactor());
    }
    compiler::OptimizationsAfterUnroll(graph);
    graph->RunPass<compiler::Peepholes>();
    graph->RunPass<compiler::EscapeAnalysis>();
    graph->RunPass<compiler::ReserveStringBuilderBuffer>();
    ASSERT(graph->IsUnrollComplete());

    graph->RunPass<compiler::Peepholes>();
    graph->RunPass<compiler::BranchElimination>();
    graph->RunPass<compiler::BalanceExpressions>();
    graph->RunPass<compiler::ValNum>();
}

void LLVMAotCompiler::PreOpt2Epilog(compiler::Graph *graph)
{
    graph->RunPass<compiler::SaveStateOptimization>();
    graph->RunPass<compiler::Peepholes>();
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif  // NDEBUG
    graph->RunPass<compiler::Cleanup>(false);
    graph->RunPass<compiler::CodeSink>();
    graph->RunPass<compiler::Cleanup>(false);
}

Expected<bool, std::string> LLVMAotCompiler::AddGraphToModule(compiler::Graph *graph, WrappedModule &module,
                                                              AddGraphMode addGraphMode)
{
    auto method = graph->GetMethod();
    if (module.HasFunctionDefinition(method) && addGraphMode == AddGraphMode::PRIMARY_FUNCTION) {
        auto function = module.GetFunctionByMethodPtr(method);
        UnmarkAsInlineObject(function);
        return true;
    }
    LLVM_LOG(DEBUG, INFRA) << "Adding graph for method = '" << runtime_->GetMethodFullName(graph->GetMethod()) << "'";
    if (!RunArkPasses(graph)) {
        LLVM_LOG(WARNING, INFRA) << "Monitors are unbalanced for method = '"
                                 << runtime_->GetMethodFullName(graph->GetMethod()) << "'";
        return false;
    }
    std::string graphError = compiler::LLVMIrConstructor::CheckGraph(graph);
    if (!graphError.empty()) {
        return Unexpected(graphError);
    }

    compiler::LLVMIrConstructor ctor(graph, module.GetModule().get(), module.GetLLVMContext().get(),
                                     module.GetLLVMArkInterface().get(), module.GetDebugData());
    auto llvmFunction = ctor.GetFunc();
    bool noInline = IsInliningDisabled(graph);
    if (addGraphMode == AddGraphMode::INLINE_FUNCTION) {
        MarkAsInlineObject(llvmFunction);
    }
    bool builtIr = ctor.BuildIr(noInline);
    if (!builtIr) {
        if (addGraphMode == AddGraphMode::PRIMARY_FUNCTION) {
            irFailed_ = true;
            LLVM_LOG(ERROR, INFRA) << "LLVM AOT failed to build IR for method '"
                                   << module.GetLLVMArkInterface()->GetUniqMethodName(method) << "'";
        } else {
            ASSERT(addGraphMode == AddGraphMode::INLINE_FUNCTION);
            LLVM_LOG(WARNING, INFRA) << "LLVM AOT failed to build IR for inline function '"
                                     << module.GetLLVMArkInterface()->GetUniqMethodName(method) << "'";
        }
        llvmFunction->deleteBody();
        return Unexpected(std::string("LLVM AOT failed to build IR"));
    }

    LLVM_LOG(DEBUG, INFRA) << "LLVM AOT built LLVM IR for method  "
                           << module.GetLLVMArkInterface()->GetUniqMethodName(method);
    return true;
}

void LLVMAotCompiler::PrepareAotGot(WrappedModule *wrappedModule)
{
    static constexpr size_t ARRAY_LENGTH = 0;
    auto array64 = llvm::ArrayType::get(llvm::Type::getInt64Ty(*wrappedModule->GetLLVMContext()), ARRAY_LENGTH);

    auto aotGot =
        new llvm::GlobalVariable(*wrappedModule->GetModule(), array64, false, llvm::GlobalValue::ExternalLinkage,
                                 llvm::ConstantAggregateZero::get(array64), "__aot_got");

    aotGot->setSection(AOT_GOT_SECTION);
    aotGot->setVisibility(llvm::GlobalVariable::ProtectedVisibility);
}

WrappedModule LLVMAotCompiler::CreateModule(uint32_t moduleId)
{
    auto ttriple = GetTripleForArch(GetArch());
    auto llvmContext = std::make_unique<llvm::LLVMContext>();
    auto module = std::make_unique<llvm::Module>("panda-llvmaot-module", *llvmContext);
    auto options = InitializeLLVMCompilerOptions();
    // clang-format off
    auto targetMachine = cantFail(TargetMachineBuilder {}
                                .SetCPU(GetCPUForArch(GetArch()))
                                .SetOptLevel(static_cast<llvm::CodeGenOpt::Level>(options.optlevel))
                                .SetFeatures(GetFeaturesForArch(GetArch()))
                                .SetTriple(ttriple)
                                .Build());
    // clang-format on
    auto layout = targetMachine->createDataLayout();
    module->setDataLayout(layout);
    module->setTargetTriple(ttriple.getTriple());
    auto debugData = std::make_unique<DebugDataBuilder>(module.get(), filename_);
    auto arkInterface = std::make_unique<LLVMArkInterface>(runtime_, ttriple, aotBuilder_, &lock_);
    arkInterface.get()->CreateRequiredIntrinsicFunctionTypes(*llvmContext.get());
    compiler::LLVMIrConstructor::InsertArkFrameInfo(module.get(), GetArch());
    compiler::LLVMIrConstructor::ProvideSafepointPoll(module.get(), arkInterface.get(), GetArch());
    WrappedModule wrappedModule {
        std::move(llvmContext),    //
        std::move(module),         //
        std::move(targetMachine),  //
        std::move(arkInterface),   //
        std::move(debugData)       //
    };
    wrappedModule.SetId(moduleId);

    PrepareAotGot(&wrappedModule);
    return wrappedModule;
}

std::unique_ptr<CompilerInterface> CreateLLVMAotCompiler(compiler::RuntimeInterface *runtime, ArenaAllocator *allocator,
                                                         compiler::LLVMAotBuilder *aotBuilder,
                                                         const std::string &cmdline, const std::string &filename)
{
    return std::make_unique<LLVMAotCompiler>(runtime, allocator, aotBuilder, cmdline, filename);
}

LLVMAotCompiler::LLVMAotCompiler(compiler::RuntimeInterface *runtime, ArenaAllocator *allocator,
                                 compiler::LLVMAotBuilder *aotBuilder, std::string cmdline, std::string filename)
    : LLVMCompiler(aotBuilder->GetArch()),
      methods_(allocator->Adapter()),
      aotBuilder_(aotBuilder),
      cmdline_(std::move(cmdline)),
      filename_(std::move(filename)),
      runtime_(runtime)
{
    auto arch = aotBuilder->GetArch();
    llvm::Triple ttriple = GetTripleForArch(arch);
    auto llvmCompilerOptions = InitializeLLVMCompilerOptions();
    SetLLVMOption("spill-slot-min-size-bytes", std::to_string(GetFrameSlotSize(arch)));
    // Disable some options of PlaceSafepoints pass
    SetLLVMOption("spp-no-entry", "true");
    SetLLVMOption("spp-no-call", "true");
    // Set limit to skip Safepoints on entry for small functions
    SetLLVMOption("isp-on-entry-limit", std::to_string(compiler::g_options.GetCompilerSafepointEliminationLimit()));
    SetLLVMOption("enable-implicit-null-checks", compiler::g_options.IsCompilerImplicitNullCheck() ? "true" : "false");
    SetLLVMOption("imp-null-check-page-size", std::to_string(runtime->GetProtectedMemorySize()));
    if (arch == Arch::AARCH64) {
        SetLLVMOption("aarch64-enable-ptr32", "true");
    }
    if (static_cast<mem::GCType>(aotBuilder->GetGcType()) != mem::GCType::G1_GC) {
        // NOTE(zdenis): workaround to prevent vector stores of adjacent references
        SetLLVMOption("vectorize-slp", "false");
    }

    spreader_ = std::make_unique<FixedCountSpreader>(
        runtime_, g_options.GetLlvmaotMethodsPerModule(),
        [this](uint32_t moduleId) { return std::make_shared<WrappedModule>(CreateModule(moduleId)); });
    int32_t numThreads = g_options.GetLlvmaotThreads();
    if (numThreads >= 0) {
        auto strategy = llvm::hardware_concurrency(numThreads);
        threadPool_ = std::make_unique<llvm::ThreadPool>(strategy);

        numThreads = strategy.compute_thread_count();
        ASSERT(numThreads > 0);
        semaphore_ = numThreads;
    }
    LLVMCompiler::InitializeLLVMOptions();
}

/* static */
std::vector<std::string> LLVMAotCompiler::GetFeaturesForArch(Arch arch)
{
    std::vector<std::string> features;
    features.reserve(2U);
    switch (arch) {
        case Arch::AARCH64: {
            features.emplace_back(std::string("+reserve-x") + std::to_string(GetThreadReg(arch)));
            if (compiler::g_options.IsCpuFeatureEnabled(compiler::CRC32)) {
                features.emplace_back(std::string("+crc"));
            }
            return features;
        }
        case Arch::X86_64:
            features.emplace_back(std::string("+fixed-r") + std::to_string(GetThreadReg(arch)));
            if (compiler::g_options.IsCpuFeatureEnabled(compiler::SSE42)) {
                features.emplace_back("+sse4.2");
            }
            return features;
        default:
            return {};
    }
}

AotBuilderOffsets LLVMAotCompiler::CollectAotBuilderOffsets(
    const std::unordered_set<std::shared_ptr<WrappedModule>> &modules)
{
    const auto &methodPtrs = aotBuilder_->GetMethods();
    const auto &headers = aotBuilder_->GetMethodHeaders();
    auto methodsIt = std::begin(methods_);
    for (size_t i = 0; i < headers.size(); ++i) {
        if (methodPtrs[i].GetMethod() != *methodsIt) {
            continue;
        }

        auto module = spreader_->GetModuleForMethod(*methodsIt);
        auto fullMethodName = module->GetLLVMArkInterface()->GetUniqMethodName(*methodsIt);
        std::unordered_map<std::string, CreatedObjectFile::StackMapSymbol> stackmapInfo;

        auto section = module->GetObjectFile()->GetSectionByFunctionName(fullMethodName);
        auto compiledMethod = AdaptCode(*methodsIt, {section.GetMemory(), section.GetSize()});
        aotBuilder_->AdjustMethodHeader(compiledMethod, i);
        EVENT_PAOC("Compiling " + runtime_->GetMethodFullName(*methodsIt) + " using llvm");
        methodsIt++;
    }
    ASSERT(methodsIt == std::end(methods_));

    std::vector<compiler::RoData> roDatas;
    for (auto &module : modules) {
        auto roDataSections = module->GetObjectFile()->GetRoDataSections();
        for (auto &item : roDataSections) {
            roDatas.push_back(compiler::RoData {
                item.ContentToVector(), item.GetName() + std::to_string(module->GetModuleId()), item.GetAlignment()});
        }
    }
    std::sort(roDatas.begin(), roDatas.end(),
              [](const compiler::RoData &a, const compiler::RoData &b) -> bool { return a.name < b.name; });
    aotBuilder_->SetRoDataSections(roDatas);

    // All necessary information is supplied to the aotBuilder.
    // And it can return the offsets of sections and methods where they will be located.
    auto sectionAddresses = aotBuilder_->GetSectionsAddresses(cmdline_, filename_);
    for (const auto &item : sectionAddresses) {
        LLVM_LOG(DEBUG, INFRA) << item.first << " starts at " << item.second;
    }

    std::unordered_map<std::string, size_t> methodOffsets;
    for (size_t i = 0; i < headers.size(); ++i) {
        auto method =
            const_cast<compiler::RuntimeInterface::MethodPtr>(static_cast<const void *>(methodPtrs[i].GetMethod()));
        std::string methodName =
            spreader_->GetModuleForMethod(method)->GetLLVMArkInterface()->GetUniqMethodName(methodPtrs[i].GetMethod());
        methodOffsets[methodName] = headers[i].codeOffset;
    }
    return AotBuilderOffsets {std::move(sectionAddresses), std::move(methodOffsets)};
}

compiler::CompiledMethod LLVMAotCompiler::AdaptCode(Method *method, Span<const uint8_t> machineCode)
{
    ASSERT(method != nullptr);
    ASSERT(!machineCode.empty());

    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};

    ArenaVector<uint8_t> code(allocator.Adapter());
    code.insert(code.begin(), machineCode.cbegin(), machineCode.cend());

    auto compiledMethod = compiler::CompiledMethod(GetArch(), method, 0);
    compiledMethod.SetCode(Span<const uint8_t>(code));

    compiler::CodeInfoBuilder codeInfoBuilder(GetArch(), &allocator);
    spreader_->GetModuleForMethod(method)->GetCodeInfoProducer()->Produce(method, &codeInfoBuilder);

    ArenaVector<uint8_t> codeInfo(allocator.Adapter());
    codeInfoBuilder.Encode(&codeInfo);
    ASSERT(!codeInfo.empty());
    compiledMethod.SetCodeInfo(Span<const uint8_t>(codeInfo));

    return compiledMethod;
}

ArkAotLinker::RoDataSections LLVMAotCompiler::LinkModule(WrappedModule *wrappedModule, ArkAotLinker *linker,
                                                         AotBuilderOffsets *offsets)
{
    exitOnErr_(linker->LoadObject(wrappedModule->TakeObjectFile()));
    exitOnErr_(linker->RelocateSections(offsets->GetSectionAddresses(), offsets->GetMethodOffsets(),
                                        wrappedModule->GetModuleId()));
    const auto &methodPtrs = aotBuilder_->GetMethods();
    const auto &headers = aotBuilder_->GetMethodHeaders();
    ASSERT((methodPtrs.size() == headers.size()) && (methodPtrs.size() >= methods_.size()));
    if (wrappedModule->GetMethods().empty()) {
        return {};
    }
    auto methodsIt = wrappedModule->GetMethods().begin();

    for (size_t i = 0; i < headers.size(); ++i) {
        if (methodsIt == wrappedModule->GetMethods().end()) {
            break;
        }
        if (methodPtrs[i].GetMethod() != *methodsIt) {
            continue;
        }
        auto methodName = wrappedModule->GetLLVMArkInterface()->GetUniqMethodName(*methodsIt);
        auto functionSection = linker->GetLinkedFunctionSection(methodName);
        Span<const uint8_t> code(functionSection.GetMemory(), functionSection.GetSize());
        auto compiledMethod = AdaptCode(static_cast<Method *>(*methodsIt), code);
        if (g_options.IsLlvmDumpCodeinfo()) {
            DumpCodeInfo(compiledMethod);
        }
        aotBuilder_->AdjustMethod(compiledMethod, i);
        methodsIt++;
    }
    return linker->GetLinkedRoDataSections();
}

void LLVMAotCompiler::FinishCompile()
{
    // No need to do anything if we have no methods
    if (methods_.empty()) {
        return;
    }
    ASSERT(currentModule_ != nullptr);
    ASSERT_PRINT(!compiled_, "Cannot compile twice");

    if (threadPool_ != nullptr) {
        // Proceed with last module
        auto thread = runtime_->GetCurrentThread();
        threadPool_->async(
            [this, thread](auto it) -> void {
                ScopedCompilerThread scopedCompilerThread {thread, runtime_};
                CompileModule(*it);
            },
            currentModule_);

        // Wait all workers to finish
        threadPool_->wait();
    } else {
        // Last module
        CompileModule(*currentModule_);
    }

    auto modules = spreader_->GetModules();
    std::vector<compiler::RoData> roDatas;
    auto offsets = CollectAotBuilderOffsets(modules);
    for (auto &wrappedModule : modules) {
        size_t functionHeaderSize = compiler::CodeInfo::GetCodeOffset(GetArch());
        ArkAotLinker linker {functionHeaderSize};
        auto newRodatas = LinkModule(wrappedModule.get(), &linker, &offsets);
        for (auto &item : newRodatas) {
            roDatas.push_back(compiler::RoData {item.ContentToVector(),
                                                item.GetName() + std::to_string(wrappedModule->GetModuleId()),
                                                item.GetAlignment()});
        }
    }
    compiled_ = true;

    std::sort(roDatas.begin(), roDatas.end(),
              [](const compiler::RoData &a, const compiler::RoData &b) -> bool { return a.name < b.name; });
    aotBuilder_->SetRoDataSections(std::move(roDatas));
}

void LLVMAotCompiler::AddInlineMethodByDepth(WrappedModule &module, compiler::Graph *caller,
                                             compiler::RuntimeInterface::MethodPtr method, int32_t depth)
{
    if (!runtime_->IsMethodCanBeInlined(method) || IsInliningDisabled(runtime_, method)) {
        LLVM_LOG(DEBUG, INFRA) << "Will not add inline function = '" << runtime_->GetMethodFullName(method)
                               << "' for caller = '" << runtime_->GetMethodFullName(caller->GetMethod())
                               << "' because !IsMethodCanBeInlined or IsInliningDisabled or the inline "
                                  "function is external for the caller";
        return;
    }

    auto function = module.GetFunctionByMethodPtr(method);
    if (function != nullptr && !function->isDeclarationForLinker()) {
        return;
    }
    ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
    ArenaAllocator localAllocator {SpaceType::SPACE_TYPE_COMPILER};

    auto graph = CreateGraph(allocator, localAllocator, *static_cast<Method *>(method));
    if (!graph) {
        [[maybe_unused]] auto message = llvm::toString(graph.takeError());
        LLVM_LOG(WARNING, INFRA) << "Could not add inline function = '" << runtime_->GetMethodFullName(method)
                                 << "' for caller = '" << runtime_->GetMethodFullName(caller->GetMethod())
                                 << "' because '" << message << "'";
        return;
    }
    auto callee = llvm::cantFail(std::move(graph));
    auto result = AddGraphToModule(callee, module, AddGraphMode::INLINE_FUNCTION);
    if (!result.HasValue() || !result.Value()) {
        LLVM_LOG(WARNING, INFRA) << "Could not add inline function = '" << runtime_->GetMethodFullName(method)
                                 << "' for caller = '" << runtime_->GetMethodFullName(caller->GetMethod()) << "'";
        return;
    }

    LLVM_LOG(DEBUG, INFRA) << "Added inline function = '" << runtime_->GetMethodFullName(callee->GetMethod(), true)
                           << "because '" << runtime_->GetMethodFullName(caller->GetMethod(), true) << "' calls it";
    AddInlineFunctionsByDepth(module, callee, depth + 1);
}

void LLVMAotCompiler::AddInlineFunctionsByDepth(WrappedModule &module, compiler::Graph *caller, int32_t depth)
{
    ASSERT(depth >= 0);
    LLVM_LOG(DEBUG, INFRA) << "Adding inline functions, depth = " << depth << ", caller = '"
                           << caller->GetRuntime()->GetMethodFullName(caller->GetMethod()) << "'";

    // Max depth for framework.abc is 26.
    // Limit the depth to avoid excessively large call depth
    if (depth >= MAX_DEPTH) {
        LLVM_LOG(DEBUG, INFRA) << "Exiting, inlining depth exceeded";
        return;
    }

    for (auto basicBlock : caller->GetBlocksRPO()) {
        for (auto instruction : basicBlock->AllInsts()) {
            if (instruction->GetOpcode() != compiler::Opcode::CallStatic) {
                continue;
            }

            auto method = instruction->CastToCallStatic()->GetCallMethod();
            ASSERT(method != nullptr);
            AddInlineMethodByDepth(module, caller, method, depth);
        }
    }
}

llvm::Expected<compiler::Graph *> LLVMAotCompiler::CreateGraph(ArenaAllocator &allocator,
                                                               ArenaAllocator &localAllocator, Method &method)
{
    // Part of Paoc::CompileAot
    auto sourceLang = method.GetClass()->GetSourceLang();
    bool isDynamic = panda_file::IsDynamicLanguage(sourceLang);

    auto graph = allocator.New<compiler::Graph>(
        compiler::Graph::GraphArgs {&allocator, &localAllocator, aotBuilder_->GetArch(), &method, runtime_}, nullptr,
        false, isDynamic);
    if (graph == nullptr) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(), "Couldn't create graph");
    }
    graph->SetLanguage(sourceLang);

    uintptr_t codeAddress = aotBuilder_->GetCurrentCodeAddress();
    auto aotData = graph->GetAllocator()->New<compiler::AotData>(
        compiler::AotData::AotDataArgs {method.GetPandaFile(),
                                        graph,
                                        nullptr,
                                        codeAddress,
                                        aotBuilder_->GetIntfInlineCacheIndex(),
                                        {aotBuilder_->GetGotPlt(), aotBuilder_->GetGotVirtIndexes(),
                                         aotBuilder_->GetGotClass(), aotBuilder_->GetGotString()},
                                        {aotBuilder_->GetGotIntfInlineCache(), aotBuilder_->GetGotCommon()}});

    aotData->SetUseCha(true);
    aotData->SetIsLLVMAotMode(true);
    graph->SetAotData(aotData);

    if (!graph->RunPass<compiler::IrBuilder>()) {
        return llvm::createStringError(llvm::inconvertibleErrorCode(), "IrBuilder failed");
    }
    return graph;
}

void LLVMAotCompiler::CompileModule(WrappedModule &module)
{
    auto compilerOptions = InitializeLLVMCompilerOptions();
    module.GetDebugData()->Finalize();
    auto arkInterface = module.GetLLVMArkInterface().get();
    auto llvmIrModule = module.GetModule().get();

    auto &targetMachine = module.GetTargetMachine();
    LLVMOptimizer llvmOptimizer {compilerOptions, arkInterface, targetMachine};

    // Dumps require lock to be not messed up between different modules compiled in parallel.
    // NB! When ark is built with TSAN there are warnings coming from libLLVM.so.
    // They do not fire in tests because we do not run parallel compilation in tests.
    // We have to use suppression file, to run parallel compilation under tsan

    {  // Dump before optimization
        llvm::sys::ScopedLock scopedLock {lock_};
        llvmOptimizer.DumpModuleBefore(llvmIrModule);
    }

    // Optimize IR
    llvmOptimizer.OptimizeModule(llvmIrModule);

    {  // Dump after optimization
        llvm::sys::ScopedLock scopedLock {lock_};
        llvmOptimizer.DumpModuleAfter(llvmIrModule);
    }

    // clang-format off
    MIRCompiler mirCompiler {targetMachine, [&arkInterface](InsertingPassManager *manager) -> void {
                                manager->InsertBefore(&llvm::FEntryInserterID, CreateFrameLoweringPass(arkInterface));
                            }};
    // clang-format on

    // Create machine code
    auto file = exitOnErr_(mirCompiler.CompileModule(*llvmIrModule));
    if (file->HasSection(".llvm_stackmaps")) {
        auto section = file->GetSection(".llvm_stackmaps");
        module.GetCodeInfoProducer()->SetStackMap(section.GetMemory(), section.GetSize());

        auto stackmapInfo = file->GetStackMapInfo();

        for (auto method : module.GetMethods()) {
            auto fullMethodName = module.GetLLVMArkInterface()->GetUniqMethodName(method);
            if (stackmapInfo.find(fullMethodName) != stackmapInfo.end()) {
                module.GetCodeInfoProducer()->AddSymbol(static_cast<Method *>(method), stackmapInfo.at(fullMethodName));
            }
        }
    }
    if (file->HasSection(".llvm_faultmaps")) {
        auto section = file->GetSection(".llvm_faultmaps");
        module.GetCodeInfoProducer()->SetFaultMaps(section.GetMemory(), section.GetSize());

        auto faultMapInfo = file->GetFaultMapInfo();

        for (auto method : module.GetMethods()) {
            auto fullMethodName = module.GetLLVMArkInterface()->GetUniqMethodName(method);
            if (faultMapInfo.find(fullMethodName) != faultMapInfo.end()) {
                module.GetCodeInfoProducer()->AddFaultMapSymbol(static_cast<Method *>(method),
                                                                faultMapInfo.at(fullMethodName));
            }
        }
    }

    if (g_options.IsLlvmDumpObj()) {
        uint32_t moduleNumber = compiledModules_++;
        std::string fileName = "llvm-output-" + std::to_string(moduleNumber) + ".o";
        file->WriteTo(fileName);
    }
    module.SetCompiled(std::move(file));
}

void LLVMAotCompiler::DumpCodeInfo(compiler::CompiledMethod &method) const
{
    LLVM_LOG(DEBUG, INFRA) << "Dumping code info for method: " << runtime_->GetMethodFullName(method.GetMethod());
    std::stringstream ss;
    compiler::CodeInfo info;
    info.Decode(method.GetCodeInfo());
    info.Dump(ss);
    LLVM_LOG(DEBUG, INFRA) << ss.str();
}

}  // namespace ark::llvmbackend
