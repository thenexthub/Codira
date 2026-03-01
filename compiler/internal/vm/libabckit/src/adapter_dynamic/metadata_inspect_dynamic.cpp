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

#include "libabckit/c/statuses.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/adapter_dynamic/metadata_inspect_dynamic.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/adapter_dynamic/helpers_dynamic.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/statuses_impl.h"
#include "libabckit/src/wrappers/graph_wrapper/graph_wrapper.h"
#include "libabckit/src/wrappers/abcfile_wrapper.h"
#include "libabckit/src/adapter_static/ir_static.h"

#include "assembler/assembly-emitter.h"
#include "assembler/annotation.h"
#include "assembler/assembly-literals.h"
#include "assembler/assembly-program.h"

#include <cstddef>
#include <cstring>
#include <string>

namespace libabckit {

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda;

// ========================================
// Module
// ========================================

bool ModuleEnumerateAnonymousFunctionsDynamic(AbckitCoreModule *m, void *data,
                                              bool (*cb)(AbckitCoreFunction *function, void *data))
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, false)
    LIBABCKIT_BAD_ARGUMENT(cb, false)

    for (auto &function : m->functions) {
        if (!FunctionIsAnonymousDynamic(function.get())) {
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

AbckitString *NamespaceGetNameDynamic(AbckitCoreNamespace *n)
{
    LIBABCKIT_LOG_FUNC;
    ASSERT(n->owningModule->target == ABCKIT_TARGET_ARK_TS_V1);
    auto func = GetDynFunction(n->GetArkTSImpl()->f.get());
    auto name = func->name;
    size_t sharpPos = name.rfind('#');
    ASSERT(sharpPos != std::string::npos);
    auto subname = name.substr(sharpPos + 1);
    return CreateStringDynamic(n->owningModule->file, subname.data(), subname.size());
}

// ========================================
// Class
// ========================================

AbckitString *ClassGetNameDynamic(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;

    auto func = GetDynFunction(klass);
    auto mPayload = GetDynModulePayload(klass->owningModule);

    auto *scopesLitArr = mPayload->scopeNamesLiteralArray;
    auto name = GetClassNameFromCtor(func->name, scopesLitArr);

    return CreateStringDynamic(klass->owningModule->file, name.data(), name.size());
}

// ========================================
// Function
// ========================================

AbckitString *FunctionGetNameDynamic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *functionImpl = GetDynFunction(function);
    auto name = functionImpl->name;

    size_t sharpPos = name.rfind('#');
    if (!function->isAnonymous) {
        if (sharpPos != std::string::npos) {
            name = name.substr(sharpPos + 1);
        } else {
            name = name.substr(name.rfind('.') + 1);
            ASSERT(name == "func_main_0");
        }
    }

    return CreateStringDynamic(function->owningModule->file, name.data(), name.size());
}

AbckitGraph *CreateGraphFromFunctionDynamic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;

    auto *func = GetDynFunction(function);
    LIBABCKIT_LOG(DEBUG) << func->name << '\n';
    LIBABCKIT_LOG_DUMP(func->DebugDump(), DEBUG);

    auto *file = function->owningModule->file;
    auto program = function->owningModule->file->GetDynamicProgram();

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
        pf = const_cast<panda_file::File *>(EmitDynamicProgram(file, program, maps, true));
    }

    if (pf == nullptr) {
        return nullptr;
    }

    if (function->owningModule->file->needOptimize) {
        file->generateStatus = {true, pf, maps};
    }

    uint32_t functionOffset = 0;
    for (auto &[id, s] : maps->methods) {
        if (s == func->name) {
            functionOffset = id;
        }
    }

    if (functionOffset == 0) {
        LIBABCKIT_LOG(DEBUG) << "functionOffset == 0\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        if (!function->owningModule->file->needOptimize) {
            delete maps;
        }
        return nullptr;
    }

    auto *irInterface =
        new AbckitIrInterface(maps->methods, maps->fields, maps->classes, maps->strings, maps->literalarrays);

    auto *wpf = new FileWrapper(reinterpret_cast<const void *>(pf));
    auto graph = GraphWrapper::BuildGraphDynamic(wpf, irInterface, file, functionOffset);
    if (!function->owningModule->file->needOptimize) {
        delete maps;
    }
    if (statuses::GetLastError() != AbckitStatus::ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    LIBABCKIT_LOG_DUMP(GdumpStatic(graph, STDERR_FILENO), DEBUG);

    ASSERT(graph->file == file);
    graph->function = function;

    return graph;
}

bool FunctionIsStaticDynamic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = GetDynFunction(function);
    return IsStatic(func->name);
}

bool FunctionIsCtorDynamic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = GetDynFunction(function);
    return IsCtor(func->name);
}

bool FunctionIsAnonymousDynamic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    return function->isAnonymous;
}

bool FunctionIsNativeDynamic(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    auto *func = GetDynFunction(function);
    return (func->metadata->GetAccessFlags() & ACC_NATIVE) != 0x0;
}

// ========================================
// Annotation
// ========================================

AbckitString *AnnotationInterfaceGetNameDynamic(AbckitCoreAnnotationInterface *ai)
{
    LIBABCKIT_LOG_FUNC;
    auto name = pandasm::GetItemName(ai->GetArkTSImpl()->GetDynamicImpl()->name);
    return CreateStringDynamic(ai->owningModule->file, name.data(), name.size());
}

// ========================================
// ImportDescriptor
// ========================================

AbckitString *ImportDescriptorGetNameDynamic(AbckitCoreImportDescriptor *i)
{
    std::string name = [i]() {
        auto mPayload = GetDynModulePayload(i->importingModule);
        auto idPayload = GetDynImportDescriptorPayload(i);
        const auto *moduleLitArr = mPayload->moduleLiteralArray;
        auto sectionOffset =
            idPayload->isRegularImport ? mPayload->regularImportsOffset : mPayload->namespaceImportsOffset;
        if (!idPayload->isRegularImport) {
            return std::string("*");
        }
        auto importNameOffset = sectionOffset + idPayload->moduleRecordIndexOff * 3 + 1;
        return std::get<std::string>(moduleLitArr->GetDynamicImpl()->literals_[importNameOffset].value_);
    }();
    return CreateStringDynamic(i->importingModule->file, name.data(), name.size());
}

AbckitString *ImportDescriptorGetAliasDynamic(AbckitCoreImportDescriptor *i)
{
    std::string name = [i]() {
        auto mPayload = GetDynModulePayload(i->importingModule);
        auto idPayload = GetDynImportDescriptorPayload(i);
        const auto *moduleLitArr = mPayload->moduleLiteralArray;
        auto sectionOffset =
            idPayload->isRegularImport ? mPayload->regularImportsOffset : mPayload->namespaceImportsOffset;
        auto gap = idPayload->isRegularImport ? 3 : 2;
        auto importNameOffset = sectionOffset + idPayload->moduleRecordIndexOff * gap;
        return std::get<std::string>(moduleLitArr->GetDynamicImpl()->literals_[importNameOffset].value_);
    }();
    return CreateStringDynamic(i->importingModule->file, name.data(), name.size());
}

// ========================================
// ExportDescriptor
// ========================================

AbckitString *ExportDescriptorGetNameDynamic(AbckitCoreExportDescriptor *i)
{
    std::string name = [i]() {
        auto mPayload = GetDynModulePayload(i->exportingModule);
        auto edPayload = GetDynExportDescriptorPayload(i);
        const auto *moduleLitArr = mPayload->moduleLiteralArray;
        size_t exportNameOffset;
        switch (edPayload->kind) {
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT: {
                exportNameOffset = mPayload->localExportsOffset + edPayload->moduleRecordIndexOff * 2 + 1;
                break;
            }
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT: {
                exportNameOffset = mPayload->indirectExportsOffset + edPayload->moduleRecordIndexOff * 3 + 1;
                break;
            }
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT: {
                std::string res = "*";
                return res;
            }
        }
        return std::get<std::string>(moduleLitArr->GetDynamicImpl()->literals_[exportNameOffset].value_);
    }();
    return CreateStringDynamic(i->exportingModule->file, name.data(), name.size());
}

AbckitString *ExportDescriptorGetAliasDynamic(AbckitCoreExportDescriptor *i)
{
    std::string name = [i]() {
        auto mPayload = GetDynModulePayload(i->exportingModule);
        auto edPayload = GetDynExportDescriptorPayload(i);
        const auto *moduleLitArr = mPayload->moduleLiteralArray;
        size_t exportNameOffset;
        switch (edPayload->kind) {
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT: {
                exportNameOffset = mPayload->localExportsOffset + edPayload->moduleRecordIndexOff * 2;
                break;
            }
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT: {
                exportNameOffset = mPayload->indirectExportsOffset + edPayload->moduleRecordIndexOff * 3;
                break;
            }
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT: {
                if (!edPayload->hasServiceImport) {
                    std::string res;
                    return res;
                }
                exportNameOffset = mPayload->localExportsOffset + edPayload->moduleRecordIndexOff * 2 + 1;
            }
        }
        return std::get<std::string>(moduleLitArr->GetDynamicImpl()->literals_[exportNameOffset].value_);
    }();
    return CreateStringDynamic(i->exportingModule->file, name.data(), name.size());
}

// ========================================
// Literal
// ========================================

bool LiteralGetBoolDynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsBoolValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return false;
    }
    return std::get<bool>(literal->value_);
}

uint8_t LiteralGetU8Dynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsByteValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint8_t>(literal->value_);
}

uint16_t LiteralGetU16Dynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsShortValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint16_t>(literal->value_);
}

uint16_t LiteralGetMethodAffiliateDynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (literal->tag_ != panda_file::LiteralTag::METHODAFFILIATE) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint16_t>(literal->value_);
}

uint32_t LiteralGetU32Dynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsIntegerValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint32_t>(literal->value_);
}

uint64_t LiteralGetU64Dynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsLongValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<uint64_t>(literal->value_);
}

float LiteralGetFloatDynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsFloatValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<float>(literal->value_);
}

double LiteralGetDoubleDynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsDoubleValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return 0;
    }
    return std::get<double>(literal->value_);
}

AbckitLiteralArray *LiteralGetLiteralArrayDynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (literal->tag_ != panda::panda_file::LiteralTag::LITERALARRAY) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return nullptr;
    }

    auto &litarrs = lit->file->GetDynamicProgram()->literalarray_table;
    auto &arrName = std::get<std::string>(literal->value_);

    // Search through already created abckit litarrs
    for (auto &item : lit->file->litarrs) {
        if (item->GetDynamicImpl() == &litarrs[arrName]) {
            return item.get();
        }
    }

    // Create new abckit litarrs
    auto litarr = std::make_unique<AbckitLiteralArray>(lit->file, &litarrs[arrName]);
    lit->file->litarrs.emplace_back(std::move(litarr));
    return lit->file->litarrs[lit->file->litarrs.size() - 1].get();
}

AbckitString *LiteralGetStringDynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto s = std::make_unique<AbckitString>();
    auto *literal = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    if (!literal->IsStringValue()) {
        statuses::SetLastError(ABCKIT_STATUS_WRONG_LITERAL_TYPE);
        return nullptr;
    }

    auto &strings = lit->file->strings;

    auto &val = std::get<std::string>(literal->value_);
    if (strings.find(val) != strings.end()) {
        return strings[val].get();
    }
    s->impl = val;
    strings.insert({val, std::move(s)});

    return strings[val].get();
}

AbckitLiteralTag LiteralGetTagDynamic(AbckitLiteral *lit)
{
    LIBABCKIT_LOG_FUNC;
    auto *litImpl = reinterpret_cast<pandasm::LiteralArray::Literal *>(lit->val.get());
    switch (litImpl->tag_) {
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
        case panda_file::LiteralTag::GETTER:
        case panda_file::LiteralTag::SETTER:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
            return ABCKIT_LITERAL_TAG_METHOD;
        case panda_file::LiteralTag::METHODAFFILIATE:
            return ABCKIT_LITERAL_TAG_METHODAFFILIATE;
        case panda_file::LiteralTag::LITERALARRAY:
            return ABCKIT_LITERAL_TAG_LITERALARRAY;
        default:
            return ABCKIT_LITERAL_TAG_INVALID;
    }
}

bool LiteralArrayEnumerateElementsDynamic(AbckitLiteralArray *litArr, void *data,
                                          bool (*cb)(AbckitFile *file, AbckitLiteral *lit, void *data))
{
    LIBABCKIT_LOG_FUNC;
    auto *arrImpl = litArr->GetDynamicImpl();

    size_t size = arrImpl->literals_.size();
    if (size % 2U != 0) {
        statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    for (size_t idx = 1; idx < size; idx += 2U) {
        auto litImpl = arrImpl->literals_[idx];
        if (!cb(litArr->file, FindOrCreateLiteralDynamic(litArr->file, litImpl), data)) {
            return false;
        }
    }
    return true;
}

// ========================================
// Value
// ========================================

AbckitType *ValueGetTypeDynamic(AbckitValue *value)
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
        case pandasm::Value::Type::U64:
            id = ABCKIT_TYPE_ID_U64;
            break;
        case pandasm::Value::Type::F64:
            id = ABCKIT_TYPE_ID_F64;
            break;
        case pandasm::Value::Type::STRING:
            id = ABCKIT_TYPE_ID_STRING;
            break;
        case pandasm::Value::Type::LITERALARRAY:
            id = ABCKIT_TYPE_ID_LITERALARRAY;
            break;
        default:
            LIBABCKIT_UNIMPLEMENTED;
    }
    // NOTE implement logic for classes
    return GetOrCreateType(value->file, id, 0, nullptr, nullptr);
}

bool ValueGetU1Dynamic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeDynamic(value)->id != ABCKIT_TYPE_ID_U1) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return false;
    }

    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    return pVal->GetValue<bool>();
}

double ValueGetDoubleDynamic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeDynamic(value)->id != ABCKIT_TYPE_ID_F64) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return 0.0;
    }

    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    return pVal->GetValue<double>();
}

AbckitString *ValueGetStringDynamic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeDynamic(value)->id != ABCKIT_TYPE_ID_STRING) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    auto valImpl = pVal->GetValue<std::string>();
    return CreateStringDynamic(value->file, valImpl.data(), valImpl.size());
}

AbckitLiteralArray *ArrayValueGetLiteralArrayDynamic(AbckitValue *value)
{
    LIBABCKIT_LOG_FUNC;
    if (ValueGetTypeDynamic(value)->id != ABCKIT_TYPE_ID_LITERALARRAY) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto *pVal = reinterpret_cast<pandasm::ScalarValue *>(value->val.get());
    auto arrName = pVal->GetValue<std::string>();
    auto &litarrs = value->file->GetDynamicProgram()->literalarray_table;

    // Search through already created abckit litarrs
    for (auto &item : value->file->litarrs) {
        if (item->GetDynamicImpl() == &litarrs[arrName]) {
            return item.get();
        }
    }

    // Create new abckit litarr
    auto litarr = std::make_unique<AbckitLiteralArray>(value->file, &(litarrs[arrName]));
    return value->file->litarrs.emplace_back(std::move(litarr)).get();
}

}  // namespace libabckit
