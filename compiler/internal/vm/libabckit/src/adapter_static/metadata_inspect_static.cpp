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

#include "helpers_static.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"
#include "assembler/annotation.h"
#include "libabckit/c/statuses.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_static/runtime_adapter_static.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "libabckit/src/adapter_static/ir_static.h"

#include "libabckit/src/wrappers/graph_wrapper/graph_wrapper.h"

#include "libarkbase/mem/arena_allocator.h"
#include "libarkfile/modifiers.h"
#include "src/logger.h"
#include "static_core/assembler/assembly-program.h"
#include "static_core/assembler/mangling.h"
#include "static_core/assembler/assembly-emitter.h"

#include "static_core/compiler/compiler_options.h"
#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir_builder/ir_builder.h"
#include "static_core/compiler/optimizer/optimizations/cleanup.h"
#include "static_core/bytecode_optimizer/check_resolver.h"

#include <cstdint>
#include <iostream>

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

namespace libabckit {

// ========================================
// Module
// ========================================

bool ModuleEnumerateAnonymousFunctionsStatic(AbckitCoreModule *m, void *data,
                                             bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)
    for (auto &function : m->functions) {
        if (!FunctionIsAnonymousStatic(function.get())) {
            continue;
        }
        if (!cb(function.get(), data)) {
            return false;
        }
    }
    return true;
}

// ========================================
// Namespace
// ========================================

AbckitString *NamespaceGetNameStatic(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;
    auto *record = ns->GetArkTSImpl()->impl.GetStaticClass();
    auto [_, namespaceName] = ClassGetNames(record->name);

    return CreateStringStatic(ns->owningModule->file, namespaceName.data(), namespaceName.size());
}

bool NamespaceIsExternalStatic(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;
    return ns->GetArkTSImpl()->impl.GetStaticClass()->metadata->IsForeign();
}

// ========================================
// Class
// ========================================

AbckitString *ClassGetNameStatic(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;
    auto *record = klass->GetArkTSImpl()->impl.GetStaticClass();
    auto [moduleName, className] = ClassGetNames(record->name);

    return CreateStringStatic(klass->owningModule->file, className.data(), className.size());
}

bool ClassIsExternalStatic(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;
    return klass->GetArkTSImpl()->impl.GetStaticClass()->metadata->IsForeign();
}

bool ClassIsFinalStatic(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;
    return (klass->GetArkTSImpl()->impl.GetStaticClass()->metadata->GetAccessFlags() & ACC_FINAL) != 0x0;
}

bool ClassIsAbstractStatic(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;
    return (klass->GetArkTSImpl()->impl.GetStaticClass()->metadata->GetAccessFlags() & ACC_ABSTRACT) != 0x0;
}

// ========================================
// Interface
// ========================================

AbckitString *InterfaceGetNameStatic(AbckitCoreInterface *iface)
{
    LIBABCKIT_LOG_FUNC;
    auto *record = iface->GetArkTSImpl()->impl.GetStaticClass();
    auto [_, interfaceName] = ClassGetNames(record->name);

    return CreateStringStatic(iface->owningModule->file, interfaceName.data(), interfaceName.size());
}

bool InterfaceIsExternalStatic(AbckitCoreInterface *iface)
{
    LIBABCKIT_LOG_FUNC;
    return iface->GetArkTSImpl()->impl.GetStaticClass()->metadata->IsForeign();
}

// ========================================
// Enum
// ========================================

AbckitString *EnumGetNameStatic(AbckitCoreEnum *enm)
{
    LIBABCKIT_LOG_FUNC;
    auto *record = enm->GetArkTSImpl()->impl.GetStaticClass();
    auto [_, enumName] = ClassGetNames(record->name);

    return CreateStringStatic(enm->owningModule->file, enumName.data(), enumName.size());
}

bool EnumIsExternalStatic(AbckitCoreEnum *enm)
{
    LIBABCKIT_LOG_FUNC;
    return enm->GetArkTSImpl()->impl.GetStaticClass()->metadata->IsForeign();
}

// ========================================
// Field
// ========================================

AbckitString *ModuleFieldGetNameStatic(AbckitCoreModuleField *field)
{
    LIBABCKIT_LOG_FUNC;
    auto fieldName = field->GetArkTSImpl()->GetStaticImpl()->name;
    return CreateStringStatic(field->owner->file, fieldName.data(), fieldName.size());
}

AbckitString *NamespaceFieldGetNameStatic(AbckitCoreNamespaceField *field)
{
    LIBABCKIT_LOG_FUNC;
    auto fieldName = field->GetArkTSImpl()->GetStaticImpl()->name;
    return CreateStringStatic(field->owner->owningModule->file, fieldName.data(), fieldName.size());
}

AbckitString *ClassFieldGetNameStatic(AbckitCoreClassField *field)
{
    LIBABCKIT_LOG_FUNC;
    auto fieldName = field->GetArkTSImpl()->GetStaticImpl()->name;
    return CreateStringStatic(field->owner->owningModule->file, fieldName.data(), fieldName.size());
}

AbckitString *EnumFieldGetNameStatic(AbckitCoreEnumField *field)
{
    LIBABCKIT_LOG_FUNC;
    auto fieldName = field->GetArkTSImpl()->GetStaticImpl()->name;
    return CreateStringStatic(field->owner->owningModule->file, fieldName.data(), fieldName.size());
}

bool ClassFieldIsPublicStatic(AbckitCoreClassField *field)
{
    LIBABCKIT_LOG_FUNC;
    return (field->GetArkTSImpl()->GetStaticImpl()->metadata->GetAccessFlags() & ACC_PUBLIC) != 0x0;
}

bool ClassFieldIsProtectedStatic(AbckitCoreClassField *field)
{
    LIBABCKIT_LOG_FUNC;
    return (field->GetArkTSImpl()->GetStaticImpl()->metadata->GetAccessFlags() & ACC_PROTECTED) != 0x0;
}

bool ClassFieldIsPrivateStatic(AbckitCoreClassField *field)
{
    LIBABCKIT_LOG_FUNC;
    return (field->GetArkTSImpl()->GetStaticImpl()->metadata->GetAccessFlags() & ACC_PRIVATE) != 0x0;
}

bool ClassFieldIsInternalStatic(AbckitCoreClassField *field)
{
    LIBABCKIT_LOG_FUNC;
    auto flag = field->GetArkTSImpl()->GetStaticImpl()->metadata->GetAccessFlags();
    return (flag & ACC_PUBLIC) == 0x0 && (flag & ACC_PROTECTED) == 0x0 && (flag & ACC_PRIVATE) == 0x0;
}

bool ClassFieldIsStaticStatic(AbckitCoreClassField *field)
{
    LIBABCKIT_LOG_FUNC;
    return (field->GetArkTSImpl()->GetStaticImpl()->metadata->GetAccessFlags() & ACC_STATIC) != 0x0;
}

bool ClassFieldIsFinalStatic(AbckitCoreClassField *field)
{
    LIBABCKIT_LOG_FUNC;
    return (field->GetArkTSImpl()->GetStaticImpl()->metadata->GetAccessFlags() & ACC_FINAL) != 0x0;
}

bool InterfaceFieldIsReadonlyStatic(AbckitCoreInterfaceField *field)
{
    LIBABCKIT_LOG_FUNC;
    return (field->flag & ACC_READONLY) != 0x0;
}

// ========================================
// Function
// ========================================
AbckitString *FunctionGetNameStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *functionImpl = function->GetArkTSImpl()->GetStaticImpl();

    auto functionName = pandasm::MangleFunctionName(functionImpl->name, functionImpl->params, functionImpl->returnType);
    auto croppedName = FuncNameCropModule(functionName);
    return CreateStringStatic(function->owningModule->file, croppedName.data(), croppedName.size());
}

bool GetMethodOffset(pandasm::Function *func, pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps,
                     uint32_t *methodOffset)
{
    auto funcFullName = pandasm::MangleFunctionName(func->name, func->params, func->returnType);
    for (auto &[id, s] : maps->methods) {
        if (s == funcFullName) {
            *methodOffset = id;
        }
    }
    if (*methodOffset == 0) {
        LIBABCKIT_LOG(DEBUG) << "methodOffset == 0\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    return true;
}

void RemoveInsts(compiler::Graph *graphImpl)
{
    graphImpl->RunPass<ark::bytecodeopt::CheckResolver>();
    graphImpl->RunPass<compiler::Cleanup>(false);
    std::vector<std::pair<compiler::BasicBlock *, compiler::Inst *>> instsToRemove;
    for (auto bb : graphImpl->GetBlocksRPO()) {
        for ([[maybe_unused]] auto inst : bb->AllInsts()) {
            if (inst->IsSaveState() || (inst->GetOpcode() == compiler::Opcode::SaveStateDeoptimize)) {
                ASSERT(!inst->HasUsers());
                /*
                 * NOTE(nsizov): find out why we still have non-movable
                 *               SaveStates with no users
                 */
                instsToRemove.emplace_back(bb, inst);
            }
        }
    }
    for (auto &[bb, inst] : instsToRemove) {
        bb->RemoveInst(inst);
    }
}

static void DeleteGraphArgs(ArenaAllocator *allocator, ArenaAllocator *localAllocator,
                            AbckitRuntimeAdapterStatic *adapter, AbckitIrInterface *irInterface)
{
    delete allocator;
    delete localAllocator;
    delete adapter;
    delete irInterface;
}

static AbckitGraph *CreateGraph(AbckitCoreFunction *function, AbckitIrInterface *irInterface,
                                ark::compiler::Graph *graphImpl, AbckitRuntimeAdapterStatic *adapter)
{
    auto graph = new AbckitGraph;
    graph->file = function->owningModule->file;
    graph->function = function;
    graph->irInterface = irInterface;
    graph->impl = graphImpl;
    GraphWrapper::CreateGraphWrappers(graph);

    auto *ctxGInternal =
        new CtxGInternal {graphImpl->GetAllocator(), graphImpl->GetLocalAllocator(), irInterface, adapter};
    graph->internal = ctxGInternal;
    LIBABCKIT_LOG_DUMP(GdumpStatic(graph, STDERR_FILENO), DEBUG);
    return graph;
}

AbckitGraph *CreateGraphFromFunctionStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;

    ark::compiler::g_options.SetCompilerFrameSize("default");
    ark::compiler::g_options.SetCompilerUseSafepoint(false);
    auto *func = function->GetArkTSImpl()->GetStaticImpl();

    LIBABCKIT_LOG_DUMP(func->DebugDump(), DEBUG);
    LIBABCKIT_LOG(DEBUG) << func->name << '\n';

    auto *file = function->owningModule->file;
    auto program = function->owningModule->file->GetStaticProgram();

    pandasm::AsmEmitter::PandaFileToPandaAsmMaps *maps;
    panda_file::File *pf = nullptr;
    if (function->owningModule->file->needOptimize && file->generateStatus.generated) {
        maps = static_cast<pandasm::AsmEmitter::PandaFileToPandaAsmMaps *>(file->generateStatus.maps);
        pf = static_cast<panda_file::File *>(file->generateStatus.pf);
    } else {
        if (file->generateStatus.pf != nullptr) {
            delete static_cast<panda_file::File *>(file->generateStatus.pf);
        }
        if (file->generateStatus.maps != nullptr) {
            delete static_cast<pandasm::AsmEmitter::PandaFileToPandaAsmMaps *>(file->generateStatus.maps);
        }

        maps = new pandasm::AsmEmitter::PandaFileToPandaAsmMaps();
        pf = const_cast<panda_file::File *>(pandasm::AsmEmitter::Emit(*program, maps).release());
    }

    if (pf == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "FAILURE: " << pandasm::AsmEmitter::GetLastError() << '\n';
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        if (!function->owningModule->file->needOptimize) {
            delete maps;
        }
        return nullptr;
    }

    if (function->owningModule->file->needOptimize) {
        file->generateStatus = {true, pf, maps};
    }

    uint32_t methodOffset = 0;
    if (!GetMethodOffset(func, maps, &methodOffset)) {
        if (!function->owningModule->file->needOptimize) {
            delete maps;
        }
        return nullptr;
    }

    auto *irInterface =
        new AbckitIrInterface(maps->methods, maps->fields, maps->classes, maps->strings, maps->literalarrays);

    auto *allocator = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
    auto *localAllocator = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER, nullptr, true);
    auto *adapter = new AbckitRuntimeAdapterStatic(*pf);

    auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(methodOffset);  // NOTE
    auto graphImpl = allocator->New<compiler::Graph>(
        compiler::Graph::GraphArgs {allocator, localAllocator, Arch::NONE, methodPtr, adapter}, nullptr, false, false,
        true);
    if (graphImpl == nullptr) {
        DeleteGraphArgs(allocator, localAllocator, adapter, irInterface);
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_MEMORY_ALLOCATION);
        if (!function->owningModule->file->needOptimize) {
            delete maps;
        }
        return nullptr;
    }
    graphImpl->SetLanguage(SourceLanguage::ETS);
    graphImpl->SetAbcKit();
#ifndef NDEBUG
    graphImpl->SetLowLevelInstructionsEnabled();
#endif

    bool irBuilderRes = graphImpl->RunPass<ark::compiler::IrBuilder>();
    if (!irBuilderRes) {
        DeleteGraphArgs(allocator, localAllocator, adapter, irInterface);
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        if (!function->owningModule->file->needOptimize) {
            delete maps;
        }
        return nullptr;
    }

    RemoveInsts(graphImpl);
    graphImpl->RemoveUnreachableBlocks();
    GraphInvalidateAnalyses(graphImpl);
    CheckInvalidOpcodes(graphImpl, false);

    AbckitGraph *graph = CreateGraph(function, irInterface, graphImpl, adapter);
    if (!function->owningModule->file->needOptimize) {
        delete maps;
    }
    return graph;
}

bool FunctionIsStaticStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_STATIC) != 0x0;
}

bool FunctionIsCtorStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    return function->GetArkTSImpl()->GetStaticImpl()->metadata->IsCtor();
}

bool FunctionIsCctorStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    return function->GetArkTSImpl()->GetStaticImpl()->metadata->IsCctor();
}

bool FunctionIsAnonymousStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    size_t pos = func->name.rfind('.');
    ASSERT(pos != std::string::npos);
    std::string name = func->name.substr(pos + 1);
    return name.find("lambda_invoke-") == 0;
}

bool FunctionIsNativeStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_NATIVE) != 0x0;
}

bool FunctionIsPublicStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_PUBLIC) != 0x0;
}

bool FunctionIsProtectedStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_PROTECTED) != 0x0;
}

bool FunctionIsPrivateStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_PRIVATE) != 0x0;
}

bool FunctionIsInternalStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto flag = function->GetArkTSImpl()->GetStaticImpl()->metadata->GetAccessFlags();
    return (flag & ACC_PUBLIC) == 0x0 && (flag & ACC_PROTECTED) == 0x0 && (flag & ACC_PRIVATE) == 0x0;
}

bool FunctionIsExternalStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    return function->GetArkTSImpl()->GetStaticImpl()->metadata->IsForeign();
}

bool FunctionIsAbstractStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_ABSTRACT) != 0x0;
}

bool FunctionIsFinalStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    return (func->metadata->GetAccessFlags() & ACC_FINAL) != 0x0;
}

bool FunctionIsAsyncStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    return function->asyncImpl != nullptr;
}

AbckitType *FunctionGetReturnTypeStatic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = function->GetArkTSImpl()->GetStaticImpl();
    auto returnType = func->returnType;
    auto file = function->owningModule->file;
    auto typeName = CreateStringStatic(file, returnType.GetName().data(), returnType.GetName().size());
    auto typeId = ArkPandasmTypeToAbckitTypeId(returnType);
    auto rank = returnType.GetRank();
    auto type = GetOrCreateType(file, typeId, rank, nullptr, typeName);
    if (returnType.IsUnion() && type->types.empty()) {
        auto typeNames = returnType.GetComponentNames();
        for (const auto &memberTypeName : typeNames) {
            auto memberName = CreateStringStatic(file, memberTypeName.data(), memberTypeName.size());
            type->types.emplace_back(GetOrCreateType(file, typeId, rank, nullptr, memberName));
        }
    }

    return type;
}

// ========================================
// Annotation
// ========================================

// ========================================
// AnnotationInterface
// ========================================

AbckitString *AnnotationInterfaceGetNameStatic(AbckitCoreAnnotationInterface *ai)
{
    LIBABCKIT_LOG_FUNC;
    auto *record = ai->GetArkTSImpl()->GetStaticImpl();
    auto [_, annotationInterfaceName] = ClassGetNames(record->name);
    return CreateStringStatic(ai->owningModule->file, annotationInterfaceName.data(), annotationInterfaceName.size());
}

// ========================================
// ImportDescriptor
// ========================================

// ========================================
// ExportDescriptor
// ========================================

// ========================================
// Literal
// ========================================

bool LiteralGetBoolStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsBoolValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return false;
    }
    return std::get<bool>(literal->value);
}

uint8_t LiteralGetU8Static(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsByteValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint8_t>(literal->value);
}

uint16_t LiteralGetU16Static(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsShortValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint16_t>(literal->value);
}

uint16_t LiteralGetMethodAffiliateStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (literal->tag != panda_file::LiteralTag::METHODAFFILIATE) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint16_t>(literal->value);
}

uint32_t LiteralGetU32Static(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsIntegerValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint32_t>(literal->value);
}

uint64_t LiteralGetU64Static(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsLongValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint64_t>(literal->value);
}

float LiteralGetFloatStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsFloatValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<float>(literal->value);
}

double LiteralGetDoubleStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsDoubleValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<double>(literal->value);
}

AbckitString *LiteralGetStringStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto s = std::make_unique<AbckitString>();
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsStringValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return nullptr;
    }

    auto &strings = lit->file->strings;

    auto &val = std::get<std::string>(literal->value);
    if (strings.find(val) != strings.end()) {
        return strings[val].get();
    }
    s->impl = val;
    strings.insert({val, std::move(s)});

    return strings[val].get();
}

AbckitLiteralTag LiteralGetTagStatic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *litImpl = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    switch (litImpl->tag) {
        case panda_file::LiteralTag::ARRAY_U1:
        case panda_file::LiteralTag::BOOL:
            return ABCKIT_LITERAL_TAG_BOOL;
        case panda_file::LiteralTag::ARRAY_U8:
        case panda_file::LiteralTag::ARRAY_I8:
            return ABCKIT_LITERAL_TAG_BYTE;
        case panda_file::LiteralTag::ARRAY_U16:
        case panda_file::LiteralTag::ARRAY_I16:
            return ABCKIT_LITERAL_TAG_SHORT;
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::INTEGER:
            return ABCKIT_LITERAL_TAG_INTEGER;
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_I64:
            return ABCKIT_LITERAL_TAG_LONG;
        case panda_file::LiteralTag::ARRAY_F32:
        case panda_file::LiteralTag::FLOAT:
            return ABCKIT_LITERAL_TAG_FLOAT;
        case panda_file::LiteralTag::ARRAY_F64:
        case panda_file::LiteralTag::DOUBLE:
            return ABCKIT_LITERAL_TAG_DOUBLE;
        case panda_file::LiteralTag::ARRAY_STRING:
        case panda_file::LiteralTag::STRING:
            return ABCKIT_LITERAL_TAG_STRING;
        case panda_file::LiteralTag::NULLVALUE:
            return ABCKIT_LITERAL_TAG_NULLVALUE;
        case panda_file::LiteralTag::TAGVALUE:
            return ABCKIT_LITERAL_TAG_TAGVALUE;
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
            return ABCKIT_LITERAL_TAG_METHOD;
        case panda_file::LiteralTag::METHODAFFILIATE:
            return ABCKIT_LITERAL_TAG_METHODAFFILIATE;
        default:
            return ABCKIT_LITERAL_TAG_INVALID;
    }
}

// ========================================
// Type
// ========================================

bool TypeIsUnionStatic(AbckitType *type)
{
    LIBABCKIT_LOG_FUNC;
    return !type->types.empty();
}

bool TypeEnumerateUnionTypesStatic(AbckitType *type, void *data, bool (*cb)(AbckitType *type, void *data))
{
    LIBABCKIT_LOG_FUNC;
    for (auto &t : type->types) {
        if (!cb(t, data)) {
            return false;
        }
    }
    return true;
}

// ========================================
// Value
// ========================================

AbckitType *ValueGetTypeStatic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    AbckitTypeId id;
    switch (pVal->GetType()) {
        case pandasm::Value::Type::U1:
            id = ABCKIT_TYPE_ID_U1;
            break;
        case pandasm::Value::Type::U8:
            id = ABCKIT_TYPE_ID_U8;
            break;
        case pandasm::Value::Type::U16:
            id = ABCKIT_TYPE_ID_U16;
            break;
        case pandasm::Value::Type::U32:
            id = ABCKIT_TYPE_ID_U32;
            break;
        case pandasm::Value::Type::I32:
            id = ABCKIT_TYPE_ID_I32;
            break;
        case pandasm::Value::Type::U64:
            id = ABCKIT_TYPE_ID_U64;
            break;
        case pandasm::Value::Type::F64:
            id = ABCKIT_TYPE_ID_F64;
            break;
        case pandasm::Value::Type::STRING:
            id = ABCKIT_TYPE_ID_STRING;
            break;
        default:
            LIBABCKIT_UNIMPLEMENTED;
    }
    // NOTE implement logic for classes
    return GetOrCreateType(value->file, id, 0, nullptr, nullptr);
}

bool ValueGetU1Static(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeStatic(value)->id != ABCKIT_TYPE_ID_U1) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return false;
    }

    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    return pVal->GetValue<bool>();
}

int ValueGetIntStatic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeStatic(value)->id != ABCKIT_TYPE_ID_I32) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return 0;
    }

    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    return pVal->GetValue<int>();
}

double ValueGetDoubleStatic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeStatic(value)->id != ABCKIT_TYPE_ID_F64) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return 0.0;
    }

    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    return pVal->GetValue<double>();
}

AbckitString *ValueGetStringStatic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeStatic(value)->id != ABCKIT_TYPE_ID_STRING) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    auto valImpl = pVal->GetValue<std::string>();
    return CreateStringStatic(value->file, valImpl.data(), valImpl.size());
}

AbckitLiteralArray *ArrayValueGetLiteralArrayStatic(AbckitValue * /*value*/)
{
    LIBABCKIT_LOG_FUNC;
    statuses::SetLastError(ABCKIT_STATUS_UNSUPPORTED);
    return nullptr;
}

}  // namespace libabckit
