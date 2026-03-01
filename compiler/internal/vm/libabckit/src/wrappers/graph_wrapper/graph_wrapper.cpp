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

#include "libabckit/src/wrappers/graph_wrapper/graph_wrapper.h"
#include "libabckit/c/statuses.h"
#include "libabckit/src/wrappers/abcfile_wrapper.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/irbuilder_dynamic/ir_builder_dyn.h"
#include "libabckit/src/adapter_dynamic/runtime_adapter_dynamic.h"
#include "libabckit/src/adapter_dynamic/convert.h"
#include "libabckit/src/adapter_static/helpers_static.h"
#include "libabckit/src/codegen/codegen_dynamic.h"
#include "libabckit/src/codegen/ic_slot_allocator.h"
#include "libabckit/src/helpers_common.h"

#include "static_core/compiler/optimizer/analysis/dominators_tree.h"
#include "static_core/assembler/assembly-function.h"
#include "static_core/assembler/assembly-record.h"
#include "static_core/assembler/mangling.h"
#include "static_core/bytecode_optimizer/check_resolver.h"
#include "static_core/bytecode_optimizer/reg_encoder.h"
#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir/graph_checker.h"
#include "static_core/compiler/optimizer/ir/graph_cloner.h"
#include "static_core/compiler/optimizer/optimizations/move_constants.h"
#include "static_core/compiler/optimizer/optimizations/cleanup.h"
#include "static_core/compiler/optimizer/optimizations/try_catch_resolving.h"
#include "libarkbase/utils/arch.h"

#include <cstdint>
#include <iostream>

namespace libabckit {
// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

struct CtxGInternalDynamic {
    ArenaAllocator *allocator;
    ArenaAllocator *localAllocator;
    const AbckitIrInterface *irInterface;
    AbckitRuntimeAdapterDynamic *runtimeAdapter;
};

AbckitGraph *GraphWrapper::BuildGraphDynamic(FileWrapper *pf, AbckitIrInterface *irInterface, AbckitFile *file,
                                             uint32_t methodOffset)
{
    ark::compiler::g_options.SetCompilerUseSafepoint(false);
    ark::compiler::g_options.SetCompilerFrameSize("large");

    auto *allocator = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    auto *localAllocator = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
    auto *adapter = new AbckitRuntimeAdapterDynamic(*pf);

    auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(methodOffset);  // NOTE
    auto graphImpl = allocator->New<compiler::Graph>(
        compiler::Graph::GraphArgs {allocator, localAllocator, Arch::NONE, methodPtr, adapter}, nullptr, false, false,
        true);
    if (graphImpl == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "graph == nullptr\n";
        delete allocator;
        delete localAllocator;
        delete adapter;
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_MEMORY_ALLOCATION);
        return nullptr;
    }
    graphImpl->SetDynamicMethod();
    graphImpl->SetAbcKit();
    if (!graphImpl->RunPass<IrBuilderDynamic>()) {
        LIBABCKIT_LOG(DEBUG) << "IrBuilder failed!\n";
        // Input file with bad bytecode
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    if (GraphHasUnreachableBlocks(graphImpl)) {
        LIBABCKIT_LOG(DEBUG) << "Cannot build graph, there are unreachable blocks in graph\n";
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    graphImpl->RunPass<ark::bytecodeopt::CheckResolver>();
    graphImpl->RunPass<ark::compiler::Cleanup>();
    CheckInvalidOpcodes(graphImpl, true);

    auto graph = new AbckitGraph;
    graph->irInterface = irInterface;
    graph->impl = graphImpl;
    graph->file = file;
    CreateGraphWrappers(graph);

    auto *ctxGInternal = new CtxGInternalDynamic {allocator, localAllocator, irInterface, adapter};
    graph->internal = ctxGInternal;

    return graph;
}

// false positive
// NOLINTNEXTLINE(readability-non-const-parameter)
static bool RunPreliminaryPasses(ark::compiler::Graph *graphImpl, uint16_t *icSlotNumber)
{
    graphImpl->RemoveUnreachableBlocks();

    graphImpl->InvalidateAnalysis<compiler::LoopAnalyzer>();

    CheckInvalidOpcodes(graphImpl, true);

    if (!ark::compiler::GraphChecker(graphImpl).Check()) {
        LIBABCKIT_LOG(DEBUG) << ": Graph Verifier failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return false;
    }

    graphImpl->InvalidateAnalysis<compiler::DominatorsTree>();

    if (!graphImpl->RunPass<compiler::DominatorsTree>()) {
        LIBABCKIT_LOG(DEBUG) << ": ICDominatorsTree failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    if (!graphImpl->RunPass<ICSlotAllocator>(icSlotNumber)) {
        LIBABCKIT_LOG(DEBUG) << ": ICSlotAllocator failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    graphImpl->RunPass<compiler::MoveConstants>();

    if (!AllocateRegisters(graphImpl, CodeGenDynamic::RESERVED_REG)) {
        LIBABCKIT_LOG(DEBUG) << ": RegAllocGraphColoring failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    if (!graphImpl->RunPass<bytecodeopt::RegEncoder>()) {
        LIBABCKIT_LOG(DEBUG) << ": RegEncoder failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    return true;
}

void *GraphWrapper::BuildCodeDynamic(AbckitGraph *graph, const std::string &funcName)
{
    LIBABCKIT_LOG(DEBUG) << "Building code for function " << funcName << "\n";

    auto graphImpl =
        compiler::GraphCloner(graph->impl, graph->impl->GetAllocator(), graph->impl->GetLocalAllocator()).CloneGraph();

    graphImpl->RemoveUnreachableBlocks();
    GraphInvalidateAnalyses(graphImpl);

    LIBABCKIT_LOG(DEBUG) << "BEFORE======================================\n";
    LIBABCKIT_LOG_DUMP(graphImpl->Dump(&std::cerr), DEBUG);
    LIBABCKIT_LOG(DEBUG) << "============================================\n";

    uint16_t icSlotNumber = 0;
    if (!RunPreliminaryPasses(graphImpl, &icSlotNumber)) {
        return nullptr;
    }

    LIBABCKIT_LOG(DEBUG) << "AFTER=======================================\n";
    LIBABCKIT_LOG_DUMP(graphImpl->Dump(&std::cerr), DEBUG);
    LIBABCKIT_LOG(DEBUG) << "============================================\n";

    const auto lang = convert::ToSourceLang(graph->function->owningModule->target);
    FunctionWrapper *wrFunc = PandasmWrapper::CreateWrappedFunction(lang);
    if (!graphImpl->RunPass<CodeGenDynamic>(wrFunc, graph->irInterface)) {
        LIBABCKIT_LOG(DEBUG) << funcName << ": Code generation failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    wrFunc->valueOfFirstParam =
        static_cast<int64_t>(graphImpl->GetStackSlotsCount()) - 1L;  // Work-around promotion rules
    wrFunc->regsNum = static_cast<size_t>(wrFunc->valueOfFirstParam + 1U);
    wrFunc->slotsNum = static_cast<size_t>(icSlotNumber);

    return PandasmWrapper::GetPandasmFunction(wrFunc);
}

void GraphWrapper::CreateGraphWrappers(AbckitGraph *graph)
{
    auto *graphImpl = graph->impl;

    for (auto *bbImpl : graphImpl->GetBlocksRPO()) {
        auto *bb = graphImpl->GetLocalAllocator()->New<AbckitBasicBlock>();
        bb->graph = graph;
        bb->impl = bbImpl;
        graph->implToBB.insert({bbImpl, bb});
        for (auto *instImpl : bbImpl->AllInsts()) {
            CreateInstFromImpl(graph, instImpl);
        }
    }

    auto *file = graph->file;

    // NOTE(ivagin)
    // Valid only for static, because in dynamic mode
    // classes are contained as functions
    if (file->frontend == Mode::STATIC) {
        // Find all class names and abckit classes
        std::unordered_map<std::string, AbckitCoreClass *> namesToClasses;
        for (auto &[_, module] : file->localModules) {
            for (auto &[_, klass] : module->ct) {
                auto *func = klass->GetArkTSImpl()->impl.GetStaticClass();
                namesToClasses[func->name] = klass.get();
            }
        }
        // Fill map offset to AbckitCoreClass for this graph context
        for (auto &[offset, str] : graph->irInterface->classes) {
            auto it = namesToClasses.find(pandasm::DeMangleName(str));
            if (it != namesToClasses.end()) {
                graph->ptrToClass[offset] = it->second;
            }
        }
    }
}

void GraphWrapper::DestroyGraphDynamic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    auto *ctxGInternal = reinterpret_cast<CtxGInternalDynamic *>(graph->internal);
    // dirty hack to obtain FileWrapper pointer
    // NOTE(mshimenkov): refactor it
    if (!graph->file->needOptimize) {
        auto *fileWrapper =
            reinterpret_cast<FileWrapper *>(ctxGInternal->runtimeAdapter->GetBinaryFileForMethod(nullptr));
        delete fileWrapper;
    }
    delete ctxGInternal->runtimeAdapter;
    delete ctxGInternal->irInterface;
    delete ctxGInternal->localAllocator;
    delete ctxGInternal->allocator;
    delete ctxGInternal;
    delete graph;
}

}  // namespace libabckit
