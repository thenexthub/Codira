/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "function_util.h"
#include "libabckit/c/statuses.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/adapter_static/helpers_static.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"
#include "libabckit/src/adapter_static/string_util.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "static_core/assembler/assembly-program.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace abckit::util {

// update lambda record annotations (FunctionalReference) when modify a function's mangle name
static bool UpdateLambdaAnnotationsForSignatureChange(ark::pandasm::Program *prog, const std::string &oldMangleName,
                                                      const std::string &newMangleName)
{
    constexpr const char *LAMBDA_RECORD_KEY = "%%lambda-";
    constexpr const char *FUNCTIONAL_REFERENCE_ANNO = "ets.annotation.FunctionalReference";
    constexpr const char *ELEMENT_NAME = "value";

    if (oldMangleName == newMangleName) {
        return true;
    }

    for (auto &[recordName, record] : prog->recordTable) {
        std::string recordNameCopy = recordName;

        if (recordName.find(LAMBDA_RECORD_KEY) == std::string::npos) {
            continue;
        }

        if (record.metadata == nullptr) {
            continue;
        }

        bool needsUpdate = false;
        std::string newMethodValue;
        bool isStatic = false;

        record.metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
            if (anno.GetName() != FUNCTIONAL_REFERENCE_ANNO) {
                return;
            }

            for (const auto &elem : anno.GetElements()) {
                if (elem.GetName() != ELEMENT_NAME) {
                    continue;
                }

                auto *value = elem.GetValue();
                if (value->GetType() != ark::pandasm::Value::Type::METHOD) {
                    continue;
                }

                auto *scalarValue = value->GetAsScalar();
                std::string methodValue = scalarValue->GetValue<std::string>();

                // in annotations the method value may contain additional prefixes
                // (e.g. "<static> "), so replace the old mangle name as a substring
                auto pos = methodValue.find(oldMangleName);
                if (pos == std::string::npos) {
                    continue;
                }

                newMethodValue = methodValue;
                newMethodValue.replace(pos, oldMangleName.size(), newMangleName);
                isStatic = scalarValue->IsStatic();
                needsUpdate = true;

                LIBABCKIT_LOG_NO_FUNC(DEBUG) << "Updating FunctionalReference in lambda record " << recordNameCopy
                                             << " from " << methodValue << " to " << newMethodValue << std::endl;
                return;
            }
        });

        if (!needsUpdate) {
            continue;
        }

        auto *metadata = record.metadata.get();
        if (metadata == nullptr) {
            continue;
        }

        metadata->DeleteAnnotationElementByName(FUNCTIONAL_REFERENCE_ANNO, ELEMENT_NAME);

        auto newVal = std::make_unique<ark::pandasm::ScalarValue>(
            ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::METHOD>(newMethodValue, isStatic));
        ark::pandasm::AnnotationElement newElement(ELEMENT_NAME, std::move(newVal));
        metadata->AddAnnotationElementByName(FUNCTIONAL_REFERENCE_ANNO, std::move(newElement));
    }

    return true;
}

std::string ReplaceFunctionModuleName(const std::string &oldModuleName, const std::string &newName)
{
    size_t pos = oldModuleName.rfind('.');
    if (pos == std::string::npos) {
        libabckit::statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return newName;
    }
    return oldModuleName.substr(0, pos + 1) + newName;
}

std::string GenerateFunctionMangleName(const std::string &moduleName, const ark::pandasm::Function &func)
{
    std::ostringstream ss;
    ss << moduleName << ":";

    for (const auto &param : func.params) {
        ss << param.type.GetName() << ";";
    }
    ss << func.returnType.GetName() << ";";
    return ss.str();
}

static ark::pandasm::Function *UpdateTableKey(std::map<std::string, ark::pandasm::Function> &table,
                                              const std::string &oldKey, const std::string &newKey,
                                              const std::string &tableName)
{
    auto it = table.find(oldKey);
    if (it == table.end()) {
        return nullptr;
    }

    if (table.find(newKey) != table.end()) {
        LIBABCKIT_LOG(FATAL) << "Target mangle name already exists in " << tableName << ": " << newKey << std::endl;
        return nullptr;
    }

    auto node = table.extract(it);
    node.key() = newKey;
    auto result = table.insert(std::move(node));

    LIBABCKIT_LOG(DEBUG) << "Updated function in " << tableName << ": " << oldKey << " -> " << newKey << std::endl;
    return &result.position->second;
}

bool UpdateFunctionTableKey(ark::pandasm::Program *prog, ark::pandasm::Function *impl, const std::string &newName,
                            std::string &oldMangleName, std::string &newMangleName, AbckitArktsFunction *arktsFunc)
{
    // Generate new mangle name if not provided
    if (newMangleName.empty()) {
        std::string newModuleName = ReplaceFunctionModuleName(impl->name, newName);
        newMangleName = GenerateFunctionMangleName(newModuleName, *impl);
    }

    LIBABCKIT_LOG(DEBUG) << "Updating function key: " << oldMangleName << " -> " << newMangleName << std::endl;

    // Try tables in priority order: static first, then instance
    ark::pandasm::Function *newImpl =
        UpdateTableKey(prog->functionStaticTable, oldMangleName, newMangleName, "static table");
    if (newImpl == nullptr) {
        newImpl = UpdateTableKey(prog->functionInstanceTable, oldMangleName, newMangleName, "instance table");
    }

    if (newImpl == nullptr) {
        LIBABCKIT_LOG(FATAL) << "Function table key not found or update failed: " << oldMangleName << std::endl;
        return false;
    }

    if (arktsFunc) {
        auto *oldImpl = arktsFunc->GetStaticImpl();
        if (oldImpl != newImpl) {
            arktsFunc->impl = newImpl;
            LIBABCKIT_LOG(DEBUG) << "Updated AbckitArktsFunction impl pointer (address changed)" << std::endl;
        }
    }

    // also refresh lambda record annotations (FunctionalReference) that reference this function by its mangle name.
    if (!UpdateLambdaAnnotationsForSignatureChange(prog, oldMangleName, newMangleName)) {
        LIBABCKIT_LOG(ERROR) << "Failed to update lambda FunctionalReference annotations for " << oldMangleName
                             << " -> " << newMangleName << std::endl;
        return false;
    }

    return true;
}

static void ReplaceIdsInInsList(std::vector<ark::pandasm::Ins> &insList, const std::string &oldId,
                                const std::string &newId)
{
    for (auto &insn : insList) {
        for (std::size_t i = 0; i < insn.IDSize(); ++i) {
            if (insn.GetID(i) == oldId) {
                insn.SetID(i, newId);
            }
        }
    }
}

static void ReplaceIdsInFunctionTable(std::map<std::string, ark::pandasm::Function> &functionTable,
                                      const std::string &oldId, const std::string &newId)
{
    for (auto &[key, func] : functionTable) {
        ReplaceIdsInInsList(func.ins, oldId, newId);
    }
}

void ReplaceInstructionIds(ark::pandasm::Program *prog, ark::pandasm::Function *impl, const std::string &oldId,
                           const std::string &newId)
{
    ReplaceIdsInInsList(impl->ins, oldId, newId);
    ReplaceIdsInFunctionTable(prog->functionStaticTable, oldId, newId);
    ReplaceIdsInFunctionTable(prog->functionInstanceTable, oldId, newId);
}

static std::string GetReferenceTypeName(const AbckitType *type)
{
    if (!type->types.empty()) {
        return libabckit::StringUtil::GetTypeNameStr(type);
    }
    auto *record = type->GetClass()->GetArkTSImpl()->impl.GetStaticClass();
    auto [moduleName, className] = libabckit::ClassGetNames(record->name);
    LIBABCKIT_LOG(DEBUG) << "parsed moduleName: " << moduleName << ", className: " << className << std::endl;
    LIBABCKIT_LOG(DEBUG) << moduleName + "." + className << std::endl;
    return moduleName + "." + className;
}

static std::string GetPrimitiveTypeName(AbckitTypeId typeId, bool useComponentFormat)
{
    static const std::map<AbckitTypeId, std::pair<std::string, std::string>> TYPE_MAP = {
        {ABCKIT_TYPE_ID_STRING, {"std.core.String", "std.core.String"}},
        {ABCKIT_TYPE_ID_U1, {"std.core.u1", "u1"}},
        {ABCKIT_TYPE_ID_U8, {"std.core.u8", "u8"}},
        {ABCKIT_TYPE_ID_I8, {"std.core.i8", "i8"}},
        {ABCKIT_TYPE_ID_U16, {"std.core.u16", "u16"}},
        {ABCKIT_TYPE_ID_I16, {"std.core.i16", "i16"}},
        {ABCKIT_TYPE_ID_U32, {"std.core.u32", "u32"}},
        {ABCKIT_TYPE_ID_I32, {"std.core.i32", "i32"}},
        {ABCKIT_TYPE_ID_U64, {"std.core.u64", "u64"}},
        {ABCKIT_TYPE_ID_I64, {"std.core.i64", "i64"}},
        {ABCKIT_TYPE_ID_F32, {"std.core.f32", "f32"}},
        {ABCKIT_TYPE_ID_F64, {"std.core.f64", "f64"}},
        {ABCKIT_TYPE_ID_ANY, {"std.core.any", "any"}},
        {ABCKIT_TYPE_ID_VOID, {"void", "void"}}};

    auto it = TYPE_MAP.find(typeId);
    if (it != TYPE_MAP.end()) {
        return useComponentFormat ? it->second.first : it->second.second;
    }
    LIBABCKIT_LOG(ERROR) << "[Error] Invalid type id: " << typeId << std::endl;
    return "invalid";
}

std::string GetTypeName(const AbckitType *type, bool useComponentFormat)
{
    if (type->id == ABCKIT_TYPE_ID_REFERENCE) {
        return GetReferenceTypeName(type);
    }
    return GetPrimitiveTypeName(type->id, useComponentFormat);
}

void AddFunctionParameterImpl(AbckitCoreFunction *coreFunc, ark::pandasm::Function *funcImpl,
                              const AbckitCoreFunctionParam *paramCore, const std::string &componentName)
{
    ark::pandasm::Type type(componentName, paramCore->type->rank);
    ark::pandasm::Function::Parameter pandaParam(type, ark::panda_file::SourceLang::ETS);
    funcImpl->params.push_back(std::move(pandaParam));

    auto paramHolder = std::make_unique<AbckitCoreFunctionParam>();
    paramHolder->function = coreFunc;
    paramHolder->type = paramCore->type;
    paramHolder->name = paramCore->name;
    paramHolder->defaultValue = paramCore->defaultValue;

    auto arktsParam = std::make_unique<AbckitArktsFunctionParam>();
    arktsParam->core = paramHolder.get();
    paramHolder->impl = std::move(arktsParam);

    coreFunc->parameters.emplace_back(std::move(paramHolder));
}

bool RemoveFunctionParameterByIndexImpl(AbckitCoreFunction *coreFunc, ark::pandasm::Function *funcImpl, size_t index)
{
    if (index >= funcImpl->params.size()) {
        LIBABCKIT_LOG(ERROR) << "Index " << index << " exceeds impl->params.size(): " << funcImpl->params.size()
                             << std::endl;
        libabckit::statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return false;
    }

    funcImpl->params.erase(funcImpl->params.begin() + index);
    coreFunc->parameters.erase(coreFunc->parameters.begin() + index);
    return true;
}

bool UpdateFileMap(AbckitFile *file, const std::string &oldMangleName, const std::string &newMangleName)
{
    if (file == nullptr) {
        LIBABCKIT_LOG(ERROR) << "File is null" << std::endl;
        return false;
    }
    bool found = false;

    auto staticIt = file->nameToFunctionStatic.find(oldMangleName);
    if (staticIt != file->nameToFunctionStatic.end()) {
        auto staticNode = file->nameToFunctionStatic.extract(staticIt);
        staticNode.key() = newMangleName;
        file->nameToFunctionStatic.insert(std::move(staticNode));
        found = true;
    }

    auto instanceIt = file->nameToFunctionInstance.find(oldMangleName);
    if (instanceIt != file->nameToFunctionInstance.end()) {
        auto instanceNode = file->nameToFunctionInstance.extract(instanceIt);
        instanceNode.key() = newMangleName;
        file->nameToFunctionInstance.insert(std::move(instanceNode));
        found = true;
    }

    if (!found) {
        LIBABCKIT_LOG(ERROR) << "Function mangle name not found in file maps: " << oldMangleName << std::endl;
        return false;
    }

    return true;
}
}  // namespace abckit::util