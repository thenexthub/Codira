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

#include "compiler/code_info/code_info.h"
#include "compiler/optimizer/code_generator/relocations.h"
#include "libarkbase/events/events.h"
#include "optimizer/ir/graph.h"
#include "runtime/include/method.h"
#include "compiler_options.h"
#include "target_machine_builder.h"

#include "llvm_irtoc_compiler.h"
#include "llvm_logger.h"
#include "llvm_options.h"
#include "mir_compiler.h"

#include "lowering/llvm_ir_constructor.h"
#include "transforms/passes/check_tail_calls.h"
#include "transforms/passes/inline_ir/patch_return_handler_stack_adjustment.h"

#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/Passes.h>
// Suppress warning about forward Pass declaration defined in another namespace
#include <llvm/Pass.h>
#include <llvm/Support/FileSystem.h>

using ark::compiler::LLVMIrConstructor;

namespace ark::llvmbackend {

std::unique_ptr<IrtocCompilerInterface> CreateLLVMIrtocCompiler(ark::compiler::RuntimeInterface *runtime,
                                                                ark::ArenaAllocator *allocator, ark::Arch arch)
{
    return std::make_unique<LLVMIrtocCompiler>(runtime, allocator, arch, "irtoc_file_name.hack");
}

LLVMIrtocCompiler::LLVMIrtocCompiler(ark::compiler::RuntimeInterface *runtime, ark::ArenaAllocator *allocator,
                                     ark::Arch arch, std::string filename)
    : LLVMCompiler(arch),
      methods_(allocator->Adapter()),
      filename_(std::move(filename)),
      arkInterface_(runtime, GetTripleForArch(arch), nullptr, nullptr)
{
    InitializeSpecificLLVMOptions(arch);
    auto llvmCompilerOptions = InitializeLLVMCompilerOptions();

    // clang-format off
    targetMachine_ = cantFail(ark::llvmbackend::TargetMachineBuilder {}
                                .SetCPU(GetCPUForArch(arch))
                                .SetOptLevel(static_cast<llvm::CodeGenOpt::Level>(llvmCompilerOptions.optlevel))
                                .SetFeatures(GetFeaturesForArch(GetArch()))
                                .SetTriple(GetTripleForArch(GetArch()))
                                .Build());
    // clang-format on
    mirCompiler_ =
        std::make_unique<MIRCompiler>(targetMachine_, [this](ark::llvmbackend::InsertingPassManager *manager) -> void {
            manager->InsertBefore(&llvm::EarlyTailDuplicateID, ark::llvmbackend::CreateCheckTailCallsPass());
            manager->InsertBefore(&llvm::FEntryInserterID,
                                  ark::llvmbackend::CreatePatchReturnHandlerStackAdjustmentPass(&arkInterface_));
        });

    optimizer_ = std::make_unique<ark::llvmbackend::LLVMOptimizer>(llvmCompilerOptions, &arkInterface_,
                                                                   mirCompiler_->GetTargetMachine());
    InitializeModule();

    debugData_ = std::make_unique<DebugDataBuilder>(module_.get(), filename_);
}

std::vector<std::string> LLVMIrtocCompiler::GetFeaturesForArch(Arch arch)
{
    if (arch == Arch::X86_64 && ark::compiler::g_options.IsCpuFeatureEnabled(compiler::SSE42)) {
        return {std::string("+sse4.2")};
    }
    return {};
}

Expected<bool, std::string> LLVMIrtocCompiler::TryAddGraph(compiler::Graph *graph)
{
    ASSERT(graph != nullptr);
    ASSERT(graph->GetArch() == GetArch());
    ASSERT(!graph->SupportManagedCode());

    if (graph->IsDynamicMethod()) {
        return false;
    }

    std::string graphError = LLVMIrConstructor::CheckGraph(graph);
    if (!graphError.empty()) {
        return Unexpected(graphError);
    }

    LLVMIrConstructor ctor(graph, module_.get(), GetLLVMContext(), &arkInterface_, debugData_);
    auto llvmFunction = ctor.GetFunc();
    if (graph->GetMode().IsFastPath()) {
        llvmFunction->addFnAttr("target-features", GetFastPathFeatures());
    }

    bool noInline = IsInliningDisabled(graph);
    bool builtIr = ctor.BuildIr(noInline);
    if (!builtIr) {
        LLVM_LOG(ERROR, INFRA) << "Failed to build LLVM IR";
        llvmFunction->deleteBody();
        return Unexpected(std::string("Failed to build LLVM IR"));
    }

    LLVM_LOG(DEBUG, INFRA) << "LLVM built LLVM IR for method  " << arkInterface_.GetUniqMethodName(graph->GetMethod());
    methods_.push_back(static_cast<Method *>(graph->GetMethod()));
    return true;
}

void LLVMIrtocCompiler::FinishCompile()
{
    // Compile even if there are no methods because we have to produce an object file, even an empty one
    ASSERT_PRINT(!HasCompiledCode(), "Cannot compile twice");

    LLVM_LOG_IF(g_options.IsLlvmDumpObj(), FATAL, INFRA)
        << "Do not use '--llvm-dump-obj' in irtoc mode. Instead, look at the object file from '--irtoc-output-llvm' "
           "option value";

    optimizer_->DumpModuleBefore(module_.get());
    optimizer_->OptimizeModule(module_.get());
    optimizer_->DumpModuleAfter(module_.get());
    debugData_->Finalize();
    objectFile_ = exitOnErr_(mirCompiler_->CompileModule(*module_));
}

std::string LLVMIrtocCompiler::GetFastPathFeatures() const
{
    std::string features;
    for (const auto &feature : GetFeaturesForArch(GetArch())) {
        features.append(feature).append(",");
    }
    // FastPath may use FP register. We should be ready for this
    switch (GetArch()) {
        case Arch::AARCH64:
            features.append("+reserve-").append(arkInterface_.GetFramePointerRegister());
            features.append(",");
            features.append("+reserve-").append(arkInterface_.GetThreadRegister());
            break;
        default:
            UNREACHABLE();
    }
    return features;
}

void LLVMIrtocCompiler::InitializeSpecificLLVMOptions(Arch arch)
{
    if (arch == Arch::X86_64) {
        SetLLVMOption("x86-use-base-pointer", "false");
    }
    if (arch == Arch::AARCH64) {
        SetLLVMOption("aarch64-enable-ptr32", "true");
    }
    SetLLVMOption("inline-remark-attribute", "true");
    LLVMCompiler::InitializeLLVMOptions();
}

void LLVMIrtocCompiler::InitializeModule()
{
    const auto &moduleFile = llvmbackend::g_options.GetLlvmInlineModule();
    auto layout = targetMachine_->createDataLayout();
    if (moduleFile.empty()) {
        module_ = std::make_unique<llvm::Module>("irtoc empty module", *GetLLVMContext());
        module_->setDataLayout(layout);
        module_->setTargetTriple(GetTripleForArch(GetArch()).getTriple());
        return;
    }
    auto buffer = errorOrToExpected(llvm::MemoryBuffer::getFile(moduleFile));
    LLVM_LOG_IF(!buffer, FATAL, INFRA) << "Could not read inline module from file = '" << moduleFile << "', error: '"
                                       << toString(buffer.takeError()) << "'";

    auto contents = llvm::getBitcodeFileContents(*buffer.get());
    LLVM_LOG_IF(!contents, FATAL, INFRA) << "Could get bitcode file contents from file = '" << moduleFile
                                         << "', error: '" << toString(contents.takeError()) << "'";

    static constexpr int32_t EXPECTED_MODULES = 1;
    LLVM_LOG_IF(contents->Mods.size() != EXPECTED_MODULES, FATAL, INFRA)
        << "Inline module file '" << moduleFile << "' has unexpected number of modules = " << contents->Mods.size()
        << ", expected " << EXPECTED_MODULES;
    auto module = contents->Mods[0].parseModule(*GetLLVMContext());
    LLVM_LOG_IF(!module, FATAL, INFRA) << "Could not parse inline module from file '" << moduleFile << "', error: '"
                                       << toString(buffer.takeError()) << "'";
    module_ = std::move(*module);
    module_->setDataLayout(layout);
    optimizer_->ProcessInlineModule(module_.get());
}

void LLVMIrtocCompiler::WriteObjectFile(std::string_view output)
{
    ASSERT_PRINT(HasCompiledCode(), "Call FinishCompile first");
    objectFile_->WriteTo(output);
}

size_t LLVMIrtocCompiler::GetObjectFileSize()
{
    return objectFile_->Size();
}

CompiledCode LLVMIrtocCompiler::GetCompiledCode(std::string_view functionName)
{
    ASSERT(objectFile_ != nullptr);
    auto reference = objectFile_->GetSectionByFunctionName(std::string {functionName});
    CompiledCode code {};
    code.size = reference.GetSize();
    code.code = reference.GetMemory();
    return code;
}
}  // namespace ark::llvmbackend
