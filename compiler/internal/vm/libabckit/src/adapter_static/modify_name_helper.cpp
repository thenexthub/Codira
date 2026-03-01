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
#include "modify_name_helper.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <regex>
#include "libabckit/src/logger.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_static/metadata_modify_static.h"
#include "static_core/assembler/assembly-program.h"
#include "static_core/assembler/mangling.h"
#include "metadata_inspect_static.h"
#include "helpers_static.h"
#include "name_util.h"
#include "string_util.h"

namespace {
constexpr std::string_view ETS_EXTENDS = "ets.extends";
constexpr std::string_view ETS_IMPLEMENTS = "ets.implements";
constexpr std::string_view GLOBAL_CLASS = "ETSGLOBAL";
constexpr std::string_view NAME_DELIMITER = ".";
constexpr std::string_view FUNCTION_DELIMITER = ":";
constexpr std::string_view INTERFACE_FIELD_PREFIX = "%%property-";
constexpr std::string_view ASYNC_PATTERN = "(%%async-)(.+):(.+)";
constexpr std::string_view ASYNC_PREFIX = "%%async-";
constexpr std::string_view GETTER_PREFIX = "%%get-";
constexpr std::string_view SETTER_PREFIX = "%%set-";
constexpr auto ARRAY_SUFFIX = "[]";

struct AnnotationUpdateData {
    std::string oldName;
    std::string newName;
};

// Forward declarations
template <typename MetadataType>
static bool UpdateAnnotationElementValue(MetadataType *metadata, const std::string &annoName,
                                         const std::string &elementName,
                                         std::unique_ptr<ark::pandasm::ScalarValue> newValue);
static bool UpdateAnnotationElementValueMethod(ark::pandasm::ItemMetadata *metadata, const std::string &annoName,
                                               const std::string &elementName, const std::string &methodName,
                                               bool isStatic);
static bool UpdateAnnotationElementValueString(ark::pandasm::RecordMetadata *metadata, const std::string &annoName,
                                               const std::string &elementName, const std::string &newValue);
static bool UpdateAnnotationElementValueRecord(ark::pandasm::RecordMetadata *metadata, const std::string &annoName,
                                               const std::string &elementName, const std::string &recordName);
static bool FunctionRefreshLambdaAnnotationElements(AbckitCoreFunction *function, const std::string &oldFunctionKeyName,
                                                    const std::string &newFunctionKeyName);
static bool FunctionRefreshAsyncAnnotationElements(AbckitCoreFunction *function, const std::string &oldFunctionKeyName,
                                                   const std::string &newFunctionKeyName);
static bool FunctionRefreshFunctionOverloadAnnotationElements(AbckitCoreFunction *function,
                                                              const std::string &oldFunctionKeyName,
                                                              const std::string &newFunctionKeyName);

static std::string ExtractSimpleFunctionName(const std::string &keyName);
static bool ProcessAsyncAnnotationsInFunctionTable(std::map<std::string, ark::pandasm::Function> &functionTable,
                                                   const std::string &oldFunctionKeyName,
                                                   const std::string &newFunctionKeyName, bool isStatic);

std::string AddGetSetPrefix(const std::string &input, const std::string &name)
{
    if (input.find(GETTER_PREFIX) != std::string::npos) {
        return std::string(GETTER_PREFIX) + name;
    }
    if (input.find(SETTER_PREFIX) != std::string::npos) {
        return std::string(SETTER_PREFIX) + name;
    }
    return name;
}

std::string GetSetPattern(const std::string &name)
{
    return "%%([gs]et)-(" + name + "):(.+)";
}

std::string GetSetReplace(const std::string &name)
{
    return "%%$1-" + name + ":$3";
}

std::string AsyncReplace(const std::string &name)
{
    return "$1" + name + ":$3";
}

// Generic function to update annotation element value
template <typename MetadataType>
static bool UpdateAnnotationElementValue(MetadataType *metadata, const std::string &annoName,
                                         const std::string &elementName,
                                         std::unique_ptr<ark::pandasm::ScalarValue> newValue)
{
    if (metadata == nullptr) {
        return false;
    }

    metadata->DeleteAnnotationElementByName(annoName, elementName);
    ark::pandasm::AnnotationElement newElement(elementName, std::move(newValue));
    metadata->AddAnnotationElementByName(annoName, std::move(newElement));

    return true;
}

// Extract simple function name from key name format: "moduleName.Class.functionName:(params)returnType"
static std::string ExtractSimpleFunctionName(const std::string &keyName)
{
    auto colonPos = keyName.find(':');
    if (colonPos == std::string::npos) {
        return "";
    }
    auto nameWithModule = keyName.substr(0, colonPos);
    auto lastDotPos = nameWithModule.find_last_of('.');
    if (lastDotPos == std::string::npos) {
        return nameWithModule;
    }
    return nameWithModule.substr(lastDotPos + 1);
}

// Process async annotations in function table
static bool ProcessAsyncAnnotationsInFunctionTable(std::map<std::string, ark::pandasm::Function> &functionTable,
                                                   const std::string &oldFunctionKeyName,
                                                   const std::string &newFunctionKeyName, bool isStatic)
{
    const std::string ASYNC_ANNOTATION = "ets.coroutine.Async";
    const std::string ELEMENT_NAME = "value";

    for (auto &[functionKeyName, func] : functionTable) {
        if (func.metadata == nullptr) {
            continue;
        }

        std::string currentFunctionKeyName = functionKeyName;
        ark::pandasm::Function &currentFunc = func;

        func.metadata->EnumerateAnnotations([&, currentFunctionKeyName](ark::pandasm::AnnotationData &anno) {
            if (anno.GetName() != ASYNC_ANNOTATION) {
                return;
            }

            for (const auto &elem : anno.GetElements()) {
                if (elem.GetName() == ELEMENT_NAME) {
                    auto *value = elem.GetValue();
                    if (value != nullptr && value->GetType() == ark::pandasm::Value::Type::METHOD) {
                        auto *scalarValue = value->GetAsScalar();
                        if (scalarValue != nullptr) {
                            std::string methodValue = scalarValue->GetValue<std::string>();
                            if (methodValue == oldFunctionKeyName) {
                                LIBABCKIT_LOG(DEBUG)
                                    << "Found async function " << currentFunctionKeyName
                                    << " with Async annotation pointing to old function: " << oldFunctionKeyName
                                    << std::endl;

                                if (!UpdateAnnotationElementValueMethod(currentFunc.metadata.get(), ASYNC_ANNOTATION,
                                                                        ELEMENT_NAME, newFunctionKeyName, isStatic)) {
                                    LIBABCKIT_LOG(ERROR) << "Failed to update Async annotation element in "
                                                         << currentFunctionKeyName << std::endl;
                                    return;
                                }

                                LIBABCKIT_LOG(DEBUG)
                                    << "Updated Async annotation in " << currentFunctionKeyName << " from "
                                    << oldFunctionKeyName << " to " << newFunctionKeyName << std::endl;
                            }
                        }
                    }
                }
            }
        });
    }
    return true;
}

std::vector<std::string> Split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> tokens;
    auto start = str.find_first_not_of(delimiter, 0);
    auto pos = str.find_first_of(delimiter, 0);
    while (start != std::string::npos) {
        if (pos > start) {
            tokens.push_back(str.substr(start, pos - start));
        }
        start = str.find_first_not_of(delimiter, pos);
        pos = str.find_first_of(delimiter, start + 1);
    }
    ASSERT(!tokens.empty());

    return tokens;
}

bool ProgramUpdateRecordTableKey(ark::pandasm::Program *prog, const std::string &oldKeyName,
                                 const std::string &newKeyName)
{
    LIBABCKIT_LOG_FUNC;

    if (prog->recordTable.find(oldKeyName) == prog->recordTable.end()) {
        LIBABCKIT_LOG(ERROR) << "no record name:" << oldKeyName << std::endl;
        return false;
    }

    if (prog->recordTable.find(newKeyName) != prog->recordTable.end()) {
        LIBABCKIT_LOG(ERROR) << "duplicate record name:" << newKeyName << std::endl;
        return false;
    }

    auto entry = prog->recordTable.extract(oldKeyName);

    entry.key() = newKeyName;

    prog->recordTable.insert(std::move(entry));

    return true;
}

bool ProgramUpdateFunctionTableKey(ark::pandasm::Program *prog, const std::string &oldKeyName,
                                   const std::string &newKeyName)
{
    LIBABCKIT_LOG_FUNC;

    std::map<std::string, ark::pandasm::Function> *functionTable = nullptr;
    if (prog->functionStaticTable.find(oldKeyName) != prog->functionStaticTable.end()) {
        functionTable = &prog->functionStaticTable;
    } else if (prog->functionInstanceTable.find(oldKeyName) != prog->functionInstanceTable.end()) {
        functionTable = &prog->functionInstanceTable;
    } else {
        LIBABCKIT_LOG(ERROR) << "invalid function name:" << oldKeyName << std::endl;
        return false;
    }

    auto entry = functionTable->extract(oldKeyName);
    entry.key() = newKeyName;
    functionTable->insert(std::move(entry));
    return true;
}

bool ObjectRefreshName(std::variant<AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreEnum *, AbckitCoreInterface *,
                                    AbckitCoreAnnotationInterface *>
                           object,
                       const std::string &newName, bool isObjectLiteral = false, bool isPartial = false)
{
    LIBABCKIT_LOG_FUNC;
    const auto funcName = libabckit::StringUtil::GetFuncNameWithSquareBrackets(__func__);
    return std::visit(
        [newName, isObjectLiteral, isPartial, funcName](auto &coreObject) {
            auto record = libabckit::GetStaticImplRecord(coreObject);
            const auto oldFullName = record->name;
            const auto newFullName = libabckit::NameUtil::GetFullName(coreObject, newName, isObjectLiteral, isPartial);
            if (oldFullName == newFullName) {
                return true;
            }

            LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "old full name:" << oldFullName << std::endl;
            LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "new full name:" << newFullName << std::endl;

            if (!ProgramUpdateRecordTableKey(coreObject->owningModule->file->GetStaticProgram(), oldFullName,
                                             newFullName)) {
                LIBABCKIT_LOG_NO_FUNC(ERROR) << funcName << "Failed to update object program record" << std::endl;
                return false;
            }
            record->name = newFullName;

            return true;
        },
        object);
}

static bool UpdateAnnotationElementValueMethod(ark::pandasm::ItemMetadata *metadata, const std::string &annoName,
                                               const std::string &elementName, const std::string &methodName,
                                               bool isStatic)
{
    auto newVal = std::make_unique<ark::pandasm::ScalarValue>(
        ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::METHOD>(methodName, isStatic));
    return UpdateAnnotationElementValue(metadata, annoName, elementName, std::move(newVal));
}

// Update method array values in FunctionOverload annotation element
static bool UpdateFunctionOverloadArrayValues(ark::pandasm::ArrayValue *arrayValue,
                                              const std::string &oldFunctionKeyName,
                                              const std::string &newFunctionKeyName,
                                              std::vector<ark::pandasm::ScalarValue> &newValues)
{
    bool elementFound = false;
    auto &values = arrayValue->GetValues();

    for (const auto &val : values) {
        if (val.GetType() != ark::pandasm::Value::Type::METHOD) {
            newValues.push_back(val);
            continue;
        }

        auto *scalarValue = val.GetAsScalar();
        if (scalarValue == nullptr) {
            newValues.push_back(val);
            continue;
        }

        std::string methodName = scalarValue->GetValue<std::string>();
        bool isStatic = scalarValue->IsStatic();

        if (methodName == oldFunctionKeyName) {
            newValues.push_back(
                ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::METHOD>(newFunctionKeyName, isStatic));
            elementFound = true;
        } else {
            newValues.push_back(val);
        }
    }

    return elementFound;
}

static bool FunctionRefreshFunctionOverloadAnnotationElements(AbckitCoreFunction *function,
                                                              const std::string &oldFunctionKeyName,
                                                              const std::string &newFunctionKeyName)
{
    LIBABCKIT_LOG_FUNC;

    if (oldFunctionKeyName == newFunctionKeyName) {
        return true;
    }

    auto *prog = function->owningModule->file->GetStaticProgram();
    constexpr const char *FUNCTION_OVERLOAD_ANNOTATION = "ets.annotation.FunctionOverload";

    for (auto &[recordNameRef, record] : prog->recordTable) {
        if (record.metadata == nullptr) {
            continue;
        }

        std::string recordName = recordNameRef;

        record.metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
            if (anno.GetName() != FUNCTION_OVERLOAD_ANNOTATION) {
                return;
            }

            anno.EnumerateAnnotationElements([&](ark::pandasm::AnnotationElement &element) {
                auto *value = element.GetValue();
                if (value == nullptr || value->GetType() != ark::pandasm::Value::Type::ARRAY) {
                    return;
                }

                auto *arrayValue = static_cast<ark::pandasm::ArrayValue *>(value);
                if (arrayValue->GetComponentType() != ark::pandasm::Value::Type::METHOD) {
                    return;
                }
                std::vector<ark::pandasm::ScalarValue> newValues;
                bool elementFound =
                    UpdateFunctionOverloadArrayValues(arrayValue, oldFunctionKeyName, newFunctionKeyName, newValues);
                if (elementFound) {
                    std::string elementName = element.GetName();
                    anno.DeleteAnnotationElementByName(elementName);
                    auto newArrayValue = std::make_unique<ark::pandasm::ArrayValue>(ark::pandasm::Value::Type::METHOD,
                                                                                    std::move(newValues));
                    ark::pandasm::AnnotationElement newElement(elementName, std::move(newArrayValue));
                    anno.AddElement(std::move(newElement));
                    LIBABCKIT_LOG(DEBUG) << "Updated FunctionOverload method in record " << recordName << ": "
                                         << oldFunctionKeyName << " -> " << newFunctionKeyName << std::endl;
                }
            });
        });
    }

    return true;
}

// Check if FunctionalReference annotation needs update
static bool CheckFunctionalReferenceNeedsUpdate(ark::pandasm::AnnotationData &anno, const std::string &elementName,
                                                const std::string &oldFunctionKeyName)
{
    if (anno.GetName() != "ets.annotation.FunctionalReference") {
        return false;
    }
    for (const auto &elem : anno.GetElements()) {
        if (elem.GetName() == elementName) {
            auto *value = elem.GetValue();
            if (value != nullptr && value->GetType() == ark::pandasm::Value::Type::METHOD) {
                auto *scalarValue = value->GetAsScalar();
                if (scalarValue != nullptr) {
                    std::string methodValue = scalarValue->GetValue<std::string>();
                    if (methodValue == oldFunctionKeyName) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Check if NamedFunctionObject annotation needs update
static bool CheckNamedFunctionObjectNeedsUpdate(ark::pandasm::AnnotationData &anno, const std::string &nameElement,
                                                const std::string &oldSimpleName)
{
    if (anno.GetName() != "std.core.NamedFunctionObject") {
        return false;
    }
    for (const auto &elem : anno.GetElements()) {
        if (elem.GetName() == nameElement) {
            auto *value = elem.GetValue();
            if (value != nullptr && value->GetType() == ark::pandasm::Value::Type::STRING) {
                auto *scalarValue = value->GetAsScalar();
                if (scalarValue != nullptr) {
                    std::string nameValue = scalarValue->GetValue<std::string>();
                    if (nameValue == oldSimpleName) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

static bool FunctionRefreshLambdaAnnotationElements(AbckitCoreFunction *function, const std::string &oldFunctionKeyName,
                                                    const std::string &newFunctionKeyName)
{
    LIBABCKIT_LOG_FUNC;

    if (oldFunctionKeyName == newFunctionKeyName) {
        return true;
    }

    auto *prog = function->owningModule->file->GetStaticProgram();
    const std::string LAMBDA_RECORD_KEY = "%%lambda-";
    const std::string FUNCTIONAL_REFERENCE_ANNO = "ets.annotation.FunctionalReference";
    const std::string NAMED_FUNCTION_OBJECT_ANNO = "std.core.NamedFunctionObject";
    const std::string ELEMENT_NAME = "value";
    const std::string NAME_ELEMENT = "name";

    std::string oldSimpleName = ExtractSimpleFunctionName(oldFunctionKeyName);
    std::string newSimpleName = ExtractSimpleFunctionName(newFunctionKeyName);
    bool nameChanged = !oldSimpleName.empty() && oldSimpleName != newSimpleName;

    for (const auto &[recordNameRef, record] : prog->recordTable) {
        std::string recordName = recordNameRef;

        if (recordName.find(LAMBDA_RECORD_KEY) == std::string::npos || record.metadata == nullptr) {
            continue;
        }

        bool needsUpdateFuncRef = false;
        bool needsUpdateNamedFunc = false;

        record.metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
            if (CheckFunctionalReferenceNeedsUpdate(anno, ELEMENT_NAME, oldFunctionKeyName)) {
                needsUpdateFuncRef = true;
                LIBABCKIT_LOG(DEBUG) << "Found lambda record " << recordName
                                     << " with FunctionalReference to old function: " << oldFunctionKeyName
                                     << std::endl;
            } else if (nameChanged && CheckNamedFunctionObjectNeedsUpdate(anno, NAME_ELEMENT, oldSimpleName)) {
                needsUpdateNamedFunc = true;
                LIBABCKIT_LOG(DEBUG) << "Found lambda record " << recordName
                                     << " with NamedFunctionObject name: " << oldSimpleName << std::endl;
            }
        });

        if (needsUpdateFuncRef) {
            LIBABCKIT_LOG(DEBUG) << "Updating FunctionalReference in " << recordName << " from " << oldFunctionKeyName
                                 << " to " << newFunctionKeyName << std::endl;
            if (!UpdateAnnotationElementValueMethod(record.metadata.get(), FUNCTIONAL_REFERENCE_ANNO, ELEMENT_NAME,
                                                    newFunctionKeyName, true)) {
                LIBABCKIT_LOG(ERROR) << "Failed to update FunctionalReference annotation element in " << recordName
                                     << std::endl;
                return false;
            }
        }

        if (needsUpdateNamedFunc) {
            LIBABCKIT_LOG(DEBUG) << "Updating NamedFunctionObject in " << recordName << " from " << oldSimpleName
                                 << " to " << newSimpleName << std::endl;
            if (!UpdateAnnotationElementValueString(record.metadata.get(), NAMED_FUNCTION_OBJECT_ANNO, NAME_ELEMENT,
                                                    newSimpleName)) {
                LIBABCKIT_LOG(ERROR) << "Failed to update NamedFunctionObject annotation element in " << recordName
                                     << std::endl;
                return false;
            }
        }
    }

    return true;
}

static bool FunctionRefreshAsyncAnnotationElements(AbckitCoreFunction *function, const std::string &oldFunctionKeyName,
                                                   const std::string &newFunctionKeyName)
{
    LIBABCKIT_LOG_FUNC;

    if (oldFunctionKeyName == newFunctionKeyName) {
        return true;
    }

    auto *prog = function->owningModule->file->GetStaticProgram();

    bool success = true;
    success &=
        ProcessAsyncAnnotationsInFunctionTable(prog->functionStaticTable, oldFunctionKeyName, newFunctionKeyName, true);
    success &= ProcessAsyncAnnotationsInFunctionTable(prog->functionInstanceTable, oldFunctionKeyName,
                                                      newFunctionKeyName, false);

    return success;
}

static bool UpdateAnnotationElementValueString(ark::pandasm::RecordMetadata *metadata, const std::string &annoName,
                                               const std::string &elementName, const std::string &newValue)
{
    auto newVal = std::make_unique<ark::pandasm::ScalarValue>(
        ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::STRING>(newValue));
    return UpdateAnnotationElementValue(metadata, annoName, elementName, std::move(newVal));
}

static bool UpdateAnnotationElementValueRecord(ark::pandasm::RecordMetadata *metadata, const std::string &annoName,
                                               const std::string &elementName, const std::string &recordName)
{
    auto recordType = ark::pandasm::Type::FromName(recordName, true);
    auto newVal = std::make_unique<ark::pandasm::ScalarValue>(
        ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::RECORD>(recordType));
    return UpdateAnnotationElementValue(metadata, annoName, elementName, std::move(newVal));
}

// Update method array values with prefix replacement
static bool UpdateFunctionOverloadArrayValuesWithPrefix(ark::pandasm::ArrayValue *arrayValue,
                                                        const std::string &oldPrefix, const std::string &newPrefix,
                                                        std::vector<ark::pandasm::ScalarValue> &newValues)
{
    bool elementFound = false;
    auto &values = arrayValue->GetValues();

    for (const auto &val : values) {
        if (val.GetType() != ark::pandasm::Value::Type::METHOD) {
            newValues.push_back(val);
            continue;
        }

        auto *scalarValue = val.GetAsScalar();
        if (scalarValue == nullptr) {
            newValues.push_back(val);
            continue;
        }

        std::string methodName = scalarValue->GetValue<std::string>();
        bool isStatic = scalarValue->IsStatic();

        if (methodName.find(oldPrefix) == 0) {
            std::string newMethodName = newPrefix + methodName.substr(oldPrefix.length());
            newValues.push_back(
                ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::METHOD>(newMethodName, isStatic));
            elementFound = true;
            LIBABCKIT_LOG(DEBUG) << "Updated FunctionOverload method: " << methodName << " -> " << newMethodName
                                 << std::endl;
        } else {
            newValues.push_back(val);
        }
    }

    return elementFound;
}

static bool RefreshFunctionOverloadAnnotationElements(ark::pandasm::Record *record, const std::string &oldPrefix,
                                                      const std::string &newPrefix)
{
    LIBABCKIT_LOG(DEBUG) << "RefreshFunctionOverloadAnnotationElements: " << oldPrefix << " -> " << newPrefix
                         << std::endl;

    if (oldPrefix == newPrefix || oldPrefix.empty() || newPrefix.empty()) {
        return true;
    }

    if (record == nullptr || record->metadata == nullptr) {
        return true;
    }

    constexpr const char *FUNCTION_OVERLOAD_ANNOTATION = "ets.annotation.FunctionOverload";
    bool found = false;

    record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
        if (anno.GetName() != FUNCTION_OVERLOAD_ANNOTATION) {
            return;
        }

        anno.EnumerateAnnotationElements([&](ark::pandasm::AnnotationElement &element) {
            auto *value = element.GetValue();
            if (value == nullptr || value->GetType() != ark::pandasm::Value::Type::ARRAY) {
                return;
            }

            auto *arrayValue = static_cast<ark::pandasm::ArrayValue *>(value);
            if (arrayValue->GetComponentType() != ark::pandasm::Value::Type::METHOD) {
                return;
            }

            std::vector<ark::pandasm::ScalarValue> newValues;
            bool elementFound =
                UpdateFunctionOverloadArrayValuesWithPrefix(arrayValue, oldPrefix, newPrefix, newValues);
            if (elementFound) {
                std::string elementName = element.GetName();
                anno.DeleteAnnotationElementByName(elementName);
                auto newArrayValue =
                    std::make_unique<ark::pandasm::ArrayValue>(ark::pandasm::Value::Type::METHOD, std::move(newValues));
                ark::pandasm::AnnotationElement newElement(elementName, std::move(newArrayValue));
                anno.AddElement(std::move(newElement));
                found = true;
            }
        });
    });

    return true;
}

static bool RefreshModuleExportedAnnotationElements(ark::pandasm::Record *record, const std::string &oldModuleName,
                                                    const std::string &newModuleName)
{
    LIBABCKIT_LOG(DEBUG) << "RefreshModuleExportedAnnotationElements: " << oldModuleName << " -> " << newModuleName
                         << std::endl;

    if (oldModuleName == newModuleName || oldModuleName.empty() || newModuleName.empty()) {
        return true;
    }

    if (record == nullptr || record->metadata == nullptr) {
        return true;
    }

    constexpr const char *MODULE_ANNOTATION = "ets.annotation.Module";
    constexpr const char *EXPORTED_ELEMENT = "exported";

    bool found = false;
    std::vector<ark::pandasm::ScalarValue> newValues;
    ark::pandasm::Value::Type componentType;
    const std::string oldPrefix = oldModuleName + ".";
    const std::string newPrefix = newModuleName + ".";

    record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
        if (anno.GetName() != MODULE_ANNOTATION) {
            return;
        }

        anno.EnumerateAnnotationElements([&](ark::pandasm::AnnotationElement &element) {
            if (element.GetName() != EXPORTED_ELEMENT) {
                return;
            }

            auto *value = element.GetValue();
            if (value == nullptr || value->GetType() != ark::pandasm::Value::Type::ARRAY) {
                return;
            }

            auto *arrayValue = static_cast<ark::pandasm::ArrayValue *>(value);
            componentType = arrayValue->GetComponentType();
            auto &values = arrayValue->GetValues();

            for (const auto &val : values) {
                if (val.GetType() != ark::pandasm::Value::Type::RECORD) {
                    newValues.push_back(val);
                    continue;
                }
                auto recordType = val.GetValue<ark::pandasm::Type>();
                std::string recordName = recordType.GetName();
                if (recordName.find(oldPrefix) == 0) {
                    std::string newRecordName = newPrefix + recordName.substr(oldPrefix.length());
                    auto newRecordType = ark::pandasm::Type::FromName(newRecordName, true);
                    newValues.push_back(
                        ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::RECORD>(newRecordType));
                    found = true;
                    LIBABCKIT_LOG(DEBUG) << "Updated exported element: " << recordName << " -> " << newRecordName
                                         << std::endl;
                } else {
                    newValues.push_back(val);
                }
            }
        });
    });

    if (found) {
        record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
            if (anno.GetName() == MODULE_ANNOTATION) {
                anno.DeleteAnnotationElementByName(EXPORTED_ELEMENT);
            }
        });

        auto newArrayValue = std::make_unique<ark::pandasm::ArrayValue>(componentType, std::move(newValues));
        ark::pandasm::AnnotationElement newElement(EXPORTED_ELEMENT, std::move(newArrayValue));

        record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
            if (anno.GetName() == MODULE_ANNOTATION) {
                anno.AddElement(std::move(newElement));
            }
        });
    }

    return true;
}

static void RefreshNamespaceExportedElementsRecursive(AbckitCoreNamespace *ns, const std::string &oldModuleName,
                                                      const std::string &newModuleName)
{
    auto record = libabckit::GetStaticImplRecord(ns);
    if (record != nullptr) {
        RefreshModuleExportedAnnotationElements(record, oldModuleName, newModuleName);
        const std::string oldModulePrefix = oldModuleName + ".";
        const std::string newModulePrefix = newModuleName + ".";
        RefreshFunctionOverloadAnnotationElements(record, oldModulePrefix, newModulePrefix);
    }

    // recursivly process child namespaces
    for (const auto &[_, childNs] : ns->nt) {
        RefreshNamespaceExportedElementsRecursive(childNs.get(), oldModuleName, newModuleName);
    }
}

static ark::pandasm::RecordMetadata *GetRecordMetadata(
    std::variant<AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreEnum *> object)
{
    return std::visit(
        [](auto &coreObject) -> ark::pandasm::RecordMetadata * {
            std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreInterface *,
                         AbckitCoreEnum *, AbckitCoreAnnotationInterface *>
                variantObj = coreObject;
            auto progRecord = libabckit::GetStaticImplRecord(variantObj);
            if (progRecord == nullptr) {
                return nullptr;
            }
            return progRecord->metadata.get();
        },
        object);
}

static bool RefreshEnclosingClassAnnotationElement(
    std::variant<AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreEnum *> object)
{
    LIBABCKIT_LOG_FUNC;

    auto *metadata = GetRecordMetadata(object);
    if (metadata == nullptr) {
        return true;
    }

    std::string parentName;

    if (std::holds_alternative<AbckitCoreNamespace *>(object)) {
        auto ns = std::get<AbckitCoreNamespace *>(object);
        if (ns->parentNamespace != nullptr) {
            std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreInterface *,
                         AbckitCoreEnum *, AbckitCoreAnnotationInterface *>
                variantObj = ns->parentNamespace;
            auto record = libabckit::GetStaticImplRecord(variantObj);
            if (record != nullptr) {
                parentName = record->name;
            }
        }
    } else if (std::holds_alternative<AbckitCoreClass *>(object)) {
        auto klass = std::get<AbckitCoreClass *>(object);
        if (klass->parentNamespace != nullptr) {
            std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreInterface *,
                         AbckitCoreEnum *, AbckitCoreAnnotationInterface *>
                variantObj = klass->parentNamespace;
            auto record = libabckit::GetStaticImplRecord(variantObj);
            if (record != nullptr) {
                parentName = record->name;
            }
        }
    } else if (std::holds_alternative<AbckitCoreEnum *>(object)) {
        auto enm = std::get<AbckitCoreEnum *>(object);
        if (enm->parentNamespace != nullptr) {
            std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreInterface *,
                         AbckitCoreEnum *, AbckitCoreAnnotationInterface *>
                variantObj = enm->parentNamespace;
            auto record = libabckit::GetStaticImplRecord(variantObj);
            if (record != nullptr) {
                parentName = record->name;
            }
        }
    }

    if (parentName.empty()) {
        return true;
    }

    LIBABCKIT_LOG(DEBUG) << "Updating EnclosingClass annotation element to: " << parentName << std::endl;
    UpdateAnnotationElementValueRecord(metadata, "ets.annotation.EnclosingClass", "value", parentName);

    return true;
}

static bool RefreshInnerClassAnnotationElement(
    std::variant<AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreEnum *> object)
{
    LIBABCKIT_LOG_FUNC;

    auto *metadata = GetRecordMetadata(object);
    if (metadata == nullptr) {
        return true;
    }

    std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *,
                 AbckitCoreAnnotationInterface *>
        variantObj = std::visit(
            [](auto &obj) -> std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *,
                                          AbckitCoreInterface *, AbckitCoreEnum *, AbckitCoreAnnotationInterface *> {
                return obj;
            },
            object);
    auto progRecord = libabckit::GetStaticImplRecord(variantObj);
    if (progRecord == nullptr) {
        return true;
    }

    std::string fullName = progRecord->name;

    LIBABCKIT_LOG(DEBUG) << "Updating InnerClass annotation name element to: " << fullName << std::endl;
    UpdateAnnotationElementValueString(metadata, "ets.annotation.InnerClass", "name", fullName);

    return true;
}

static bool RefreshExportedAnnotationElement(ark::pandasm::Record *record, const std::string &oldName,
                                             const std::string &newName)
{
    LIBABCKIT_LOG(DEBUG) << "RefreshExportedAnnotationElement: " << oldName << " -> " << newName << std::endl;

    if (oldName == newName || oldName.empty() || newName.empty()) {
        return true;
    }

    if (record == nullptr || record->metadata == nullptr) {
        return true;
    }

    constexpr const char *MODULE_ANNOTATION = "ets.annotation.Module";
    constexpr const char *EXPORTED_ELEMENT = "exported";

    bool found = false;
    std::vector<ark::pandasm::ScalarValue> newValues;
    ark::pandasm::Value::Type componentType;
    const std::string oldPrefix = oldName + ".";
    const std::string newPrefix = newName + ".";

    record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
        if (anno.GetName() != MODULE_ANNOTATION) {
            return;
        }

        anno.EnumerateAnnotationElements([&](ark::pandasm::AnnotationElement &element) {
            if (element.GetName() != EXPORTED_ELEMENT) {
                return;
            }

            auto *value = element.GetValue();
            if (value == nullptr || value->GetType() != ark::pandasm::Value::Type::ARRAY) {
                return;
            }

            auto *arrayValue = static_cast<ark::pandasm::ArrayValue *>(value);
            componentType = arrayValue->GetComponentType();
            auto &values = arrayValue->GetValues();

            for (const auto &val : values) {
                if (val.GetType() != ark::pandasm::Value::Type::RECORD) {
                    newValues.push_back(val);
                    continue;
                }

                auto recordType = val.GetValue<ark::pandasm::Type>();
                std::string recordName = recordType.GetName();
                // Check for exact match or prefix match
                if (recordName == oldName) {
                    // Exact match: update to new name
                    auto newRecordType = ark::pandasm::Type::FromName(newName, true);
                    newValues.push_back(
                        ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::RECORD>(newRecordType));
                    found = true;
                    LIBABCKIT_LOG(DEBUG) << "Updated exported element: " << oldName << " -> " << newName << std::endl;
                } else if (recordName.find(oldPrefix) == 0) {
                    // Prefix match: update namespace prefix for nested elements (enum, class, etc.)
                    std::string newRecordName = newPrefix + recordName.substr(oldPrefix.length());
                    auto newRecordType = ark::pandasm::Type::FromName(newRecordName, true);
                    newValues.push_back(
                        ark::pandasm::ScalarValue::Create<ark::pandasm::Value::Type::RECORD>(newRecordType));
                    found = true;
                    LIBABCKIT_LOG(DEBUG) << "Updated exported element: " << recordName << " -> " << newRecordName
                                         << std::endl;
                } else {
                    newValues.push_back(val);
                }
            }
        });
    });

    if (found) {
        record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
            if (anno.GetName() == MODULE_ANNOTATION) {
                anno.DeleteAnnotationElementByName(EXPORTED_ELEMENT);
            }
        });

        auto newArrayValue = std::make_unique<ark::pandasm::ArrayValue>(componentType, std::move(newValues));
        ark::pandasm::AnnotationElement newElement(EXPORTED_ELEMENT, std::move(newArrayValue));

        record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
            if (anno.GetName() == MODULE_ANNOTATION) {
                anno.AddElement(std::move(newElement));
            }
        });
    }

    return true;
}

}  // namespace

// --------------------------------------- public ----------------------
bool libabckit::ModifyNameHelper::ModuleRefreshName(AbckitCoreModule *m, const std::string &newName)
{
    LIBABCKIT_LOG(DEBUG) << "old module name:" << m->moduleName->impl << std::endl;
    LIBABCKIT_LOG(DEBUG) << "new module name:" << newName << std::endl;

    auto record = GetStaticImplRecord(m);
    const auto oldRecordName = record->name;
    const auto newRecordName = newName + std::string(NAME_DELIMITER) + std::string(GLOBAL_CLASS);
    const std::string oldModuleName = m->moduleName->impl.data();  // Get old module name before updating

    LIBABCKIT_LOG(DEBUG) << "old module record name:" << oldRecordName << std::endl;
    LIBABCKIT_LOG(DEBUG) << "new module record name:" << newRecordName << std::endl;

    // update prog record name
    if (!ProgramUpdateRecordTableKey(m->file->GetStaticProgram(), oldRecordName, newRecordName)) {
        LIBABCKIT_LOG(ERROR) << "Failed to update module program record" << std::endl;
        return false;
    }

    record->name = newRecordName;
    record->sourceFile = StringUtil::ReplaceAll(record->sourceFile, m->moduleName->impl.data(), newName);

    // update module name
    m->moduleName = CreateStringStatic(m->file, newName.data(), newName.length());

    // update all exported elements that start with the old module name prefix
    // handles both the module itself and all its child elements (namespaces, classes)
    if (!RefreshModuleExportedAnnotationElements(record, oldModuleName, newName)) {
        LIBABCKIT_LOG(ERROR) << "Failed to refresh module exported annotation elements" << std::endl;
        return false;
    }

    // Update FunctionOverload annotation elements in the module record
    const std::string oldModulePrefix = oldModuleName + ".";
    const std::string newModulePrefix = newName + ".";
    if (!RefreshFunctionOverloadAnnotationElements(record, oldModulePrefix, newModulePrefix)) {
        LIBABCKIT_LOG(ERROR) << "Failed to refresh module FunctionOverload annotation elements" << std::endl;
        return false;
    }

    // Also update exported elements in all namespaces (including nested ones)
    // Namespaces can also have Module annotations with exported elements
    for (const auto &[_, ns] : m->nt) {
        RefreshNamespaceExportedElementsRecursive(ns.get(), oldModuleName, newName);
    }

    ModuleRefreshNamespaces(m);
    ModuleRefreshEnums(m);
    ModuleRefreshInterfaces(m);
    ModuleRefreshClasses(m);
    ModuleRefreshFunctions(m);
    ModuleRefreshAnnotationInterfaces(m);

    return true;
}

bool libabckit::ModifyNameHelper::NamespaceRefreshName(AbckitCoreNamespace *ns, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;

    auto oldFullName = GetStaticImplRecord(ns)->name;

    if (!ObjectRefreshName(ns, newName)) {
        return false;
    }

    auto newFullName = GetStaticImplRecord(ns)->name;

    // Update the exported annotation element of the parent container
    if (oldFullName != newFullName) {
        ark::pandasm::Record *parentRecord = nullptr;
        if (ns->parentNamespace == nullptr) {
            // Top-level: update module's ETSGLOBAL record
            parentRecord = GetStaticImplRecord(ns->owningModule);
        } else {
            // Nested: update parent namespace's record
            parentRecord = GetStaticImplRecord(ns->parentNamespace);
        }
        if (!RefreshExportedAnnotationElement(parentRecord, oldFullName, newFullName)) {
            return false;
        }
    }

    // Refresh namespace annotation elements for the current namespace first
    if (!RefreshEnclosingClassAnnotationElement(ns)) {
        LIBABCKIT_LOG(ERROR) << "Failed to refresh EnclosingClass annotation element" << std::endl;
        return false;
    }
    if (!RefreshInnerClassAnnotationElement(ns)) {
        LIBABCKIT_LOG(ERROR) << "Failed to refresh InnerClass annotation element" << std::endl;
        return false;
    }

    // Update FunctionOverload annotation elements in the namespace record
    // If namespace name changed, method names in FunctionOverload annotations need to be updated
    auto nsRecord = GetStaticImplRecord(ns);
    if (nsRecord != nullptr && oldFullName != newFullName) {
        const std::string oldNsPrefix = oldFullName + ".";
        const std::string newNsPrefix = newFullName + ".";
        RefreshFunctionOverloadAnnotationElements(nsRecord, oldNsPrefix, newNsPrefix);
    }

    // Recursively refresh the annotation elements of the child namespaces
    NamespaceRefreshNamespaces(ns);

    // Refresh the annotation elements of the classes and enums directly included in the current namespace
    if (!NamespaceRefreshAnnotationElements(ns)) {
        return false;
    }

    // Refresh the names of the other child elements
    NamespaceRefreshEnums(ns);
    NamespaceRefreshInterfaces(ns);
    NamespaceRefreshClasses(ns);
    NamespaceRefreshFunctions(ns);
    NamespaceRefreshAnnotationInterfaces(ns);

    return true;
}

void libabckit::ModifyNameHelper::GenerateNewFunctionNames(AbckitCoreFunction *function, const std::string &newName,
                                                           const std::string &oldFunctionKeyName,
                                                           std::string &newFunctionKeyName,
                                                           std::string &newFunctionName)
{
    auto *funcImpl = function->GetArkTSImpl()->GetStaticImpl();

    if (newName.empty()) {
        FunctionRefreshParams(function);
        FunctionRefreshReturnType(function);
        newFunctionKeyName = libabckit::NameUtil::GetFullName(function);
        newFunctionName = Split(newFunctionKeyName, std::string(FUNCTION_DELIMITER))[0];
    } else if (oldFunctionKeyName.find(std::string(ASYNC_PREFIX)) != std::string::npos) {
        newFunctionKeyName =
            std::regex_replace(oldFunctionKeyName, std::regex(std::string(ASYNC_PATTERN)), AsyncReplace(newName));
        newFunctionName = Split(newFunctionKeyName, std::string(FUNCTION_DELIMITER))[0];
    } else {
        auto newFuncName = AddGetSetPrefix(oldFunctionKeyName, newName);
        newFunctionName = libabckit::NameUtil::GetPackageName(function) + std::string(NAME_DELIMITER) + newFuncName;
        newFunctionKeyName = ark::pandasm::MangleFunctionName(newFunctionName, funcImpl->params, funcImpl->returnType);
    }
}

bool libabckit::ModifyNameHelper::FunctionRefreshName(AbckitCoreFunction *function, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;

    if (function->asyncImpl && !FunctionRefreshName(function->asyncImpl.get(), newName)) {
        return false;
    }

    auto *funcImpl = function->GetArkTSImpl()->GetStaticImpl();
    auto oldFunctionKeyName = ark::pandasm::MangleFunctionName(funcImpl->name, funcImpl->params, funcImpl->returnType);
    std::string newFunctionKeyName;
    std::string newFunctionName;

    ModifyNameHelper::GenerateNewFunctionNames(function, newName, oldFunctionKeyName, newFunctionKeyName,
                                               newFunctionName);

    LIBABCKIT_LOG(DEBUG) << "old function key name:" << oldFunctionKeyName << std::endl;
    LIBABCKIT_LOG(DEBUG) << "new function key name:" << newFunctionKeyName << std::endl;
    LIBABCKIT_LOG(DEBUG) << "new function name:" << newFunctionName << std::endl;

    if (oldFunctionKeyName == newFunctionKeyName) {
        return true;
    }

    if (!ProgramUpdateFunctionTableKey(function->owningModule->file->GetStaticProgram(), oldFunctionKeyName,
                                       newFunctionKeyName)) {
        LIBABCKIT_LOG(ERROR) << "Failed to update function program table" << std::endl;
        return false;
    }
    funcImpl->name = newFunctionName;

    if (!FunctionRefreshAsyncAnnotationElements(function, oldFunctionKeyName, newFunctionKeyName)) {
        LIBABCKIT_LOG(ERROR) << "Failed to refresh async annotation elements" << std::endl;
        return false;
    }

    if (!FunctionRefreshLambdaAnnotationElements(function, oldFunctionKeyName, newFunctionKeyName)) {
        LIBABCKIT_LOG(ERROR) << "Failed to refresh lambda annotation elements" << std::endl;
        return false;
    }

    if (!FunctionRefreshFunctionOverloadAnnotationElements(function, oldFunctionKeyName, newFunctionKeyName)) {
        LIBABCKIT_LOG(ERROR) << "Failed to refresh FunctionOverload annotation elements" << std::endl;
        return false;
    }

    return true;
}

bool libabckit::ModifyNameHelper::ClassRefreshName(AbckitCoreClass *klass, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;

    auto oldFullName = GetStaticImplRecord(klass)->name;
    if (!ObjectRefreshName(klass, newName)) {
        return false;
    }

    auto newFullName = GetStaticImplRecord(klass)->name;
    if (oldFullName != newFullName) {
        ark::pandasm::Record *parentRecord = nullptr;
        if (klass->parentNamespace == nullptr) {
            parentRecord = GetStaticImplRecord(klass->owningModule);
        } else {
            parentRecord = GetStaticImplRecord(klass->parentNamespace);
        }
        if (!RefreshExportedAnnotationElement(parentRecord, oldFullName, newFullName)) {
            return false;
        }
    }

    if (!ObjectRefreshTypeUsersName(klass, newFullName)) {
        return false;
    }

    ClassRefreshMethods(klass);

    // Update FunctionOverload annotation elements in the class record
    // Method names have changed, so FunctionOverload annotations need to be updated
    auto classRecord = GetStaticImplRecord(klass);
    if (classRecord != nullptr && oldFullName != newFullName) {
        const std::string oldClassPrefix = oldFullName + ".";
        const std::string newClassPrefix = newFullName + ".";
        RefreshFunctionOverloadAnnotationElements(classRecord, oldClassPrefix, newClassPrefix);
    }

    ClassRefreshObjectLiteral(klass);
    ClassRefreshSubClasses(klass);
    RefreshPartial(klass);
    RefreshArray(klass);

    return true;
}

bool libabckit::ModifyNameHelper::EnumRefreshName(AbckitCoreEnum *enm, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;

    auto oldEnumName = GetStaticImplRecord(enm)->name;
    if (!ObjectRefreshName(enm, newName)) {
        return false;
    }

    auto newFullName = GetStaticImplRecord(enm)->name;
    if (oldEnumName != newFullName) {
        // Update parent namespace's exported annotation (if enum is nested in namespace)
        if (enm->parentNamespace != nullptr) {
            ark::pandasm::Record *parentRecord = GetStaticImplRecord(enm->parentNamespace);
            if (!RefreshExportedAnnotationElement(parentRecord, oldEnumName, newFullName)) {
                return false;
            }
        }
        // Always update module's exported annotation (export enum appears in module's exported list)
        ark::pandasm::Record *moduleRecord = GetStaticImplRecord(enm->owningModule);
        if (!RefreshExportedAnnotationElement(moduleRecord, oldEnumName, newFullName)) {
            return false;
        }
    }

    if (!ObjectRefreshTypeUsersName(enm, newFullName)) {
        return false;
    }

    EnumRefreshMethods(enm);
    EnumRefreshFieldsType(enm);
    RefreshPartial(enm);
    RefreshArray(enm);

    return true;
}

bool libabckit::ModifyNameHelper::AnnotationRefreshName(AbckitCoreAnnotation *anno, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    if (anno->ai == nullptr) {
        return true;
    }
    if (!newName.empty()) {
        if (!AnnotationInterfaceRefreshName(anno->ai, newName)) {
            return false;
        }
    }
    const std::string oldAIName = anno->name->impl.data();
    const std::string newAIName = NameUtil::GetFullName(anno->ai, newName);
    LIBABCKIT_LOG(DEBUG) << "old annotation name:" << oldAIName << std::endl;
    LIBABCKIT_LOG(DEBUG) << "new annotation name:" << newAIName << std::endl;
    anno->name = CreateStringStatic(anno->ai->owningModule->file, newAIName.c_str(), newAIName.size());

    return std::visit(
        [&](auto owner) {
            using Type = std::decay_t<decltype(owner)>;
            if constexpr (std::is_same_v<Type, AbckitCoreClass *>) {
                return ClassRefreshAnnotations(owner, oldAIName, newAIName);
            } else if constexpr (std::is_same_v<Type, AbckitCoreInterface *>) {
                return InterfaceRefreshAnnotations(owner, oldAIName, newAIName);
            } else if constexpr (std::is_same_v<Type, AbckitCoreFunction *>) {
                return FunctionRefreshAnnotations(owner, oldAIName, newAIName);
            } else if constexpr (std::is_same_v<Type, AbckitCoreClassField *>) {
                return ClassFieldRefreshAnnotations(owner, oldAIName, newAIName);
            } else {
                return false;
            }
        },
        anno->owner);
}

bool libabckit::ModifyNameHelper::InterfaceRefreshName(AbckitCoreInterface *iface, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ObjectRefreshName(iface, newName)) {
        return false;
    }

    if (!ObjectRefreshTypeUsersName(iface, GetStaticImplRecord(iface)->name)) {
        return false;
    }

    InterfaceRefreshObjectLiteral(iface);
    // refresh partial first, to get the updated partial name when InterfaceRefreshClasses is called
    RefreshPartial(iface);
    InterfaceRefreshClasses(iface);
    InterfaceRefreshSubInterfaces(iface);
    InterfaceRefreshMethods(iface);
    RefreshPartial(iface);
    RefreshArray(iface);

    return true;
}

bool libabckit::ModifyNameHelper::AnnotationInterfaceRefreshName(AbckitCoreAnnotationInterface *ai,
                                                                 const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    const auto record = GetStaticImplRecord(ai);
    const auto oldAIName = record->name;
    const auto newAIName = NameUtil::GetFullName(ai, newName);
    if (oldAIName == newAIName) {
        return true;
    }

    LIBABCKIT_LOG(DEBUG) << "old annotation interface name:" << oldAIName << std::endl;
    LIBABCKIT_LOG(DEBUG) << "new annotation interface name:" << newAIName << std::endl;

    if (!ProgramUpdateRecordTableKey(ai->owningModule->file->GetStaticProgram(), oldAIName, newAIName)) {
        LIBABCKIT_LOG(ERROR) << "Failed to update annotation "
                                "interface program record"
                             << std::endl;
        return false;
    }

    record->name = newAIName;

    AnnotationInterfaceRefreshAnnotation(ai);
    return true;
}

bool libabckit::ModifyNameHelper::FieldRefreshName(
    std::variant<AbckitCoreModuleField *, AbckitCoreNamespaceField *, AbckitCoreClassField *, AbckitCoreEnumField *>
        field,
    const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;

    if (newName.empty()) {
        return false;
    }

    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    return std::visit(
        [newName, funcName](auto &object) {
            auto fieldRecord = object->GetArkTSImpl()->GetStaticImpl();
            const auto oldName = fieldRecord->name;
            if (oldName == newName) {
                return true;
            }
            std::string processNewName;
            // if oldName is <property>xxx and newName does not
            // contain '<property>' prefix, add '<property>' to the
            // beginning of newName
            if (oldName.find(INTERFACE_FIELD_PREFIX) == 0 && newName.find(INTERFACE_FIELD_PREFIX) != 0) {
                processNewName.append(INTERFACE_FIELD_PREFIX).append(newName);
            } else {
                processNewName = newName;
            }

            LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "fieldName refreshed: " << oldName << " --> " << processNewName
                                         << std::endl;
            fieldRecord->name = processNewName;

            return true;
        },
        field);
}

bool libabckit::ModifyNameHelper::InterfaceFieldRefreshName(AbckitCoreInterfaceField *ifaceField,
                                                            const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    if (newName.empty()) {
        return false;
    }

    std::string oldName = ifaceField->name->impl.data();
    std::string newFieldName = std::string(INTERFACE_FIELD_PREFIX) + newName;
    ifaceField->name =
        CreateStringStatic(ifaceField->owner->owningModule->file, newFieldName.data(), newFieldName.size());

    // Refresh main interface's get/set methods
    GetSetMethodRefreshName(ifaceField->owner, oldName, newName);

    // Refresh partial interface's get/set methods
    if (ifaceField->owner->partial != nullptr) {
        GetSetMethodRefreshName(ifaceField->owner->partial.get(), oldName, newName);
    }

    // Refresh object literals for both main and partial
    InterfaceRefreshObjectLiteralField(ifaceField->owner, oldName, newName);

    return true;
}

bool libabckit::ModifyNameHelper::ModuleRefreshNamespaces(AbckitCoreModule *m)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, ns] : m->nt) {
        if (!NamespaceRefreshName(ns.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::ModuleRefreshFunctions(AbckitCoreModule *m)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &function : m->functions) {
        if (!FunctionRefreshName(function.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::ModuleRefreshClasses(AbckitCoreModule *m)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, klass] : m->ct) {
        if (!ClassRefreshName(klass.get())) {
            return false;
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::ModuleRefreshInterfaces(AbckitCoreModule *m)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, iface] : m->it) {
        if (!InterfaceRefreshName(iface.get())) {
            return false;
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::ModuleRefreshEnums(AbckitCoreModule *m)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, enm] : m->et) {
        if (!EnumRefreshName(enm.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::ModuleRefreshAnnotationInterfaces(AbckitCoreModule *m)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, ai] : m->at) {
        if (!AnnotationInterfaceRefreshName(ai.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::NamespaceRefreshNamespaces(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, ns] : ns->nt) {
        if (!NamespaceRefreshName(ns.get())) {
            return false;
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::NamespaceRefreshFunctions(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &function : ns->functions) {
        if (!FunctionRefreshName(function.get())) {
            return false;
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::NamespaceRefreshClasses(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, klass] : ns->ct) {
        if (!ClassRefreshName(klass.get())) {
            return false;
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::NamespaceRefreshInterfaces(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, iface] : ns->it) {
        if (!InterfaceRefreshName(iface.get())) {
            return false;
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::NamespaceRefreshEnums(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, enm] : ns->et) {
        if (!EnumRefreshName(enm.get())) {
            return false;
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::NamespaceRefreshAnnotationInterfaces(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, ai] : ns->at) {
        if (!AnnotationInterfaceRefreshName(ai.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::NamespaceRefreshAnnotationElements(AbckitCoreNamespace *ns)
{
    LIBABCKIT_LOG_FUNC;

    for (const auto &[_, klass] : ns->ct) {
        if (!RefreshEnclosingClassAnnotationElement(klass.get())) {
            LIBABCKIT_LOG(ERROR) << "Failed to refresh EnclosingClass annotation element" << std::endl;
            return false;
        }
        if (!RefreshInnerClassAnnotationElement(klass.get())) {
            LIBABCKIT_LOG(ERROR) << "Failed to refresh InnerClass annotation element" << std::endl;
            return false;
        }
    }

    for (const auto &[_, enm] : ns->et) {
        if (!RefreshEnclosingClassAnnotationElement(enm.get())) {
            LIBABCKIT_LOG(ERROR) << "Failed to refresh EnclosingClass annotation element" << std::endl;
            return false;
        }
        if (!RefreshInnerClassAnnotationElement(enm.get())) {
            LIBABCKIT_LOG(ERROR) << "Failed to refresh InnerClass annotation element" << std::endl;
            return false;
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::FunctionRefreshParams(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG(DEBUG) << "functionName:" << function->GetArkTSImpl()->GetStaticImpl()->name << std::endl;
    const auto funcImpl = function->GetArkTSImpl()->GetStaticImpl();
    for (size_t i = 0; i < function->parameters.size(); i++) {
        const auto type = function->parameters[i]->type;
        if (type->id != AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE) {
            continue;
        }

        auto newName = StringUtil::GetTypeNameStr(function->parameters[i]->type);
        newName = StringUtil::RemoveBracketsSuffix(newName);  // ark::pandasm::Type will add []
        LIBABCKIT_LOG(DEBUG) << "old param[" << i << "] type name:" << funcImpl->params[i].type.GetName() << std::endl;
        LIBABCKIT_LOG(DEBUG) << "new param[" << i << "] type name:" << newName << std::endl;
        auto progType = funcImpl->params[i].type;
        funcImpl->params[i].type = ark::pandasm::Type(newName, progType.GetRank());
    }
    return true;
}

bool libabckit::ModifyNameHelper::FunctionRefreshReturnType(AbckitCoreFunction *function)
{
    LIBABCKIT_LOG(DEBUG) << "functionName:" << function->GetArkTSImpl()->GetStaticImpl()->name << std::endl;
    if (function->returnType->id != AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE) {
        return true;
    }

    const auto funcImpl = function->GetArkTSImpl()->GetStaticImpl();
    auto newName = StringUtil::GetTypeNameStr(function->returnType);
    newName = StringUtil::RemoveBracketsSuffix(newName);  // ark::pandasm::Type will add []
    LIBABCKIT_LOG(DEBUG) << "old returnType name:" << funcImpl->returnType.GetName() << std::endl;
    LIBABCKIT_LOG(DEBUG) << "new returnType name:" << newName << std::endl;
    const auto progType = funcImpl->returnType;
    funcImpl->returnType = ark::pandasm::Type(newName, progType.GetRank());

    return true;
}

bool libabckit::ModifyNameHelper::FunctionRefreshAnnotations(AbckitCoreFunction *function, const std::string &oldName,
                                                             const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    const auto &record = function->GetArkTSImpl()->GetStaticImpl();
    AnnotationUpdateData updateData {oldName, newName};
    record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
        if (anno.GetName() == oldName) {
            anno.SetName(newName);
        }
    });
    return true;
}

bool libabckit::ModifyNameHelper::ClassRefreshMethods(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;

    for (auto &function : klass->methods) {
        if (!FunctionRefreshName(function.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::ClassRefreshObjectLiteral(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;

    auto record = GetStaticImplRecord(klass);
    auto baseName = record->name;
    bool isInterface = record->metadata->GetAccessFlags() & ark::ACC_INTERFACE;

    for (const auto &objectLiteral : klass->objectLiterals) {
        const auto &objectLiteralRecord = GetStaticImplRecord(objectLiteral.get());
        bool isDeclareInterface =
            record->metadata->IsForeign() &&
            !objectLiteralRecord->metadata->GetAttributeValues(std::string(ETS_IMPLEMENTS)).empty();
        if (isInterface || isDeclareInterface) {
            // for interface partials: set ets.implements, not modify ets.extends
            objectLiteralRecord->metadata->SetAttributeValues(std::string(ETS_IMPLEMENTS), {baseName});
        } else {
            // for class partials: get existing extends values and update
            auto existingExtends = objectLiteralRecord->metadata->GetAttributeValues(std::string(ETS_EXTENDS).data());
            if (!existingExtends.empty()) {
                // update the first extends value (should be the partial class name) to the new name
                // and keep other extends values (like std.core.Object) unchanged
                existingExtends[0] = baseName;
                objectLiteralRecord->metadata->SetAttributeValues(std::string(ETS_EXTENDS), existingExtends);
            }
        }

        ObjectLiteralRefreshName(objectLiteral.get(), baseName);
    }
    return true;
}

bool libabckit::ModifyNameHelper::ClassRefreshSubClasses(AbckitCoreClass *klass)
{
    LIBABCKIT_LOG_FUNC;
    const auto superClassName = GetStaticImplRecord(klass)->name;

    auto [moduleName, className] = ClassGetNames(superClassName);
    const std::string superClassPartialName = moduleName + "." + "%%partial-" + className;

    for (const auto &subClass : klass->subClasses) {
        const auto &record = GetStaticImplRecord(subClass);
        record->metadata->SetAttributeValues(std::string(ETS_EXTENDS), {superClassName});
        // Also update the subClass's partial metadata ets.extends

        if (subClass->partial != nullptr) {
            const auto &partialRecord = GetStaticImplRecord(subClass->partial.get());
            auto existingExtends = partialRecord->metadata->GetAttributeValues(std::string(ETS_EXTENDS).data());
            if (!existingExtends.empty()) {
                existingExtends[0] = superClassPartialName;
                partialRecord->metadata->SetAttributeValues(std::string(ETS_EXTENDS), existingExtends);
            }
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::ClassRefreshAnnotations(AbckitCoreClass *klass, const std::string &oldName,
                                                          const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    const auto &record = GetStaticImplRecord(klass);
    AnnotationUpdateData updateData {oldName, newName};
    record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
        if (anno.GetName() == oldName) {
            anno.SetName(newName);
        }
    });
    return true;
}

bool libabckit::ModifyNameHelper::EnumRefreshMethods(AbckitCoreEnum *enm)
{
    LIBABCKIT_LOG_FUNC;

    for (auto &method : enm->methods) {
        if (!FunctionRefreshName(method.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::EnumRefreshFieldsType(AbckitCoreEnum *enm)
{
    for (const auto &field : enm->fields) {
        if (!FieldRefreshTypeName(field.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::InterfaceRefreshMethods(AbckitCoreInterface *iface)
{
    LIBABCKIT_LOG_FUNC;

    for (auto &method : iface->methods) {
        if (!FunctionRefreshName(method.get())) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::InterfaceRefreshObjectLiteral(AbckitCoreInterface *iface)
{
    LIBABCKIT_LOG_FUNC;

    auto baseName = GetStaticImplRecord(iface)->name;
    for (const auto &objectLiteral : iface->objectLiterals) {
        const auto &record = GetStaticImplRecord(objectLiteral.get());
        record->metadata->SetAttributeValues(std::string(ETS_IMPLEMENTS), {baseName});
        ObjectLiteralRefreshName(objectLiteral.get(), baseName);
    }
    return true;
}

bool libabckit::ModifyNameHelper::InterfaceRefreshClasses(AbckitCoreInterface *iface)
{
    LIBABCKIT_LOG_FUNC;
    auto newName = GetStaticImplRecord(iface)->name;
    for (const auto &klass : iface->classes) {
        std::vector<std::string> implements;
        for (const auto &interface : klass->interfaces) {
            implements.emplace_back(GetStaticImplRecord(interface)->name);
        }
        if (!implements.empty()) {
            GetStaticImplRecord(klass)->metadata->SetAttributeValues(std::string(ETS_IMPLEMENTS), implements);
        }
        if (klass->partial != nullptr) {
            std::vector<std::string> partialImplements;
            for (const auto &interface : klass->interfaces) {
                if (interface->partial != nullptr) {
                    partialImplements.emplace_back(GetStaticImplRecord(interface->partial.get())->name);
                }
            }
            if (!partialImplements.empty()) {
                GetStaticImplRecord(klass->partial.get())
                    ->metadata->SetAttributeValues(std::string(ETS_IMPLEMENTS), partialImplements);
            }
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::InterfaceRefreshAnnotations(AbckitCoreInterface *iface, const std::string &oldName,
                                                              const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    const auto &record = GetStaticImplRecord(iface);
    AnnotationUpdateData updateData {oldName, newName};
    record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
        if (anno.GetName() == oldName) {
            anno.SetName(newName);
        }
    });
    return true;
}

bool libabckit::ModifyNameHelper::InterfaceRefreshSubInterfaces(AbckitCoreInterface *iface)
{
    LIBABCKIT_LOG_FUNC;
    for (const auto &subInterface : iface->subInterfaces) {
        // refresh subInterface itself's ets.implements
        const auto &record = GetStaticImplRecord(subInterface);
        std::vector<std::string> superInterfaceNames;
        for (const auto &superInterface : subInterface->superInterfaces) {
            superInterfaceNames.emplace_back(NameUtil::GetFullName(superInterface));
        }
        record->metadata->SetAttributeValues(std::string(ETS_IMPLEMENTS), superInterfaceNames);

        // refresh subInterface's partial's ets.implements
        if (subInterface->partial != nullptr) {
            std::vector<std::string> partialSuperInterfaceNames;
            for (const auto &superInterface : subInterface->superInterfaces) {
                if (superInterface->partial != nullptr) {
                    partialSuperInterfaceNames.emplace_back(GetStaticImplRecord(superInterface->partial.get())->name);
                }
            }
            if (!partialSuperInterfaceNames.empty()) {
                GetStaticImplRecord(subInterface->partial.get())
                    ->metadata->SetAttributeValues(std::string(ETS_IMPLEMENTS), partialSuperInterfaceNames);
            }
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::AnnotationInterfaceRefreshAnnotation(AbckitCoreAnnotationInterface *ai)
{
    LIBABCKIT_LOG_FUNC;
    for (const auto &anno : ai->annotations) {
        if (!AnnotationRefreshName(anno)) {
            return false;
        }
    }
    return true;
}

bool libabckit::ModifyNameHelper::RefreshPartial(
    const std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *> &object)
{
    LIBABCKIT_LOG_FUNC;

    std::visit(
        [](auto &&base) {
            auto baseName = GetStaticImplRecord(base)->name;
            PartialRefreshName(base->partial.get(), baseName);
        },
        object);
    return true;
}

bool libabckit::ModifyNameHelper::RefreshArray(
    const std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *> &object)
{
    LIBABCKIT_LOG_FUNC;

    std::visit(
        [](auto &&base) {
            auto baseName = NameUtil::GetName(base) + ARRAY_SUFFIX;
            ArrayRefreshName(base->array.get(), baseName);
        },
        object);
    return true;
}

bool libabckit::ModifyNameHelper::ArrayRefreshName(AbckitCoreClass *array, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    if (array == nullptr) {
        return true;
    }

    if (!ObjectRefreshName(array, newName, false, false)) {
        return false;
    }

    if (!ObjectRefreshTypeUsersName(array, GetStaticImplRecord(array)->name)) {
        return false;
    }

    return true;
}

bool libabckit::ModifyNameHelper::ObjectLiteralRefreshName(AbckitCoreClass *objectLiteral, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ObjectRefreshName(objectLiteral, newName, true)) {
        return false;
    }

    if (!ObjectRefreshTypeUsersName(objectLiteral, GetStaticImplRecord(objectLiteral)->name)) {
        return false;
    }

    ClassRefreshMethods(objectLiteral);
    return true;
}

bool libabckit::ModifyNameHelper::PartialRefreshName(AbckitCoreClass *partial, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    if (partial == nullptr) {
        return true;
    }

    if (!ObjectRefreshName(partial, newName, false, true)) {
        return false;
    }

    if (!ObjectRefreshTypeUsersName(partial, GetStaticImplRecord(partial)->name)) {
        return false;
    }

    ClassRefreshMethods(partial);
    ClassRefreshObjectLiteral(partial);
    ClassRefreshSubClasses(partial);
    return true;
}

bool libabckit::ModifyNameHelper::ClassFieldRefreshAnnotations(AbckitCoreClassField *classField,
                                                               const std::string &oldName, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    const auto &record = classField->GetArkTSImpl()->GetStaticImpl();
    AnnotationUpdateData updateData {oldName, newName};
    record->metadata->EnumerateAnnotations([&](ark::pandasm::AnnotationData &anno) {
        if (anno.GetName() == oldName) {
            anno.SetName(newName);
        }
    });
    return true;
}

bool libabckit::ModifyNameHelper::GetSetMethodRefreshName(
    const std::variant<AbckitCoreClass *, AbckitCoreInterface *> &object, const std::string &oldName,
    const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    std::string fieldName = oldName;
    if (fieldName.find(INTERFACE_FIELD_PREFIX) != std::string::npos) {
        fieldName = fieldName.substr(INTERFACE_FIELD_PREFIX.size());
    }
    std::regex re(GetSetPattern(fieldName));
    std::string replacement = GetSetReplace(newName);

    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    return std::visit(
        [&](auto *coreObject) {
            for (const auto &function : coreObject->methods) {
                auto *funcImpl = function->GetArkTSImpl()->GetStaticImpl();
                auto oldFunctionKeyName =
                    ark::pandasm::MangleFunctionName(funcImpl->name, funcImpl->params, funcImpl->returnType);
                if (!std::regex_search(oldFunctionKeyName, re)) {
                    continue;
                }

                std::string newFunctionKeyName = std::regex_replace(oldFunctionKeyName, re, replacement);
                std::string newFunctionName = Split(newFunctionKeyName, std::string(FUNCTION_DELIMITER))[0];
                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "old function key name:" << oldFunctionKeyName << std::endl;
                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "new function key name:" << newFunctionKeyName << std::endl;
                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "new function name:" << newFunctionName << std::endl;

                if (!ProgramUpdateFunctionTableKey(function->owningModule->file->GetStaticProgram(), oldFunctionKeyName,
                                                   newFunctionKeyName)) {
                    LIBABCKIT_LOG_NO_FUNC(ERROR) << funcName << "Failed to update function program table" << std::endl;
                    return false;
                }
                funcImpl->name = newFunctionName;
            }
            return true;
        },
        object);
}

bool libabckit::ModifyNameHelper::InterfaceRefreshObjectLiteralField(AbckitCoreInterface *iface,
                                                                     const std::string &oldName,
                                                                     const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;
    // Refresh main interface's object literals
    for (const auto &objectLiteral : iface->objectLiterals) {
        for (const auto &field : objectLiteral->fields) {
            if (ClassFieldGetNameStatic(field.get())->impl.data() == oldName) {
                FieldRefreshName(field.get(), newName);
                GetSetMethodRefreshName(objectLiteral.get(), oldName, newName);
            }
        }
    }

    // Refresh partial interface's object literals
    if (iface->partial != nullptr) {
        for (const auto &objectLiteral : iface->partial->objectLiterals) {
            for (const auto &field : objectLiteral->fields) {
                if (ClassFieldGetNameStatic(field.get())->impl.data() == oldName) {
                    FieldRefreshName(field.get(), newName);
                    GetSetMethodRefreshName(objectLiteral.get(), oldName, newName);
                }
            }
        }
    }

    return true;
}

bool libabckit::ModifyNameHelper::FieldRefreshTypeName(
    std::variant<AbckitCoreModuleField *, AbckitCoreNamespaceField *, AbckitCoreClassField *, AbckitCoreEnumField *,
                 AbckitCoreAnnotationInterfaceField *>
        field)
{
    LIBABCKIT_LOG_FUNC;
    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    return std::visit(
        [funcName](auto &object) {
            if (object->type->id != AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE) {
                return true;
            }

            auto &fieldType = object->GetArkTSImpl()->GetStaticImpl()->type;
            const auto oldName = fieldType.GetName();
            auto newName = StringUtil::GetTypeNameStr(object->type);
            newName = StringUtil::RemoveBracketsSuffix(newName);  // ark::pandasm::Type will add []
            LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "FieldRefreshTypeName : " << oldName << " --> " << newName
                                         << std::endl;
            fieldType = ark::pandasm::Type(newName, fieldType.GetRank());

            return true;
        },
        field);
}

bool libabckit::ModifyNameHelper::ObjectRefreshTypeUsersName(
    std::variant<AbckitCoreClass *, AbckitCoreEnum *, AbckitCoreInterface *> object, const std::string &newName)
{
    LIBABCKIT_LOG_FUNC;

    if (newName.empty()) {
        return false;
    }

    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    return std::visit(
        [newName, funcName](auto &coreObject) {
            for (const auto typeUser : coreObject->typeUsers) {
                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "type name:" << typeUser->name->impl.data() << std::endl;
                typeUser->name =
                    libabckit::CreateStringStatic(coreObject->owningModule->file, newName.data(), newName.length());

                for (auto function : typeUser->functionUsers) {
                    FunctionRefreshName(function);
                }

                for (const auto fieldTypeUser : typeUser->fieldTypeUsers) {
                    FieldRefreshTypeName(fieldTypeUser);
                }
            }
            return true;
        },
        object);
}
