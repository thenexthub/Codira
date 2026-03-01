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

#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_dynamic/helpers_dynamic.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/statuses_impl.h"
#include "assembler/assembly-program.h"
#include "assembler/assembly-record.h"

#if defined(_WIN32)
#include <cwchar>
#include <codecvt>
#include <locale>
#endif
#include <cstdint>
#include <memory>

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda;
// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace libabckit;

namespace {

// CC-OFFNXT(G.NAM.03) project code style
bool UpdateLitArrSectionPostAction(ModuleIterateData *data, std::pair<size_t, size_t> res)
{
    bool *isChangeField {nullptr};
    size_t *offset {nullptr};
    if (data->isRegularImport.first) {
        if (data->isRegularImport.second) {
            ASSERT(data->payload->regularImportsOffset == res.second);
            isChangeField = &(data->updateData.isRegularImportsChange);
            offset = &(data->payload->regularImportsOffset);
        } else {
            offset = &(data->payload->namespaceImportsOffset);
        }
    } else {
        switch (data->kind) {
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT: {
                isChangeField = &(data->updateData.isLocalExportsChange);
                offset = &(data->payload->localExportsOffset);
                break;
            }
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT: {
                offset = &(data->payload->indirectExportsOffset);
                break;
            }
            case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT: {
                offset = &(data->payload->starExportsOffset);
                break;
            }
        }
    }

    auto prevNum = std::get<uint32_t>(data->moduleLitArr->literals_[*offset - 1].value_);
    auto newNum = std::get<uint32_t>((data->updateData.newLiterals)[res.second - 1].value_);
    if (prevNum != newNum && (isChangeField != nullptr)) {
        *isChangeField = true;
    }
    return true;
}

std::pair<size_t, size_t> UpdateLitArrSection(ModuleIterateData *data, size_t idx, size_t fieldNum)
{
    auto &newLiterals = data->updateData.newLiterals;
    auto *idxMap =
        data->isRegularImport.first ? &data->updateData.regularImportsIdxMap : &data->updateData.localExportsIdxMap;

    auto num = std::get<uint32_t>(data->moduleLitArr->literals_[idx].value_);
    newLiterals.push_back(std::move(data->moduleLitArr->literals_[idx++]));
    auto newOffset = newLiterals.size();
    auto prevOffset = idx;
    uint32_t oldIdx = 0;
    uint32_t actualNumber = 0;
    while (idx < prevOffset + num * fieldNum) {
        if (data->moduleLitArr->literals_[idx].tag_ == panda_file::LiteralTag::NULLVALUE) {
            for (size_t i = 0; i < fieldNum; i++) {
                idx++;
            }
            oldIdx++;
            continue;
        }
        idxMap->emplace(oldIdx, actualNumber);
        actualNumber++;
        oldIdx++;
        for (size_t i = 0; i < fieldNum; i++) {
            newLiterals.push_back(std::move(data->moduleLitArr->literals_[idx++]));
        }
    }
    if (actualNumber != num) {
        newLiterals[newOffset - 1].value_ = actualNumber;
    }
    return {idx, newOffset};
}

size_t IterateRequestIdxSectionBeforeUpdate(ModuleIterateData *data)
{
    auto &newLiterals = data->updateData.newLiterals;
    size_t idx = 0;
    while (idx < (data->payload->regularImportsOffset - 1)) {
        newLiterals.push_back(std::move(data->moduleLitArr->literals_[idx++]));
    }
    return idx;
}

bool CheckRegularImport(ModuleUpdateData *updateData, panda::pandasm::InsPtr &inst)
{
    if (updateData->isRegularImportsChange) {
        if (inst->opcode == pandasm::Opcode::LDEXTERNALMODULEVAR ||
            inst->opcode == pandasm::Opcode::WIDE_LDEXTERNALMODULEVAR) {
            auto imm = static_cast<uint32_t>(std::get<int64_t>(inst->GetImm(0)));
            auto foundIdx = updateData->regularImportsIdxMap.find(imm);
            if (foundIdx == updateData->regularImportsIdxMap.end()) {
                LIBABCKIT_LOG(DEBUG) << "There is an instruction '" << inst->ToString()
                                     << "' with an unknown regular import index '" << std::hex << imm << "'\n";
                statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
                return false;
            }
            auto idx = (int64_t)foundIdx->second;
            inst->SetImm(0, idx);
        }
    }
    return true;
}

bool CheckLocalExports(ModuleUpdateData *updateData, panda::pandasm::InsPtr &inst)
{
    if (updateData->isLocalExportsChange) {
        auto op = inst->opcode;
        if (op == pandasm::Opcode::LDLOCALMODULEVAR || op == pandasm::Opcode::WIDE_LDLOCALMODULEVAR ||
            op == pandasm::Opcode::STMODULEVAR || op == pandasm::Opcode::WIDE_STMODULEVAR) {
            auto imm = static_cast<uint32_t>(std::get<int64_t>(inst->GetImm(0)));
            auto foundIdx = updateData->localExportsIdxMap.find(imm);
            if (foundIdx == updateData->localExportsIdxMap.end()) {
                LIBABCKIT_LOG(DEBUG) << "There is an instruction '" << inst->ToString()
                                     << "' with an unknown local export index '" << std::hex << imm << "'\n";
                statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
                return false;
            }
            auto idx = (int64_t)foundIdx->second;
            inst->SetImm(0, idx);
        }
    }
    return true;
}

bool UpdateInsImms(pandasm::Program *program, ModuleUpdateData *updateData, const std::string &mainRecordName)
{
    for (auto &[name, func] : program->function_table) {
        if (pandasm::GetOwnerName(name) != mainRecordName) {
            continue;
        }
        for (auto &inst : func.ins) {
            if (!CheckRegularImport(updateData, inst)) {
                return false;
            }

            if (!CheckLocalExports(updateData, inst)) {
                return false;
            }
        }
    }
    return true;
}

AbckitModulePayloadDyn *GetModulePayload(AbckitCoreModule *module)
{
    switch (module->target) {
        case ABCKIT_TARGET_JS:
            return &(module->GetJsImpl()->impl);
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            return &(module->GetArkTSImpl()->impl.GetDynModule());
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

bool UpdateModuleLiteralArray(AbckitFile *file, const std::string &recName)
{
    auto program = file->GetDynamicProgram();
    auto module = file->localModules.find(recName);
    if (module == file->localModules.end()) {
        LIBABCKIT_LOG(DEBUG) << "Can not find module with name '" << recName << "'\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    AbckitModulePayloadDyn *mPayload = GetModulePayload(module->second.get());
    auto moduleLitArr = mPayload->moduleLiteralArray->GetDynamicImpl();
    if (mPayload->absPaths) {
        size_t idx = 1;
        auto moduleBasePath = fs::path("./" + recName).remove_filename();
        while (idx < (mPayload->regularImportsOffset - 1)) {
            auto moduleAbsPath = std::get<std::string>(moduleLitArr->literals_[idx].value_);
            if (moduleAbsPath[0] == '@') {
                moduleLitArr->literals_[idx++].value_ = moduleAbsPath;
            } else {
                auto relativePath = Relative(moduleAbsPath, moduleBasePath).string();
                moduleLitArr->literals_[idx++].value_ = fs::path("./").append(relativePath).string();
            }
        }
        mPayload->absPaths = false;
    }

    bool modified = std::any_of(
        moduleLitArr->literals_.cbegin(), moduleLitArr->literals_.cend(),
        [](const pandasm::LiteralArray::Literal &lit) { return lit.tag_ == panda_file::LiteralTag::NULLVALUE; });
    if (!modified) {
        return true;
    }

    ModuleIterateData iterData;
    iterData.m = module->second.get();
    iterData.payload = mPayload;
    iterData.moduleLitArr = moduleLitArr;

    if (!IterateModuleSections(iterData, IterateRequestIdxSectionBeforeUpdate, UpdateLitArrSection,
                               UpdateLitArrSectionPostAction)) {
        return false;
    }

    if (iterData.updateData.isRegularImportsChange || iterData.updateData.isLocalExportsChange) {
        if (!UpdateInsImms(program, &iterData.updateData, recName)) {
            return false;
        }
    }

    moduleLitArr->literals_ = std::move(iterData.updateData.newLiterals);

    LIBABCKIT_LOG_DUMP(DumpModuleArray(moduleLitArr, iterData.m->moduleName->impl), DEBUG);
    return true;
}

}  // namespace

namespace libabckit {

// CC-OFFNXT(G.FUN.01-CPP) helper function
const panda_file::File *EmitDynamicProgram(AbckitFile *file, pandasm::Program *program,
                                           pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp, bool getFile,
                                           std::string_view path)
{
    for (auto &[recName, rec] : program->record_table) {
        if (!IsModuleDescriptorRecord(rec) || IsExternalRecord(rec)) {
            continue;
        }
        if (!UpdateModuleLiteralArray(file, recName)) {
            return nullptr;
        }
    }

    const panda_file::File *pf = [&]() {
        const panda_file::File *res {nullptr};
        if (!getFile) {
            std::map<std::string, size_t> *statp = nullptr;
            if (!pandasm::AsmEmitter::Emit(std::string(path), *program, statp, mapsp)) {
                LIBABCKIT_LOG_NO_FUNC(DEBUG)
                    << "[EmitDynamicProgram] FAILURE: " << pandasm::AsmEmitter::GetLastError() << '\n';
                statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
            }
            return res;
        }
        return pandasm::AsmEmitter::Emit(*program, mapsp, 1U).release();
    }();
    if (getFile && pf == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "FAILURE: " << pandasm::AsmEmitter::GetLastError() << '\n';
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    return pf;
}

fs::path Relative(const fs::path &src, const fs::path &base)
{
#ifdef STD_FILESYSTEM_EXPERIMENTAL
    fs::path tmpPath = src;
    fs::path relPath;
    while (fs::absolute(tmpPath) != fs::absolute(base)) {
        relPath = relPath.empty() ? tmpPath.filename() : tmpPath.filename() / relPath;
        if (tmpPath == tmpPath.parent_path()) {
            return "";
        }
        tmpPath = tmpPath.parent_path();
    }
    return relPath;
#else
    return fs::relative(src, base);
#endif
}

bool IterateModuleSections(
    ModuleIterateData &data, const std::function<size_t(ModuleIterateData *)> &requestIdxSectionModifier,
    const std::function<std::pair<size_t, size_t>(ModuleIterateData *, size_t, size_t)> &sectionModifier,
    const std::function<bool(ModuleIterateData *, std::pair<size_t, size_t>)> &postAction)
{
    auto idx = requestIdxSectionModifier(&data);

    data.isRegularImport = {true, true};
    auto r = sectionModifier(&data, idx, 3U);
    if (!postAction(&data, r)) {
        return false;
    }
    idx = r.first;
    data.payload->regularImportsOffset = r.second;

    data.isRegularImport = {true, false};
    r = sectionModifier(&data, idx, 2U);
    if (!postAction(&data, r)) {
        return false;
    }
    idx = r.first;
    data.payload->namespaceImportsOffset = r.second;

    data.isRegularImport = {false, false};
    data.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    r = sectionModifier(&data, idx, 2U);
    if (!postAction(&data, r)) {
        return false;
    }
    idx = r.first;
    data.payload->localExportsOffset = r.second;

    data.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT;
    r = sectionModifier(&data, idx, 3U);
    if (!postAction(&data, r)) {
        return false;
    }
    idx = r.first;
    data.payload->indirectExportsOffset = r.second;

    data.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT;
    r = sectionModifier(&data, idx, 1);
    if (!postAction(&data, r)) {
        return false;
    }
    data.payload->starExportsOffset = r.second;

    return true;
}

void DumpModuleRequests(const pandasm::LiteralArray *moduleLitArr, size_t &idx, std::stringstream &ss)
{
    auto numModuleRequests = std::get<uint32_t>(moduleLitArr->literals_[idx++].value_);
    auto moduleRequestsOffset = idx;
    ss << "\n    numModuleRequests " << numModuleRequests << " | offset " << idx << std::endl;
    while (idx < moduleRequestsOffset + numModuleRequests) {
        ss << "        [\n";
        ss << "            module_request: " << std::get<std::string>(moduleLitArr->literals_[idx].value_) << std::endl;
        idx++;
        ss << "        ]\n";
    }
}

void DumpRegularImports(const pandasm::LiteralArray *moduleLitArr, size_t &idx, std::stringstream &ss)
{
    auto numRegularImports = std::get<uint32_t>(moduleLitArr->literals_[idx++].value_);
    auto regularImportsOffset = idx;
    ss << "    numRegularImports " << numRegularImports << " | offset " << idx << std::endl;
    while (idx < regularImportsOffset + numRegularImports * 3U) {
        ss << "        [\n";
        ss << "            localNameOffset: " << std::get<std::string>(moduleLitArr->literals_[idx].value_)
           << std::endl;
        idx++;
        ss << "            importNameOffset: " << std::get<std::string>(moduleLitArr->literals_[idx].value_)
           << std::endl;
        idx++;
        ss << "            moduleRequestIdx: " << std::get<uint16_t>(moduleLitArr->literals_[idx].value_) << std::endl;
        idx++;
        ss << "        ]\n";
    }
}

void DumpNamespaceImports(const pandasm::LiteralArray *moduleLitArr, size_t &idx, std::stringstream &ss)
{
    auto numNamespaceImports = std::get<uint32_t>(moduleLitArr->literals_[idx++].value_);
    auto namespaceImportsOffset = idx;
    ss << "    numNamespaceImports " << numNamespaceImports << " | offset " << idx << std::endl;
    while (idx < namespaceImportsOffset + numNamespaceImports * 2U) {
        ss << "        [\n";
        ss << "            localNameOffset: " << std::get<std::string>(moduleLitArr->literals_[idx].value_)
           << std::endl;
        idx++;
        ss << "            moduleRequestIdx: " << std::get<uint16_t>(moduleLitArr->literals_[idx].value_) << std::endl;
        idx++;
        ss << "        ]\n";
    }
}

void DumpLocalExports(const pandasm::LiteralArray *moduleLitArr, size_t &idx, std::stringstream &ss)
{
    auto numLocalExports = std::get<uint32_t>(moduleLitArr->literals_[idx++].value_);
    auto localExportsOffset = idx;
    ss << "    numLocalExports " << numLocalExports << " | offset " << idx << std::endl;
    while (idx < localExportsOffset + numLocalExports * 2U) {
        ss << "        [\n";
        ss << "            localNameOffset: " << std::get<std::string>(moduleLitArr->literals_[idx].value_)
           << std::endl;
        idx++;
        ss << "            exportNameOffset: " << std::get<std::string>(moduleLitArr->literals_[idx].value_)
           << std::endl;
        idx++;
        ss << "        ]\n";
    }
}

void DumpIndirectExports(const pandasm::LiteralArray *moduleLitArr, size_t &idx, std::stringstream &ss)
{
    auto numIndirectExports = std::get<uint32_t>(moduleLitArr->literals_[idx++].value_);
    auto indirectExportsOffset = idx;
    ss << "    numIndirectExports " << numIndirectExports << " | offset " << idx << std::endl;
    while (idx < indirectExportsOffset + numIndirectExports * 3U) {
        ss << "        [\n";
        ss << "            exportNameOffset: " << std::get<std::string>(moduleLitArr->literals_[idx].value_)
           << std::endl;
        idx++;
        ss << "            importNameOffset: " << std::get<std::string>(moduleLitArr->literals_[idx].value_)
           << std::endl;
        idx++;
        ss << "            moduleRequestIdx: " << std::get<uint16_t>(moduleLitArr->literals_[idx].value_) << std::endl;
        idx++;
        ss << "        ]\n";
    }
}

void DumpStarExports(const pandasm::LiteralArray *moduleLitArr, size_t &idx, std::stringstream &ss)
{
    auto numStarExports = std::get<uint32_t>(moduleLitArr->literals_[idx++].value_);
    auto starExportsOffset = idx;
    ss << "    numStarExports " << numStarExports << " | offset " << idx << std::endl;
    while (idx < starExportsOffset + numStarExports) {
        ss << "        [\n";
        ss << "          moduleRequestIdx: " << std::get<uint16_t>(moduleLitArr->literals_[idx].value_) << std::endl;
        idx++;
        ss << "        ]\n";
    }
    ss << "]\n";
}

void DumpModuleArray(const pandasm::LiteralArray *moduleLitArr, std::string_view name)
{
    std::stringstream ss;
    ss << "ModuleLiteralArray " << name << " \n[";
    size_t idx = 0;

    DumpModuleRequests(moduleLitArr, idx, ss);
    DumpRegularImports(moduleLitArr, idx, ss);
    DumpNamespaceImports(moduleLitArr, idx, ss);
    DumpLocalExports(moduleLitArr, idx, ss);
    DumpIndirectExports(moduleLitArr, idx, ss);
    DumpStarExports(moduleLitArr, idx, ss);
    LIBABCKIT_LOG(DEBUG) << ss.str();
}

bool IsServiceRecord(const std::string &name)
{
    constexpr const std::string_view TSTYPE_ANNO_RECORD_NAME = "_TestAnnotation";
    return name == TSTYPE_ANNO_RECORD_NAME || name == "_ESConcurrentModuleRequestsAnnotation" ||
           name == "L_ESSlotNumberAnnotation" || name == "_ESSlotNumberAnnotation" ||
           name == "_ESExpectedPropertyCountAnnotation";
}

bool IsModuleDescriptorRecord(const pandasm::Record &rec)
{
    constexpr std::array<std::string_view, 2> REQUIRED_FIELDS = {
        "moduleRecordIdx",
        "scopeNames",
    };

    for (const auto &fieldName : REQUIRED_FIELDS) {
        auto it = std::find_if(rec.field_list.begin(), rec.field_list.end(),
                               [&fieldName](const auto &field) { return fieldName == field.name; });
        if (it == rec.field_list.end()) {
            return false;
        }
    }

    return true;
}

bool IsAnnotationInterfaceRecord(const pandasm::Record &rec)
{
    return (rec.metadata->GetAccessFlags() & ACC_ANNOTATION) != 0;
}

bool IsExternalRecord(const pandasm::Record &rec)
{
    return rec.metadata->IsForeign();
}

bool IsMain(const std::string &funcName)
{
    size_t dotPos = funcName.rfind('.');
    ASSERT(dotPos != std::string::npos);
    return funcName.substr(dotPos + 1) == "func_main_0";
}

bool IsFunction(const std::string &fullFuncName)
{
    if (IsMain(fullFuncName)) {
        return true;
    }
    size_t dotPos = fullFuncName.rfind('.');
    ASSERT(dotPos != std::string::npos);
    std::string funcName = fullFuncName.substr(dotPos);
    for (auto &sub : {"<#", ">#", "=#", "*#"}) {
        if (funcName.find(sub) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool IsCtor(const std::string &funcName)
{
    return funcName.find("=#") != std::string::npos;
}

bool IsStatic(const std::string &funcName)
{
    if (IsMain(funcName)) {
        return true;
    }
    if (funcName.find("<#") != std::string::npos) {
        return true;
    }
    return funcName.find("*#") != std::string::npos;
}

bool IsAnonymousName(const std::string &funcName)
{
    if (IsMain(funcName)) {
        return false;
    }
    size_t pos = funcName.rfind('#');
    ASSERT(pos != std::string::npos);
    std::string name = funcName.substr(pos + 1);
    return name.empty() || name.find('^') == 0;
}

static std::string DemangleScopeName(const std::string &mangled, const AbckitLiteralArray *scopeNames)
{
    const int base = 16;
    size_t scopeIdx = stoi(mangled, nullptr, base);
    auto *litArr = scopeNames->GetDynamicImpl();
    ASSERT(litArr->literals_.size() % 2U == 0);
    ASSERT(scopeIdx < litArr->literals_.size());
    auto &lit = litArr->literals_[scopeIdx * 2 + 1];
    ASSERT(lit.tag_ == panda_file::LiteralTag::STRING);
    return std::get<std::string>(lit.value_);
}

panda::pandasm::Function *GetDynFunction(AbckitCoreFunction *function)
{
    switch (function->owningModule->target) {
        case ABCKIT_TARGET_JS:
            return function->GetJsImpl()->impl;
        case ABCKIT_TARGET_ARK_TS_V1:
            return function->GetArkTSImpl()->GetDynamicImpl();
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

panda::pandasm::Function *GetDynFunction(AbckitCoreClass *klass)
{
    switch (klass->owningModule->target) {
        case ABCKIT_TARGET_JS:
            return klass->GetJsImpl()->impl;
        case ABCKIT_TARGET_ARK_TS_V1:
            return klass->GetArkTSImpl()->impl.GetDynamicClass();
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

AbckitModulePayloadDyn *GetDynModulePayload(AbckitCoreModule *mod)
{
    switch (mod->target) {
        case ABCKIT_TARGET_JS:
            return &mod->GetJsImpl()->impl;
        case ABCKIT_TARGET_ARK_TS_V1:
            return &mod->GetArkTSImpl()->impl.GetDynModule();
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

AbckitDynamicImportDescriptorPayload *GetDynImportDescriptorPayload(AbckitCoreImportDescriptor *id)
{
    switch (id->importingModule->target) {
        case ABCKIT_TARGET_JS:
            return &id->GetJsImpl()->payload.GetDynId();
        case ABCKIT_TARGET_ARK_TS_V1:
            return &id->GetArkTSImpl()->payload.GetDynId();
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

AbckitDynamicExportDescriptorPayload *GetDynExportDescriptorPayload(AbckitCoreExportDescriptor *ed)
{
    AbckitDynamicExportDescriptorPayload *edPayload = nullptr;
    switch (ed->exportingModule->target) {
        case ABCKIT_TARGET_JS:
            edPayload = &ed->GetJsImpl()->payload.GetDynamicPayload();
            break;
        case ABCKIT_TARGET_ARK_TS_V1:
            edPayload = &ed->GetArkTSImpl()->payload.GetDynamicPayload();
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }
    return edPayload;
}

std::string GetClassNameFromCtor(const std::string &ctorName, const AbckitLiteralArray *scopeNames)
{
    ASSERT(IsCtor(ctorName));
    auto className = ctorName.substr(ctorName.find('~') + 1);
    className = className.substr(0, className.find("=#"));
    if (className[0] != '@') {
        return className;
    }
    return DemangleScopeName(className.substr(1), scopeNames);
}

template <class T>
static AbckitLiteral *FindOrCreateLiteral(AbckitFile *file,
                                          std::unordered_map<T, std::unique_ptr<AbckitLiteral>> &cache, T value,
                                          panda_file::LiteralTag tagImpl)
{
    if (cache.count(value) == 1) {
        return cache[value].get();
    }

    auto literalDeleter = [](pandasm_Literal *p) -> void {
        delete reinterpret_cast<pandasm::LiteralArray::Literal *>(p);
    };

    auto *literal = new pandasm::LiteralArray::Literal();
    literal->tag_ = tagImpl;
    literal->value_ = value;
    auto abcLit = std::make_unique<AbckitLiteral>(
        file, AbckitLiteralImplT(reinterpret_cast<pandasm_Literal *>(literal), literalDeleter));
    cache.insert({value, std::move(abcLit)});
    return cache[value].get();
}

AbckitLiteral *FindOrCreateLiteralBoolDynamicImpl(AbckitFile *file, bool value)
{
    return FindOrCreateLiteral(file, file->literals.boolLits, value, panda_file::LiteralTag::BOOL);
}

AbckitLiteral *FindOrCreateLiteralU8DynamicImpl(AbckitFile *file, uint8_t value)
{
    return FindOrCreateLiteral(file, file->literals.u8Lits, value, panda_file::LiteralTag::ARRAY_U8);
}

AbckitLiteral *FindOrCreateLiteralU16DynamicImpl(AbckitFile *file, uint16_t value)
{
    return FindOrCreateLiteral(file, file->literals.u16Lits, value, panda_file::LiteralTag::ARRAY_U16);
}

AbckitLiteral *FindOrCreateLiteralMethodAffiliateDynamicImpl(AbckitFile *file, uint16_t value)
{
    return FindOrCreateLiteral(file, file->literals.methodAffilateLits, value, panda_file::LiteralTag::METHODAFFILIATE);
}

AbckitLiteral *FindOrCreateLiteralU32DynamicImpl(AbckitFile *file, uint32_t value)
{
    return FindOrCreateLiteral(file, file->literals.u32Lits, value, panda_file::LiteralTag::INTEGER);
}

AbckitLiteral *FindOrCreateLiteralU64DynamicImpl(AbckitFile *file, uint64_t value)
{
    return FindOrCreateLiteral(file, file->literals.u64Lits, value, panda_file::LiteralTag::ARRAY_U64);
}

AbckitLiteral *FindOrCreateLiteralFloatDynamicImpl(AbckitFile *file, float value)
{
    return FindOrCreateLiteral(file, file->literals.floatLits, value, panda_file::LiteralTag::FLOAT);
}

AbckitLiteral *FindOrCreateLiteralDoubleDynamicImpl(AbckitFile *file, double value)
{
    return FindOrCreateLiteral(file, file->literals.doubleLits, value, panda_file::LiteralTag::DOUBLE);
}

AbckitLiteral *FindOrCreateLiteralLiteralArrayDynamicImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateLiteral(file, file->literals.litArrLits, value, panda_file::LiteralTag::LITERALARRAY);
}

AbckitLiteral *FindOrCreateLiteralStringDynamicImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateLiteral(file, file->literals.stringLits, value, panda_file::LiteralTag::STRING);
}

AbckitLiteral *FindOrCreateLiteralMethodDynamicImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateLiteral(file, file->literals.methodLits, value, panda_file::LiteralTag::METHOD);
}

AbckitLiteral *FindOrCreateLiteralNullValueDynamicImpl(AbckitFile *file)
{
    if (file->literals.nullValueLit != nullptr) {
        return file->literals.nullValueLit.get();
    }

    auto literalDeleter = [](pandasm_Literal *p) -> void {
        delete reinterpret_cast<pandasm::LiteralArray::Literal *>(p);
    };

    // According to assembly-literals.h, NULLVALUE uses uint8_t (IsByteValue() returns true)
    // The emitter expects std::get<uint8_t>(literal.value_) for NULLVALUE
    auto *literal = new pandasm::LiteralArray::Literal();
    literal->tag_ = panda_file::LiteralTag::NULLVALUE;
    literal->value_ = uint8_t {0};
    auto abcLit = std::make_unique<AbckitLiteral>(
        file, AbckitLiteralImplT(reinterpret_cast<pandasm_Literal *>(literal), literalDeleter));
    file->literals.nullValueLit = std::move(abcLit);
    return file->literals.nullValueLit.get();
}

AbckitLiteral *FindOrCreateLiteralDynamic(AbckitFile *file, const pandasm::LiteralArray::Literal &value)
{
    switch (value.tag_) {
        case panda_file::LiteralTag::ARRAY_U1:
        case panda_file::LiteralTag::BOOL:
            return FindOrCreateLiteralBoolDynamicImpl(file, std::get<bool>(value.value_));
        case panda_file::LiteralTag::ARRAY_U8:
        case panda_file::LiteralTag::ARRAY_I8:
            return FindOrCreateLiteralU8DynamicImpl(file, std::get<uint8_t>(value.value_));
        case panda_file::LiteralTag::ARRAY_U16:
        case panda_file::LiteralTag::ARRAY_I16:
            return FindOrCreateLiteralU16DynamicImpl(file, std::get<uint16_t>(value.value_));
        case panda_file::LiteralTag::ARRAY_U32:
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::INTEGER:
            return FindOrCreateLiteralU32DynamicImpl(file, std::get<uint32_t>(value.value_));
        case panda_file::LiteralTag::ARRAY_U64:
        case panda_file::LiteralTag::ARRAY_I64:
            return FindOrCreateLiteralU64DynamicImpl(file, std::get<uint64_t>(value.value_));
        case panda_file::LiteralTag::ARRAY_F32:
        case panda_file::LiteralTag::FLOAT:
            return FindOrCreateLiteralFloatDynamicImpl(file, std::get<float>(value.value_));
        case panda_file::LiteralTag::ARRAY_F64:
        case panda_file::LiteralTag::DOUBLE:
            return FindOrCreateLiteralDoubleDynamicImpl(file, std::get<double>(value.value_));
        case panda_file::LiteralTag::ARRAY_STRING:
        case panda_file::LiteralTag::STRING:
            return FindOrCreateLiteralStringDynamicImpl(file, std::get<std::string>(value.value_));
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GETTER:
        case panda_file::LiteralTag::SETTER:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
            return FindOrCreateLiteralMethodDynamicImpl(file, std::get<std::string>(value.value_));
        case panda_file::LiteralTag::METHODAFFILIATE:
            return FindOrCreateLiteralMethodAffiliateDynamicImpl(file, std::get<uint16_t>(value.value_));
        case panda_file::LiteralTag::LITERALARRAY:
            return FindOrCreateLiteralLiteralArrayDynamicImpl(file, std::get<std::string>(value.value_));
        case panda_file::LiteralTag::NULLVALUE:
            return FindOrCreateLiteralNullValueDynamicImpl(file);
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

template <class T, pandasm::Value::Type PANDASM_VALUE_TYPE>
static AbckitValue *FindOrCreateScalarValue(AbckitFile *file,
                                            std::unordered_map<T, std::unique_ptr<AbckitValue>> &cache, T value)
{
    if (cache.count(value) == 1) {
        return cache[value].get();
    }

    auto valueDeleter = [](pandasm_Value *val) -> void { delete reinterpret_cast<panda::pandasm::ScalarValue *>(val); };

    auto *pval = new pandasm::ScalarValue(pandasm::ScalarValue::Create<PANDASM_VALUE_TYPE>(value));
    auto abcVal =
        std::make_unique<AbckitValue>(file, AbckitValueImplT(reinterpret_cast<pandasm_Value *>(pval), valueDeleter));
    cache.insert({value, std::move(abcVal)});
    return cache[value].get();
}

AbckitValue *FindOrCreateValueU1DynamicImpl(AbckitFile *file, bool value)
{
    return FindOrCreateScalarValue<bool, panda::pandasm::Value::Type::U1>(file, file->values.boolVals, value);
}

AbckitValue *FindOrCreateValueIntDynamicImpl(AbckitFile *file, int value)
{
    return FindOrCreateScalarValue<int, panda::pandasm::Value::Type::I32>(file, file->values.intVals, value);
}

AbckitValue *FindOrCreateValueDoubleDynamicImpl(AbckitFile *file, double value)
{
    return FindOrCreateScalarValue<double, panda::pandasm::Value::Type::F64>(file, file->values.doubleVals, value);
}

AbckitValue *FindOrCreateValueStringDynamicImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateScalarValue<std::string, panda::pandasm::Value::Type::STRING>(file, file->values.stringVals,
                                                                                     value);
}

AbckitValue *FindOrCreateLiteralArrayValueDynamicImpl(AbckitFile *file, const std::string &value)
{
    return FindOrCreateScalarValue<std::string, panda::pandasm::Value::Type::LITERALARRAY>(
        file, file->values.litarrVals, value);
}

AbckitValue *FindOrCreateValueDynamic(AbckitFile *file, const pandasm::Value &value)
{
    switch (value.GetType()) {
        case pandasm::Value::Type::U1:
            return FindOrCreateValueU1DynamicImpl(file, value.GetAsScalar()->GetValue<bool>());
        case pandasm::Value::Type::F64:
            return FindOrCreateValueDoubleDynamicImpl(file, value.GetAsScalar()->GetValue<double>());
        case pandasm::Value::Type::STRING:
            return FindOrCreateValueStringDynamicImpl(file, value.GetAsScalar()->GetValue<std::string>());
        case pandasm::Value::Type::LITERALARRAY:
            return FindOrCreateLiteralArrayValueDynamicImpl(file, value.GetAsScalar()->GetValue<std::string>());
        case pandasm::Value::Type::I8:
        case pandasm::Value::Type::U8:
        case pandasm::Value::Type::I16:
        case pandasm::Value::Type::U16:
        case pandasm::Value::Type::I32:
            return FindOrCreateValueIntDynamicImpl(file, value.GetAsScalar()->GetValue<int>());
        case pandasm::Value::Type::U32:
        case pandasm::Value::Type::I64:
        case pandasm::Value::Type::U64:
        case pandasm::Value::Type::F32:
        case pandasm::Value::Type::STRING_NULLPTR:
        case pandasm::Value::Type::RECORD:
        case pandasm::Value::Type::METHOD:
        case pandasm::Value::Type::ENUM:
        case pandasm::Value::Type::ANNOTATION:
        case pandasm::Value::Type::ARRAY:
        case pandasm::Value::Type::VOID:
        case pandasm::Value::Type::METHOD_HANDLE:
        case pandasm::Value::Type::UNKNOWN:
        default:
            LIBABCKIT_UNREACHABLE;
    }
}

}  // namespace libabckit
