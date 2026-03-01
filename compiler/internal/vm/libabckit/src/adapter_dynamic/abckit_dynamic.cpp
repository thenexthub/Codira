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

#include "libabckit/c/metadata_core.h"
#include "libabckit/src/adapter_dynamic/abckit_dynamic.h"
#include "libabckit/src/adapter_dynamic/convert.h"
#include "libabckit/src/adapter_dynamic/helpers_dynamic.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/statuses_impl.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/wrappers/graph_wrapper/graph_wrapper.h"
#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"

#include "abc2program/abc2program_driver.h"
#include "assembler/annotation.h"
#include "assembler/assembly-program.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-record.h"
#include "assembler/assembly-emitter.h"
#include "libpandabase/os/filesystem.h"
#include "libpandafile/file.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
#define STD_FILESYSTEM_EXPERIMENTAL
namespace fs = std::experimental::filesystem;
#endif

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda;
// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace libabckit;

namespace {

AbckitTypeId PandaTypeToAbcKitTypeId(const pandasm::Type &type)
{
    switch (type.GetId()) {
        case panda_file::Type::TypeId::VOID:
            return ABCKIT_TYPE_ID_VOID;
        case panda_file::Type::TypeId::U1:
            return ABCKIT_TYPE_ID_U1;
        case panda_file::Type::TypeId::I8:
            return ABCKIT_TYPE_ID_I8;
        case panda_file::Type::TypeId::U8:
            return ABCKIT_TYPE_ID_U8;
        case panda_file::Type::TypeId::I16:
            return ABCKIT_TYPE_ID_I16;
        case panda_file::Type::TypeId::U16:
            return ABCKIT_TYPE_ID_U16;
        case panda_file::Type::TypeId::I32:
            return ABCKIT_TYPE_ID_I32;
        case panda_file::Type::TypeId::U32:
            return ABCKIT_TYPE_ID_U32;
        case panda_file::Type::TypeId::F32:
            return ABCKIT_TYPE_ID_F32;
        case panda_file::Type::TypeId::F64:
            return ABCKIT_TYPE_ID_F64;
        case panda_file::Type::TypeId::I64:
            return ABCKIT_TYPE_ID_I64;
        case panda_file::Type::TypeId::U64:
            return ABCKIT_TYPE_ID_U64;
        case panda_file::Type::TypeId::REFERENCE:
            if (type.IsArray()) {
                return PandaTypeToAbcKitTypeId(type.GetComponentType());
            }
            return ABCKIT_TYPE_ID_REFERENCE;
        default:
            return AbckitTypeId::ABCKIT_TYPE_ID_INVALID;
    }
}

AbckitString *CreateNameString(std::string &name, AbckitFile *file)
{
    return CreateStringDynamic(file, name.data(), name.size());
}

AbckitCoreModule *TryFindModule(const std::string &name, AbckitFile *file)
{
    auto tryFindLocal = file->localModules.find(name);
    if (tryFindLocal != file->localModules.end()) {
        return tryFindLocal->second.get();
    }
    auto tryFindExternal = file->externalModules.find(name);
    if (tryFindExternal != file->externalModules.end()) {
        return tryFindExternal->second.get();
    }
    return nullptr;
}

std::string GetModuleRequestName(uint16_t moduleRequestsOffset, const pandasm::LiteralArray *litArr)
{
    ASSERT(litArr->literals_[moduleRequestsOffset + 1].IsStringValue());
    std::string name = std::get<std::string>(litArr->literals_[moduleRequestsOffset + 1].value_);
    if (name.substr(0, 2U) == "./") {
        std::string newName = name.substr(2U);
        return newName;
    }
    return name;
}

bool FillExportDescriptor(ModuleIterateData *data, uint16_t idx, size_t recordIndexOff, bool hasServiceImport = false,
                          size_t serviceNamespaceImportIdx = 0)
{
    auto ed = std::make_unique<AbckitCoreExportDescriptor>();
    switch (data->m->target) {
        case ABCKIT_TARGET_JS:
            ed->impl = std::make_unique<AbckitJsExportDescriptor>();
            ed->GetJsImpl()->core = ed.get();
            ed->GetJsImpl()->kind = AbckitImportExportDescriptorKind::UNTYPED;
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            ed->impl = std::make_unique<AbckitArktsExportDescriptor>();
            ed->GetArkTSImpl()->core = ed.get();
            ed->GetArkTSImpl()->kind = AbckitImportExportDescriptorKind::UNTYPED;
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
    ed->exportingModule = data->m;
    if (data->kind == ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT) {
        ed->exportedModule = data->m;
    } else {
        auto moduleRequestIdx = std::get<uint16_t>(data->moduleLitArr->literals_[idx].value_);
        auto exportedModuleName = GetModuleRequestName(moduleRequestIdx, data->moduleLitArr);
        auto tryFindModule = TryFindModule(exportedModuleName, data->m->file);
        ed->exportedModule = tryFindModule;
    }
    auto payloadDyn = AbckitDynamicExportDescriptorPayload();
    payloadDyn.kind = data->kind;
    payloadDyn.hasServiceImport = hasServiceImport;
    payloadDyn.serviceNamespaceImportIdx = serviceNamespaceImportIdx;
    payloadDyn.moduleRecordIndexOff = recordIndexOff;
    auto payload = AbckitCoreExportDescriptorPayload();
    payload.impl = payloadDyn;
    switch (data->m->target) {
        case ABCKIT_TARGET_JS:
            ed->GetJsImpl()->payload = payload;
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            ed->GetArkTSImpl()->payload = payload;
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
    data->m->ed.push_back(std::move(ed));
    return true;
}

void SetIdImpl(ModuleIterateData *data, std::unique_ptr<AbckitCoreImportDescriptor> &id)
{
    switch (data->m->target) {
        case ABCKIT_TARGET_JS:
            id->impl = std::make_unique<AbckitJsImportDescriptor>();
            id->GetJsImpl()->core = id.get();
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            id->impl = std::make_unique<AbckitArktsImportDescriptor>();
            id->GetArkTSImpl()->core = id.get();
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

void SetIdModule(ModuleIterateData *data, std::unique_ptr<AbckitCoreImportDescriptor> &id, uint16_t moduleRequestIdx)
{
    id->importingModule = data->m;
    auto importedModuleName = GetModuleRequestName(moduleRequestIdx, data->moduleLitArr);
    id->importedModule = TryFindModule(importedModuleName, data->m->file);
    switch (data->m->target) {
        case ABCKIT_TARGET_JS:
            id->GetJsImpl()->kind = AbckitImportExportDescriptorKind::UNTYPED;
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            id->GetArkTSImpl()->kind = AbckitImportExportDescriptorKind::UNTYPED;
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

bool SetRegularImport(ModuleIterateData *data, size_t *idx, size_t &fieldNum, size_t *importRecordIndexOff,
                      size_t *recordIndexOff, bool *skipFilling)
{
    for (size_t i = 0; i < (fieldNum - 1); i++) {
        auto name = std::get<std::string>(data->moduleLitArr->literals_[(*idx)++].value_);
        if ((i == 0) && !(data->isRegularImport.second) && (name.find("=ens") != std::string::npos)) {
            auto tmp = data->isRegularImport.second;
            data->isRegularImport.second = false;
            data->kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT;
            if (!FillExportDescriptor(data, (*idx)++, (*importRecordIndexOff)++, true, *recordIndexOff)) {
                return true;
            }
            data->isRegularImport.second = tmp;
            *skipFilling = true;
            (*recordIndexOff)++;
            break;
        }
    }
    return false;
}

std::pair<size_t, size_t> FillImportSection(ModuleIterateData *data, size_t idx, size_t fieldNum)
{
    size_t recordIndexOff = 0;
    size_t importRecordIndexOff = 0;
    size_t numImports = std::get<uint32_t>(data->moduleLitArr->literals_[idx++].value_);
    size_t sectionOffset = idx;
    bool skipFilling = false;
    while (idx < sectionOffset + numImports * fieldNum) {
        if (SetRegularImport(data, &idx, fieldNum, &importRecordIndexOff, &recordIndexOff, &skipFilling)) {
            return {0, 0};
        }
        if (skipFilling) {
            skipFilling = false;
            continue;
        }
        auto id = std::make_unique<AbckitCoreImportDescriptor>();
        SetIdImpl(data, id);
        auto moduleRequestIdx = std::get<uint16_t>(data->moduleLitArr->literals_[idx++].value_);
        SetIdModule(data, id, moduleRequestIdx);

        auto payloadDyn = AbckitDynamicImportDescriptorPayload();
        payloadDyn.isRegularImport = data->isRegularImport.second;
        payloadDyn.moduleRecordIndexOff = recordIndexOff++;
        auto payload = AbckitCoreImportDescriptorPayload();
        payload.impl = payloadDyn;
        switch (data->m->target) {
            case ABCKIT_TARGET_JS:
                id->GetJsImpl()->payload = payload;
                break;
            case ABCKIT_TARGET_ARK_TS_V1:
                id->GetArkTSImpl()->payload = payload;
                break;
            default:
                LIBABCKIT_UNREACHABLE;
        }
        data->m->id.push_back(std::move(id));
    }

    return {idx, sectionOffset};
}

bool FindJSTarget(const std::unique_ptr<AbckitCoreExportDescriptor> &ed, ModuleIterateData *data,
                  const std::string &serviceName)
{
    if (ed->GetJsImpl()->payload.GetDynamicPayload().hasServiceImport) {
        auto offset = data->payload->namespaceImportsOffset +
                      ed->GetJsImpl()->payload.GetDynamicPayload().serviceNamespaceImportIdx * 2;
        auto namespaceLocalName = std::get<std::string>(data->moduleLitArr->literals_[offset].value_);
        if (serviceName == namespaceLocalName) {
            return true;
        }
    }
    return false;
}

bool FindTSTarget(const std::unique_ptr<AbckitCoreExportDescriptor> &ed, ModuleIterateData *data,
                  const std::string &serviceName)
{
    if (ed->GetArkTSImpl()->payload.GetDynamicPayload().hasServiceImport) {
        auto offset = data->payload->namespaceImportsOffset +
                      ed->GetArkTSImpl()->payload.GetDynamicPayload().serviceNamespaceImportIdx * 2;
        auto namespaceLocalName = std::get<std::string>(data->moduleLitArr->literals_[offset].value_);
        if (serviceName == namespaceLocalName) {
            return true;
        }
    }
    return false;
}

AbckitCoreExportDescriptor *FindStarExport(ModuleIterateData *data, const std::string &serviceName)
{
    for (const auto &ed : data->m->ed) {
        switch (ed->exportingModule->target) {
            case ABCKIT_TARGET_JS:
                if (FindJSTarget(ed, data, serviceName)) {
                    return ed.get();
                }
                break;
            case ABCKIT_TARGET_ARK_TS_V1:
                if (FindTSTarget(ed, data, serviceName)) {
                    return ed.get();
                }
                break;
            default:
                LIBABCKIT_UNREACHABLE;
        }
    }
    LIBABCKIT_LOG(DEBUG) << "Appropriate StarExport was not found for service namespace import '" << serviceName << "'"
                         << std::endl;
    libabckit::statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
    return nullptr;
}

bool SetRecordIndexOff(size_t &i, ModuleIterateData *data, std::basic_string<char> &name, size_t *recordIndexOff,
                       bool *skipFilling)
{
    if ((i == 0) && (data->kind == ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT) &&
        (name.find("=ens") != std::string::npos)) {
        auto *starExport = FindStarExport(data, name);
        switch (starExport->exportingModule->target) {
            case ABCKIT_TARGET_JS: {
                starExport->GetJsImpl()->payload.GetDynamicPayload().moduleRecordIndexOff = (*recordIndexOff)++;
                break;
            }
            case ABCKIT_TARGET_ARK_TS_V1: {
                starExport->GetArkTSImpl()->payload.GetDynamicPayload().moduleRecordIndexOff = (*recordIndexOff)++;
                break;
            }
            default:
                LIBABCKIT_UNREACHABLE;
        }
        *skipFilling = true;
        return true;
    }
    return false;
}

std::pair<size_t, size_t> FillExportSection(ModuleIterateData *data, size_t idx, size_t fieldNum)
{
    size_t recordIndexOff = 0;
    size_t numExports = std::get<uint32_t>(data->moduleLitArr->literals_[idx++].value_);
    size_t sectionOffset = idx;
    bool skipFilling = false;
    while (idx < sectionOffset + numExports * fieldNum) {
        for (size_t i = 0; i < (fieldNum - 1); i++) {
            auto name = std::get<std::string>(data->moduleLitArr->literals_[idx++].value_);
            if (SetRecordIndexOff(i, data, name, &recordIndexOff, &skipFilling)) {
                break;
            }
        }
        if (skipFilling) {
            skipFilling = false;
            idx++;
            continue;
        }
        if (!FillExportDescriptor(data, idx++, recordIndexOff++)) {
            return {0, 0};
        }
    }

    return {idx, sectionOffset};
}

size_t FillRequestIdxSection(ModuleIterateData *data)
{
    auto *file = data->m->file;
    size_t idx = 0;
    auto numModuleRequests = std::get<uint32_t>(data->moduleLitArr->literals_[idx++].value_);
    data->payload->moduleRequestsOffset = idx;
    auto moduleBasePath = fs::path("./" + std::string(data->m->moduleName->impl)).remove_filename().string();
    while (idx < data->payload->moduleRequestsOffset + numModuleRequests) {
        auto relativePathStr = std::get<std::string>(data->moduleLitArr->literals_[idx].value_);
        if (relativePathStr[0] == '@') {
            data->moduleLitArr->literals_[idx++].value_ = relativePathStr;
            data->m->md.push_back(TryFindModule(relativePathStr, file));
        } else {
#ifdef STD_FILESYSTEM_EXPERIMENTAL
            auto moduleAbsPath = fs::absolute(fs::path(moduleBasePath).append(relativePathStr));
#else
            auto moduleAbsPath = fs::weakly_canonical(fs::path(moduleBasePath).append(relativePathStr));
#endif
            auto moduleName = Relative(moduleAbsPath, fs::current_path()).string();
            data->moduleLitArr->literals_[idx++].value_ = fs::path("./").append(moduleName).string();
            data->m->md.push_back(TryFindModule(moduleName, file));
        }
    }
    return idx;
}

std::pair<size_t, size_t> FillLiteralArraySection(ModuleIterateData *data, size_t idx, size_t fieldNum)
{
    if (data->isRegularImport.first) {
        return FillImportSection(data, idx, fieldNum);
    }
    return FillExportSection(data, idx, fieldNum);
}

std::string GetRecordFieldValue(const pandasm::Record *rec, const std::string &fieldName)
{
    for (const auto &field : rec->field_list) {
        if (field.name == fieldName) {
            return field.metadata->GetValue()->GetAsScalar()->GetValue<std::string>();
        }
    }
    LIBABCKIT_LOG(DEBUG) << "'" << fieldName << "' field was not found for record " << rec->name << std::endl;
    LIBABCKIT_UNREACHABLE;
}

pandasm::LiteralArray *GetLiteralArray(const pandasm::Program *prog, const std::string &arrName)
{
    auto it = prog->literalarray_table.find(arrName);
    if (it != prog->literalarray_table.end()) {
        return const_cast<pandasm::LiteralArray *>(&it->second);
    }
    LIBABCKIT_LOG(DEBUG) << "'" << arrName << "' array was not found in program" << std::endl;
    LIBABCKIT_UNREACHABLE;
}

void CreateModuleImpl(AbckitCoreModule *m)
{
    switch (m->target) {
        case ABCKIT_TARGET_JS:
            m->impl = std::make_unique<AbckitJsModule>();
            m->GetJsImpl()->core = m;
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            m->impl = std::make_unique<AbckitArktsModule>();
            m->GetArkTSImpl()->core = m;
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
    LIBABCKIT_LOG(DEBUG) << m->moduleName->impl << std::endl;
}

std::unique_ptr<AbckitCoreModule> CreateModule(pandasm::Program *prog, const pandasm::Record *record, AbckitFile *file)
{
    auto m = std::make_unique<AbckitCoreModule>();
    m->file = file;
    m->moduleName = CreateStringDynamic(file, record->name.data(), record->name.size());
    m->target = convert::ToTargetDynamic(record->language);
    LIBABCKIT_LOG(DEBUG) << "name=" << m->moduleName << " target=" << convert::ToString(m->target)
                         << " lang=" << record->language << "\n";

    CreateModuleImpl(m.get());

    auto modulePayloadDyn = AbckitModulePayloadDyn();
    modulePayloadDyn.record = record;

    pandasm::LiteralArray *moduleLitArr = GetLiteralArray(prog, GetRecordFieldValue(record, "moduleRecordIdx"));
    LIBABCKIT_LOG_DUMP(DumpModuleArray(moduleLitArr, m->moduleName->impl), DEBUG);
    auto moduleLiteralArray = std::make_unique<AbckitLiteralArray>(file, moduleLitArr);
    modulePayloadDyn.moduleLiteralArray = file->litarrs.emplace_back(std::move(moduleLiteralArray)).get();

    auto scopeNamesLiteralArray =
        std::make_unique<AbckitLiteralArray>(file, GetLiteralArray(prog, GetRecordFieldValue(record, "scopeNames")));
    modulePayloadDyn.scopeNamesLiteralArray = file->litarrs.emplace_back(std::move(scopeNamesLiteralArray)).get();

    ModuleIterateData iterData;
    iterData.m = m.get();
    iterData.payload = &modulePayloadDyn;
    iterData.moduleLitArr = moduleLitArr;

    auto res = IterateModuleSections(
        iterData, FillRequestIdxSection, FillLiteralArraySection,
        [](ModuleIterateData * /*data*/, std::pair<size_t, size_t> res) { return res.first != 0; });
    if (!res) {
        return nullptr;
    }

    modulePayloadDyn.absPaths = true;
    switch (m->target) {
        case ABCKIT_TARGET_JS:
            m->GetJsImpl()->impl = modulePayloadDyn;
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            m->GetArkTSImpl()->impl.impl = modulePayloadDyn;
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
    return m;
}

bool IsNamespaceFunc(const std::string &name)
{
    return name.find("&#") != std::string::npos;
}

enum class ParentKind {
    MODULE,
    NAMESPACE,
    CLASS,
    FUNCTION,
};

/*
 * For example:
 * - @1 -> M0N0
 * - A -> A
 */
std::string DemangleScope(const std::string &mangledName, const AbckitLiteralArray *scopeNames, bool isClass)
{
    if (mangledName.empty()) {
        return mangledName;
    }

    if (mangledName[0] != '@') {
        if (isClass) {
            size_t caretPos = mangledName.find('^');
            if (caretPos != std::string::npos) {
                return mangledName.substr(0, caretPos);
            }
        }
        return mangledName;
    }

    const int base = 16;
    size_t scopeIdx = std::stoul(mangledName.substr(1, mangledName.size() - 1), nullptr, base);
    auto *litArr = scopeNames->GetDynamicImpl();
    const size_t two = 2;
    ASSERT(litArr->literals_.size() % two == 0);
    size_t litIdx = scopeIdx * two + 1;
    ASSERT(litIdx < litArr->literals_.size());
    auto &lit = litArr->literals_[litIdx];
    ASSERT(lit.tag_ == panda_file::LiteralTag::STRING);
    return std::get<std::string>(lit.value_);
}

/*
 * This function consumes full mangled name and returns full parent name, for example:
 * - module.#&@2&A&B~C>D*E*#F -> module.#&@2&A&B~C>D*#E
 * - module.#&@2&A&B~C>D*#E -> module.#&@2&A&B~C>#D
 * - module.#&@2&A&B~C>#D -> module.#&@2&A&B~C=#C
 * - module.#&@2&A&B~C=#C -> module.#&@2&A&#B
 * - module.#&@2&A&#B -> module.#&@2&#A
 * - module.#&@2&#A -> module.#&#M0N0
 * - module.#&#M0N0 -> module
 * NOTE: There is single function name for both constructor and class (for example "module.#~C=#C")
 *       For class parent is scope where this class was declared ("module")
 *       For constructor parent is class itself ("module.#~C=#C"))
 *       So for functions "GetFunctionParentKindAndName" should be used instead of "GetParentKindAndName" to handle this
 * case
 */
std::pair<ParentKind, std::string> GetParentKindAndName(const std::string &name, const AbckitLiteralArray *scopeNames)
{
    size_t dotPos = name.rfind('.');
    std::string moduleName = name.substr(0, dotPos);
    size_t sharpPos1 = name.find('#', dotPos);
    if (sharpPos1 == std::string::npos) {
        ASSERT(name.substr(dotPos + 1) == "func_main_0");
        return {ParentKind::MODULE, moduleName};
    }
    size_t sharpPos2 = name.find('#', sharpPos1 + 1);
    ASSERT(sharpPos2 != std::string::npos);

    std::string parentName;
    auto getParentName = [](std::string &parentName, const AbckitLiteralArray *scopeNames, const std::string &prefix,
                            bool isClass = false) {
        return prefix + '#' + DemangleScope(parentName, scopeNames, isClass);
    };
    bool isCtor = IsCtor(name);
    size_t startPos = isCtor ? (name.rfind('~', sharpPos2) - 1) : (sharpPos2 - 2);
    for (size_t idx = startPos; name[idx] != '#'; idx--) {
        auto c = name[idx];
        switch (c) {
            case '&':
                return {ParentKind::NAMESPACE, getParentName(parentName, scopeNames, name.substr(0, idx + 1))};
            case '~':
                return {ParentKind::CLASS,
                        getParentName(parentName, scopeNames,
                                      name.substr(0, isCtor ? (name.rfind('~', sharpPos2)) : (sharpPos2 - 1)) + '=',
                                      true)};
            case '*':
            case '>':
            case '<':
            case '=':
                return {ParentKind::FUNCTION, getParentName(parentName, scopeNames, name.substr(0, idx + 1))};
            default:
                parentName.insert(0, 1, c);
        }
    }
    return {ParentKind::MODULE, moduleName};
}

std::pair<ParentKind, std::string> GetFunctionParentKindAndName(const std::string &name,
                                                                const AbckitLiteralArray *scopeNames)
{
    if (IsCtor(name)) {
        return {ParentKind::CLASS, name};
    }
    return GetParentKindAndName(name, scopeNames);
}

std::unique_ptr<AbckitCoreNamespace> CreateNamespace(pandasm::Function &func, AbckitFile *file)
{
    std::string &functionName = func.name;
    std::string moduleName = pandasm::GetOwnerName(functionName);
    ASSERT(file->localModules.count(moduleName) != 0);
    auto &m = file->localModules[moduleName];
    ASSERT(m->target == ABCKIT_TARGET_ARK_TS_V1);

    auto namespaceFunc = std::make_unique<AbckitCoreFunction>();
    namespaceFunc->owningModule = m.get();
    namespaceFunc->impl = std::make_unique<AbckitArktsFunction>();
    namespaceFunc->GetArkTSImpl()->impl = &func;
    namespaceFunc->GetArkTSImpl()->core = namespaceFunc.get();
    auto &nameToFunction = func.IsStatic() ? file->nameToFunctionStatic : file->nameToFunctionInstance;
    ASSERT(nameToFunction.count(func.name) == 0);
    nameToFunction[func.name] = namespaceFunc.get();

    auto n = std::make_unique<AbckitCoreNamespace>(m.get());
    n->impl = std::make_unique<AbckitArktsNamespace>();
    n->GetArkTSImpl()->core = n.get();
    n->GetArkTSImpl()->f = std::move(namespaceFunc);
    return n;
}

AbckitLiteralArray *GetScopeNamesArray(AbckitCoreModule *m)
{
    switch (m->target) {
        case ABCKIT_TARGET_JS:
            return m->GetJsImpl()->impl.scopeNamesLiteralArray;
        case ABCKIT_TARGET_ARK_TS_V1:
            return m->GetArkTSImpl()->impl.GetDynModule().scopeNamesLiteralArray;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

void AssignNamespacesToParent(std::vector<std::unique_ptr<AbckitCoreNamespace>> &namespaces,
                              std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace)
{
    for (auto &n : namespaces) {
        ASSERT(n->owningModule->target == ABCKIT_TARGET_ARK_TS_V1);
        auto &nName = GetDynFunction(n->GetArkTSImpl()->f.get())->name;
        panda::pandasm::Function *func = n->GetArkTSImpl()->f->GetArkTSImpl()->GetDynamicImpl();
        auto [kind, parentName] = GetParentKindAndName(func->name, GetScopeNamesArray(n->owningModule));
        switch (kind) {
            case ParentKind::MODULE:
                n->owningModule->nt.emplace(nName, std::move(n));
                break;
            case ParentKind::NAMESPACE: {
                ASSERT(nameToNamespace.count(parentName) == 1);
                auto *parentNamespace = nameToNamespace[parentName];
                n->parentNamespace = nameToNamespace[parentName];
                parentNamespace->nt.emplace(nName, std::move(n));
                break;
            }
            default:
                LIBABCKIT_UNREACHABLE;
        }
    }
}

std::unique_ptr<AbckitCoreClass> CreateClass(const std::string &functionName, panda::pandasm::Function &function,
                                             AbckitFile *file,
                                             std::unordered_map<std::string, AbckitCoreClass *> &nameToClass)
{
    std::string moduleName = pandasm::GetOwnerName(functionName);
    ASSERT(file->localModules.count(moduleName) != 0);
    auto &m = file->localModules[moduleName];

    auto abckitFunction = &function;
    std::unique_ptr<AbckitCoreClass> c;

    switch (m->target) {
        case ABCKIT_TARGET_JS:
            c = std::make_unique<AbckitCoreClass>(m.get(), AbckitJsClass(abckitFunction));
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            c = std::make_unique<AbckitCoreClass>(m.get(), AbckitArktsClass(abckitFunction));
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
    ASSERT(nameToClass.count(functionName) == 0);
    nameToClass[functionName] = c.get();
    return c;
}

void AssignClassesToParent(std::vector<std::unique_ptr<AbckitCoreClass>> &klasses,
                           std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
                           std::unordered_map<std::string, AbckitCoreFunction *> &nameToFunction)
{
    for (auto &klass : klasses) {
        std::string name;
        switch (klass->owningModule->target) {
            case ABCKIT_TARGET_JS:
                name = klass->GetJsImpl()->impl->name;
                break;
            case ABCKIT_TARGET_ARK_TS_V1:
                name = klass->GetArkTSImpl()->impl.GetDynamicClass()->name;
                break;
            default:
                LIBABCKIT_UNREACHABLE;
        }
        auto [kind, parentName] = GetParentKindAndName(name, GetScopeNamesArray(klass->owningModule));
        switch (kind) {
            case ParentKind::MODULE:
                klass->owningModule->InsertClass(name, std::move(klass));
                break;
            case ParentKind::NAMESPACE: {
                ASSERT(nameToNamespace.count(parentName) != 0);
                auto *n = nameToNamespace[parentName];
                klass->parentNamespace = n;
                n->ct.emplace(name, std::move(klass));
                break;
            }
            case ParentKind::FUNCTION: {
                ASSERT(nameToFunction.count(parentName) != 0);
                auto *f = nameToFunction[parentName];
                klass->parentFunction = f;
                f->nestedClasses.emplace_back(std::move(klass));
                break;
            }
            case ParentKind::CLASS:
                klass->owningModule->InsertClass(name, std::move(klass));
                break;
            default:
                LIBABCKIT_UNREACHABLE;
        }
    }
}

void CreateAnnotationInterface(AbckitFile *file, const std::string &recName, pandasm::Record &rec)
{
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    std::string moduleName = pandasm::GetOwnerName(recName);
    ASSERT(file->localModules.count(moduleName) != 0);
    auto &module = file->localModules[moduleName];
    ai->owningModule = module.get();
    ai->impl = std::make_unique<AbckitArktsAnnotationInterface>();
    ai->GetArkTSImpl()->impl = &rec;
    ai->GetArkTSImpl()->core = ai.get();

    auto &fields = rec.field_list;
    ASSERT(module->target != ABCKIT_TARGET_JS || fields.empty());
    for (auto &field : fields) {
        auto aiField = std::make_unique<AbckitCoreAnnotationInterfaceField>();
        aiField->impl = std::make_unique<AbckitArktsAnnotationInterfaceField>();
        aiField->GetArkTSImpl()->core = aiField.get();

        aiField->ai = ai.get();
        aiField->name = CreateNameString(field.name, file);

        aiField->type =
            GetOrCreateType(file, PandaTypeToAbcKitTypeId(field.type), field.type.GetRank(), nullptr, nullptr);

        if (field.metadata->GetValue().has_value()) {
            aiField->value = FindOrCreateValueDynamic(file, field.metadata->GetValue().value());
        }

        ai->fields.emplace_back(std::move(aiField));
    }

    module->at.emplace(pandasm::GetItemName(recName), std::move(ai));
}

void SetImpl(std::unique_ptr<AbckitCoreFunction> &function, panda::pandasm::Function &functionImpl,
             std::unique_ptr<AbckitCoreModule> &m)
{
    switch (m->target) {
        case ABCKIT_TARGET_JS:
            function->impl = std::make_unique<AbckitJsFunction>();
            function->GetJsImpl()->impl = &functionImpl;
            function->GetJsImpl()->core = function.get();
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            function->impl = std::make_unique<AbckitArktsFunction>();
            function->GetArkTSImpl()->impl = &functionImpl;
            function->GetArkTSImpl()->core = function.get();
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

constexpr int REGULAR_IMPORT_SPAN = 3;
constexpr int NOT_REGULAR_IMPORT_SPAN = 2;

void CollectAnnotationModuleMap(AbckitCoreFunction *function,
                                std::unordered_map<std::string, std::string> &importModules)
{
    for (auto &id : function->owningModule->id) {
        if (id->importingModule->GetArkTSImpl() == nullptr) {
            continue;
        }
        std::string_view mName = id->importedModule->moduleName->impl;
        auto mPayload = &id->importingModule->GetArkTSImpl()->impl.GetDynModule();
        auto idPayload = &id->GetArkTSImpl()->payload.GetDynId();
        const auto *moduleLitArr = mPayload->moduleLiteralArray;
        auto sectionOffset =
            idPayload->isRegularImport ? mPayload->regularImportsOffset : mPayload->namespaceImportsOffset;
        if (idPayload->isRegularImport) {
            sectionOffset = sectionOffset + idPayload->moduleRecordIndexOff * REGULAR_IMPORT_SPAN;
        } else {
            sectionOffset = sectionOffset + idPayload->moduleRecordIndexOff * NOT_REGULAR_IMPORT_SPAN;
        }
        std::string iName = std::get<std::string>(moduleLitArr->GetDynamicImpl()->literals_[sectionOffset].value_);
        importModules[iName] = mName;
    }
}

AbckitCoreAnnotationInterface *CollectAnnotationCorrectInfo(AbckitFile *file,
                                                            const panda::pandasm::AnnotationData &annoImpl,
                                                            std::unordered_map<std::string, std::string> &importModules,
                                                            std::vector<std::string> &removes,
                                                            std::vector<panda::pandasm::AnnotationData> &inserts)
{
    std::string aiModuleName = pandasm::GetOwnerName(annoImpl.GetName());
    std::string aiName = pandasm::GetItemName(annoImpl.GetName());
    std::string mName;
    if (importModules.find(aiName) != importModules.end() && (mName = importModules[aiName]) != aiModuleName) {
        auto &localModule = file->localModules[mName];
        if (localModule.get() == nullptr) {
            return nullptr;
        }
        auto &annotationInterface = localModule->at[aiName];
        if (annotationInterface.get() != nullptr &&
            libabckit::IsAnnotationInterfaceRecord(*(annotationInterface->GetArkTSImpl()->GetDynamicImpl()))) {
            inserts.emplace_back(mName + "." + aiName, annoImpl.GetElements());
            removes.emplace_back(annoImpl.GetName());
            return annotationInterface.get();
        }
    }
    return nullptr;
}

void CreateFunctionAnnotations(AbckitFile *file, panda::pandasm::Function &functionImpl, AbckitCoreFunction *function)
{
    std::vector<std::string> removes;
    std::vector<panda::pandasm::AnnotationData> inserts;
    std::unordered_map<std::string, std::string> importModules;
    if (function->owningModule->target == ABCKIT_TARGET_ARK_TS_V1) {
        CollectAnnotationModuleMap(function, importModules);
    }

    for (auto &annoImpl : functionImpl.metadata->GetAnnotations()) {
        if (libabckit::IsServiceRecord(annoImpl.GetName())) {
            continue;
        }
        ASSERT(function->owningModule->target == ABCKIT_TARGET_ARK_TS_V1);
        auto anno = std::make_unique<AbckitCoreAnnotation>();
        std::string aiModuleName = pandasm::GetOwnerName(annoImpl.GetName());
        std::string aiName = pandasm::GetItemName(annoImpl.GetName());
        anno->name = CreateNameString(aiName, file);
        anno->ai = file->localModules[aiModuleName]->at[aiName].get();
        if (anno->ai == nullptr) {
            anno->ai = CollectAnnotationCorrectInfo(file, annoImpl, importModules, removes, inserts);
        }
        anno->impl = std::make_unique<AbckitArktsAnnotation>();
        anno->GetArkTSImpl()->core = anno.get();

        for (auto &annoElemImpl : annoImpl.GetElements()) {
            auto annoElem = std::make_unique<AbckitCoreAnnotationElement>();
            annoElem->ann = anno.get();
            annoElem->name = CreateStringDynamic(file, annoElemImpl.GetName().data(), annoElemImpl.GetName().size());
            annoElem->impl = std::make_unique<AbckitArktsAnnotationElement>();
            annoElem->GetArkTSImpl()->core = annoElem.get();
            auto value = FindOrCreateValueDynamic(file, *annoElemImpl.GetValue());
            annoElem->value = value;

            anno->elements.emplace_back(std::move(annoElem));
        }
        anno->owner = function;
        function->annotations.emplace_back(std::move(anno));
    }

    if (function->owningModule->target == ABCKIT_TARGET_ARK_TS_V1) {
        for (auto &remove : removes) {
            functionImpl.metadata->DeleteAnnotationByName(remove);
        }
        functionImpl.metadata->AddAnnotations(inserts);
    }
}

std::unique_ptr<AbckitCoreFunction> CreateFunction(const std::string &functionName,
                                                   panda::pandasm::Function &functionImpl, AbckitFile *file)
{
    std::string moduleName = pandasm::GetOwnerName(functionName);
    if (file->localModules.count(moduleName) == 0) {
        return std::unique_ptr<AbckitCoreFunction> {};
    }
    auto &m = file->localModules[moduleName];

    auto function = std::make_unique<AbckitCoreFunction>();
    function->owningModule = m.get();
    if (m->target == ABCKIT_TARGET_ARK_TS_V1) {
        auto functionKind = functionImpl.GetFunctionKind();
        function->isAnonymous = functionKind == panda::panda_file::FunctionKind::NC_FUNCTION ||
                                functionKind == panda::panda_file::FunctionKind::ASYNC_NC_FUNCTION;
    } else {
        function->isAnonymous = IsAnonymousName(functionName);
    }
    SetImpl(function, functionImpl, m);
    auto &nameToFunction = functionImpl.IsStatic() ? file->nameToFunctionStatic : file->nameToFunctionInstance;
    ASSERT(nameToFunction.count(functionImpl.name) == 0);
    nameToFunction[functionImpl.name] = function.get();

    CreateFunctionAnnotations(file, functionImpl, function.get());

    return function;
}

void AssignFunction(std::unique_ptr<AbckitCoreFunction> &f, std::string &name,
                    AbckitLiteralArray *scopeNamesLiteralArray,
                    std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
                    std::unordered_map<std::string, AbckitCoreClass *> &nameToClass,
                    std::unordered_map<std::string, AbckitCoreFunction *> &nameToFunction)
{
    auto [kind, parentName] = GetFunctionParentKindAndName(name, scopeNamesLiteralArray);
    switch (kind) {
        case ParentKind::MODULE:
            f->owningModule->functions.emplace_back(std::move(f));
            return;
        case ParentKind::NAMESPACE: {
            ASSERT(nameToNamespace.count(parentName) == 1);
            auto *n = nameToNamespace[parentName];
            f->parentNamespace = n;
            n->functions.emplace_back(std::move(f));
            return;
        }
        case ParentKind::CLASS: {
            ASSERT(nameToClass.count(parentName) == 1);
            auto *c = nameToClass[parentName];
            f->parentClass = c;
            c->methods.emplace_back(std::move(f));
            return;
        }
        case ParentKind::FUNCTION: {
            ASSERT(nameToFunction.count(parentName) == 1);
            auto *parentF = nameToFunction[parentName];
            f->parentFunction = parentF;
            parentF->nestedFunction.emplace_back(std::move(f));
            return;
        }
    }
}

void AssignFunctionsToParent(std::vector<std::unique_ptr<AbckitCoreFunction>> &functions,
                             std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
                             std::unordered_map<std::string, AbckitCoreClass *> &nameToClass,
                             std::unordered_map<std::string, AbckitCoreFunction *> &nameToFunction)
{
    for (auto &f : functions) {
        std::string name;
        AbckitLiteralArray *scopeNamesLiteralArray;
        switch (f->owningModule->target) {
            case ABCKIT_TARGET_JS:
                name = f->GetJsImpl()->impl->name;
                scopeNamesLiteralArray = f->owningModule->GetJsImpl()->impl.scopeNamesLiteralArray;
                break;
            case ABCKIT_TARGET_ARK_TS_V1:
                name = f->GetArkTSImpl()->GetDynamicImpl()->name;
                scopeNamesLiteralArray = f->owningModule->GetArkTSImpl()->impl.GetDynModule().scopeNamesLiteralArray;
                break;
            default:
                LIBABCKIT_UNREACHABLE;
        }

        if (f->isAnonymous) {
            f->owningModule->functions.emplace_back(std::move(f));
            continue;
        }

        if (IsCtor(name)) {
            ASSERT(nameToClass.count(name) == 1);
            auto *c = nameToClass[name];
            f->parentClass = c;
            for (auto &anno : f->annotations) {
                anno->owner = c;
                c->annotations.emplace_back(std::move(anno));
            }
            f->annotations.clear();
            c->methods.emplace_back(std::move(f));
            continue;
        }

        AssignFunction(f, name, scopeNamesLiteralArray, nameToNamespace, nameToClass, nameToFunction);
    }
}

void DumpHierarchy(AbckitFile *file)
{
    std::function<void(AbckitCoreFunction * f, const std::string &indent)> dumpFunc;
    std::function<void(AbckitCoreClass * c, const std::string &indent)> dumpClass;

    dumpFunc = [&dumpFunc, &dumpClass](AbckitCoreFunction *f, const std::string &indent = "") {
        auto fName = GetDynFunction(f)->name;
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << fName << std::endl;
        for (auto &cNested : f->nestedClasses) {
            dumpClass(cNested.get(), indent + "  ");
        }
        for (auto &fNested : f->nestedFunction) {
            dumpFunc(fNested.get(), indent + "  ");
        }
    };

    dumpClass = [&dumpFunc](AbckitCoreClass *c, const std::string &indent = "") {
        auto cName = GetDynFunction(c)->name;
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << cName << std::endl;
        for (auto &f : c->methods) {
            dumpFunc(f.get(), indent + "  ");
        }
    };

    std::function<void(AbckitCoreNamespace * n, const std::string &indent)> dumpNamespace =
        [&dumpFunc, &dumpClass, &dumpNamespace](AbckitCoreNamespace *n, const std::string &indent = "") {
            ASSERT(n->owningModule->target == ABCKIT_TARGET_ARK_TS_V1);
            auto &nName = GetDynFunction(n->GetArkTSImpl()->f.get())->name;
            LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << nName << std::endl;
            for (auto &[_, n] : n->nt) {
                dumpNamespace(n.get(), indent + "  ");
            }
            for (auto &[_, c] : n->ct) {
                dumpClass(c.get(), indent + "  ");
            }
            for (auto &f : n->functions) {
                dumpFunc(f.get(), indent + "  ");
            }
        };

    for (auto &[mName, m] : file->localModules) {
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << mName << std::endl;
        for (auto &[_, n] : m->nt) {
            dumpNamespace(n.get(), "");
        }
        for (auto &[cName, c] : m->ct) {
            dumpClass(c.get(), "");
        }
        for (auto &f : m->functions) {
            dumpFunc(f.get(), "");
        }
    }
}

AbckitCoreModule *ResolveUnfoundModule(AbckitCoreModule *m, AbckitFile *file, size_t offset)
{
    auto mPayload = GetDynModulePayload(m);
    auto literalArr = mPayload->moduleLiteralArray->GetDynamicImpl();
    auto requestIdx = std::get<uint16_t>(literalArr->literals_[offset].value_);
    auto moduleName = std::get<std::string>(literalArr->literals_[requestIdx + 1].value_);
    if (moduleName.substr(0, 2U) == "./") {
        moduleName = moduleName.substr(2U);
    }
    auto foundModule = TryFindModule(moduleName, file);
    if (foundModule == nullptr) {
        auto md = std::make_unique<AbckitCoreModule>();
        md->target = ABCKIT_TARGET_UNKNOWN;
        md->isExternal = true;
        md->file = file;
        md->moduleName = CreateNameString(moduleName, file);
        file->externalModules.emplace(moduleName, std::move(md));
        foundModule = file->externalModules[moduleName].get();
    }
    if (m->md.at(requestIdx) == nullptr) {
        m->md.at(requestIdx) = foundModule;
    }
    ASSERT(m->md.at(requestIdx) == foundModule);
    return foundModule;
}

void ResolveImportedModule(AbckitFile *file, std::unique_ptr<AbckitCoreModule> &m,
                           std::unique_ptr<AbckitCoreImportDescriptor> &id, AbckitModulePayloadDyn *mPayload)
{
    AbckitDynamicImportDescriptorPayload *idDyn = GetDynImportDescriptorPayload(id.get());
    auto offset = idDyn->moduleRecordIndexOff;
    auto absOffset = idDyn->isRegularImport ? mPayload->regularImportsOffset + offset * 3U + 2U
                                            : mPayload->namespaceImportsOffset + offset * 2U + 1;
    auto importedModule = ResolveUnfoundModule(m.get(), file, absOffset);
    id->importedModule = importedModule;
}

void ResolveExportedModule(AbckitFile *file, std::unique_ptr<AbckitCoreModule> &m,
                           std::unique_ptr<AbckitCoreExportDescriptor> &ed, AbckitModulePayloadDyn *mPayload)
{
    AbckitDynamicExportDescriptorPayload *edDyn = GetDynExportDescriptorPayload(ed.get());
    auto offset = edDyn->moduleRecordIndexOff;
    auto absOffset = 0;
    switch (edDyn->kind) {
        case ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT: {
            absOffset = mPayload->indirectExportsOffset + offset * 3U + 2U;
            break;
        }
        case ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT: {
            if (edDyn->hasServiceImport) {
                absOffset = mPayload->namespaceImportsOffset + edDyn->serviceNamespaceImportIdx * 2U + 1;
            } else {
                absOffset = mPayload->starExportsOffset + offset;
            }
            break;
        }
        default:
            UNREACHABLE();
    }
    auto exportedModule = ResolveUnfoundModule(m.get(), file, absOffset);
    ed->exportedModule = exportedModule;
}

void ResolveUnfoundModules(AbckitFile *file)
{
    for (auto &[name, m] : file->localModules) {
        if (std::find(m->md.begin(), m->md.end(), nullptr) == m->md.end()) {
            continue;
        }
        auto mPayload = GetDynModulePayload(m.get());
        for (auto &id : m->id) {
            if (id->importedModule != nullptr) {
                continue;
            }
            ResolveImportedModule(file, m, id, mPayload);
        }
        for (auto &ed : m->ed) {
            if (ed->exportedModule != nullptr) {
                continue;
            }
            ResolveExportedModule(file, m, ed, mPayload);
        }
    }
}

void CollectModules(pandasm::Program *prog, AbckitFile *file)
{
    for (const auto &[recName, rec] : prog->record_table) {
        LIBABCKIT_LOG(DEBUG) << "RECORD: " << recName << ' ' << rec.name << '\n';
        if (!libabckit::IsModuleDescriptorRecord(rec) || libabckit::IsExternalRecord(rec)) {
            continue;
        }
        auto m = CreateModule(prog, &rec, file);
        file->localModules.insert({rec.name, std::move(m)});
    }
    ResolveUnfoundModules(file);
}

void CollectAI(pandasm::Program *prog, AbckitFile *file)
{
    for (auto &[recName, rec] : prog->record_table) {
        if (!libabckit::IsAnnotationInterfaceRecord(rec) || libabckit::IsServiceRecord(recName)) {
            continue;
        }
        LIBABCKIT_LOG(DEBUG) << "ANNOTATION: " << recName << ' ' << rec.name << '\n';
        CreateAnnotationInterface(file, recName, rec);
    }
}

void CollectNamespaces(pandasm::Program *prog, AbckitFile *file,
                       std::vector<std::unique_ptr<AbckitCoreNamespace>> *namespaces,
                       std::unordered_map<std::string, AbckitCoreNamespace *> *nameToNamespace)
{
    for (auto &[functionName, function] : prog->function_table) {
        if (!IsNamespaceFunc(functionName)) {
            continue;
        }
        namespaces->emplace_back(CreateNamespace(function, file));
        ASSERT(nameToNamespace->count(function.name) == 0);
        (*nameToNamespace)[function.name] = namespaces->back().get();
    }
    AssignNamespacesToParent(*namespaces, *nameToNamespace);
}

void CreateWrappers(pandasm::Program *prog, AbckitFile *file)
{
    file->program = prog;

    CollectModules(prog, file);
    CollectAI(prog, file);
    std::vector<std::unique_ptr<AbckitCoreNamespace>> namespaces;
    std::unordered_map<std::string, AbckitCoreNamespace *> nameToNamespace;
    CollectNamespaces(prog, file, &namespaces, &nameToNamespace);

    // Collect classes
    std::vector<std::unique_ptr<AbckitCoreClass>> classes;
    std::unordered_map<std::string, AbckitCoreClass *> nameToClass;
    for (auto &[functionName, function] : prog->function_table) {
        if (!libabckit::IsCtor(functionName)) {
            continue;
        }
        classes.emplace_back(CreateClass(functionName, function, file, nameToClass));
    }

    // Collect functions
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    std::unordered_map<std::string, AbckitCoreFunction *> nameToFunction;
    for (auto &[functionName, function] : prog->function_table) {
        LIBABCKIT_LOG(DEBUG) << "f: " << functionName << " ; " << function.name << '\n';
        if (!libabckit::IsFunction(functionName)) {
            continue;
        }
        auto abckitCoreFunction = CreateFunction(functionName, function, file);
        if (abckitCoreFunction == nullptr) {
            continue;
        }
        functions.emplace_back(std::move(abckitCoreFunction));
        ASSERT(nameToFunction.count(function.name) == 0);
        nameToFunction[function.name] = functions.back().get();
    }

    // Assign parents
    AssignClassesToParent(classes, nameToNamespace, nameToFunction);
    AssignFunctionsToParent(functions, nameToNamespace, nameToClass, nameToFunction);

    LIBABCKIT_LOG_DUMP(DumpHierarchy(file), DEBUG);

    // Strings
    for (auto &sImpl : prog->strings) {
        auto s = std::make_unique<AbckitString>();
        s->impl = sImpl;
        file->strings.insert({sImpl, std::move(s)});
    }
}

void ClearProgramDebugInfo(pandasm::Program &program)
{
    for (auto &[name, func] : program.function_table) {
        func.local_variable_debug.clear();
    }
}
}  // namespace

namespace libabckit {

struct CtxIInternal {
    panda::abc2program::Abc2ProgramDriver *driver = nullptr;
};

AbckitFile *OpenAbcDynamic(const char *path, size_t len)
{
    LIBABCKIT_LOG_FUNC;
    auto spath = std::string(path, len);
    LIBABCKIT_LOG(DEBUG) << spath << '\n';
    auto *abc2program = new panda::abc2program::Abc2ProgramDriver();
    if (!abc2program->Compile(spath)) {
        LIBABCKIT_LOG(DEBUG) << "Failed to open " << spath << "\n";
        delete abc2program;
        return nullptr;
    }
    auto &prog = const_cast<pandasm::Program &>(abc2program->GetProgram());
    ClearProgramDebugInfo(prog);
    auto file = new AbckitFile();
    file->frontend = Mode::DYNAMIC;
    CreateWrappers(&prog, file);

    auto pf = panda_file::File::Open(spath);
    if (pf == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "Failed to panda_file::File::Open\n";
        delete abc2program;
        delete file;
        return nullptr;
    }

    auto pandaFileVersion = pf->GetHeader()->version;
    uint8_t *fileVersion = pandaFileVersion.data();

    file->version = new uint8_t[ABCKIT_VERSION_SIZE];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy(fileVersion, fileVersion + sizeof(uint8_t) * ABCKIT_VERSION_SIZE, file->version);

    file->internal = new CtxIInternal {abc2program};
    return file;
}

void WriteAbcDynamic(AbckitFile *file, const char *path, size_t len)
{
    if (file->needOptimize) {
        std::unordered_map<AbckitCoreFunction *, FunctionStatus *> &functionsMap = file->functionsMap;
        for (auto &[func, status] : functionsMap) {
            if (status->writeBack) {
                FunctionSetGraphDynamicSync(func, status->graph);
            }
            delete status;
        }
        functionsMap.clear();
    }

    auto program = file->GetDynamicProgram();

    pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = nullptr;

    EmitDynamicProgram(file, program, mapsp, false, std::string_view(path, len));
    if (statuses::GetLastError() != AbckitStatus::ABCKIT_STATUS_NO_ERROR) {
        return;
    }

    LIBABCKIT_LOG(DEBUG) << "SUCCESS\n";
}

void DestroyGraphDynamic(AbckitGraph *graph)
{
    if (!graph->file->needOptimize) {
        DestroyGraphDynamicSync(graph);
    }
}

void DestroyGraphDynamicSync(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    GraphWrapper::DestroyGraphDynamic(graph);
}

void CloseFileDynamic(AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    for (auto &[_, type] : file->types) {
        if (type != nullptr) {
            type->functionUsers.clear();
            type->fieldTypeUsers.clear();
            type->types.clear();
        }
    }
    for (auto &[_, type] : file->types) {
        if (type != nullptr) {
            delete type;
        }
    }
    file->types.clear();

    if (file->needOptimize) {
        std::unordered_map<AbckitCoreFunction *, FunctionStatus *> &functionsMap = file->functionsMap;
        for (auto &[_, status] : functionsMap) {
            DestroyGraphDynamicSync(status->graph);
            delete status;
        }
        functionsMap.clear();

        if (file->generateStatus.pf != nullptr) {
            delete static_cast<panda_file::File *>(file->generateStatus.pf);
        }
        if (file->generateStatus.maps != nullptr) {
            delete static_cast<pandasm::AsmEmitter::PandaFileToPandaAsmMaps *>(file->generateStatus.maps);
        }
    }

    auto *ctxIInternal = reinterpret_cast<struct CtxIInternal *>(file->internal);
    delete ctxIInternal->driver;
    delete ctxIInternal;
    delete[] file->version;
    delete file;
}

}  // namespace libabckit
