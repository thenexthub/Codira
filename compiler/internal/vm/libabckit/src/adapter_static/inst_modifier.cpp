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

#include "inst_modifier.h"

#include "libabckit/src/logger.h"
#include "metadata_inspect_static.h"
#include "name_util.h"
#include "helpers_static.h"
#include "string_util.h"

using ark::pandasm::Opcode;

namespace {
using OpcodeList = std::vector<Opcode>;

/**
 * opcodes of using field_id ins
 */
const OpcodeList FIELD_INST_LIST = {Opcode::STSTATIC,
                                    Opcode::STSTATIC_64,
                                    Opcode::STSTATIC_OBJ,
                                    Opcode::STOBJ,
                                    Opcode::STOBJ_64,
                                    Opcode::STOBJ_OBJ,
                                    Opcode::STOBJ_V,
                                    Opcode::STOBJ_V_64,
                                    Opcode::STOBJ_V_OBJ,
                                    Opcode::LDSTATIC,
                                    Opcode::LDSTATIC_64,
                                    Opcode::LDSTATIC_OBJ,
                                    Opcode::LDOBJ,
                                    Opcode::LDOBJ_64,
                                    Opcode::LDOBJ_OBJ,
                                    Opcode::LDOBJ_V,
                                    Opcode::LDOBJ_V_64,
                                    Opcode::LDOBJ_V_OBJ,
                                    Opcode::ETS_LDOBJ_NAME_OBJ,
                                    Opcode::ETS_LDOBJ_NAME,
                                    Opcode::ETS_LDOBJ_NAME_64,
                                    Opcode::ETS_STOBJ_NAME,
                                    Opcode::ETS_STOBJ_NAME_64,
                                    Opcode::ETS_STOBJ_NAME_OBJ};

/**
 * opcodes of using type_id ins
 */
const OpcodeList CLASS_INST_LIST = {Opcode::NEWOBJ, Opcode::NEWARR, Opcode::ISINSTANCE, Opcode::CHECKCAST,
                                    Opcode::LDA_TYPE};

/**
 * opcodes of using method_id ins
 */
const OpcodeList METHOD_INST_LIST = {
    Opcode::INITOBJ,
    Opcode::INITOBJ_SHORT,
    Opcode::INITOBJ_RANGE,
    Opcode::CALL,
    Opcode::CALL_SHORT,
    Opcode::CALL_RANGE,
    Opcode::CALL_ACC,
    Opcode::CALL_ACC_SHORT,
    Opcode::CALL_VIRT,
    Opcode::CALL_VIRT_SHORT,
    Opcode::CALL_VIRT_RANGE,
    Opcode::CALL_VIRT_ACC,
    Opcode::CALL_VIRT_ACC_SHORT,
};

/**
 * check whether ins is in the opcode list
 */
bool InOpcodeList(const ark::pandasm::Ins &ins, const OpcodeList &list)
{
    return std::any_of(list.begin(), list.end(), [&](const auto &elem) { return elem == ins.opcode; });
}

constexpr auto ARRAY_SUFFIX = "[]";
constexpr size_t ARRAY_SUFFIX_LEN = 2;
constexpr size_t RESERVE_CAPACITY_FACTOR = 2;
constexpr auto UNION_PREFIX = "{U";
constexpr size_t UNION_PREFIX_LEN = 2;
constexpr auto UNION_SUFFIX = "}";
constexpr size_t UNION_SUFFIX_LEN = 1;
constexpr size_t UNION_PREFIX_SUFFIX_LEN = UNION_PREFIX_LEN + UNION_SUFFIX_LEN;
constexpr auto UNION_DELIMITER = ',';

bool IsUnion(const std::string &name)
{
    return name.rfind(UNION_PREFIX) == 0;
}

bool IsArray(const std::string &name)
{
    if (name.size() < ARRAY_SUFFIX_LEN) {
        return false;
    }
    return name.compare(name.length() - ARRAY_SUFFIX_LEN, ARRAY_SUFFIX_LEN, ARRAY_SUFFIX) == 0;
}

std::vector<std::string> ParseUnionTypes(const std::string &input)
{
    std::vector<std::string> types;

    std::string content = input.substr(UNION_PREFIX_LEN, input.length() - UNION_PREFIX_SUFFIX_LEN);

    std::istringstream iss(content);
    std::string type;
    while (std::getline(iss, type, UNION_DELIMITER)) {
        if (!type.empty()) {
            types.push_back(type);
        }
    }

    return types;
}

std::string JoinUnionTypes(const std::vector<std::string> &types)
{
    std::ostringstream oss;
    oss << UNION_PREFIX;

    for (size_t i = 0; i < types.size(); ++i) {
        if (i > 0) {
            oss << UNION_DELIMITER;
        }
        oss << types[i];
    }

    oss << UNION_SUFFIX;
    return oss.str();
}

ark::pandasm::Record *GetClassRecord(std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *> instance)
{
    return std::visit(
        [&](auto *object) {
            if (object == nullptr) {
                return static_cast<ark::pandasm::Record *>(nullptr);
            }
            // Check if this is an AbckitCoreClass and handle impl variant safely
            if constexpr (std::is_same_v<std::decay_t<decltype(object)>, AbckitCoreClass *>) {
                // Check if the impl variant contains a valid AbckitArktsClass
                if (std::holds_alternative<std::unique_ptr<AbckitArktsClass>>(object->impl)) {
                    auto arktsImpl = std::get<std::unique_ptr<AbckitArktsClass>>(object->impl).get();
                    if (arktsImpl != nullptr) {
                        return arktsImpl->impl.GetStaticClass();
                    }
                }
                return static_cast<ark::pandasm::Record *>(nullptr);
            } else {
                // For AbckitCoreInterface or AbckitCoreEnum, use the original logic
                return object->GetArkTSImpl()->impl.GetStaticClass();
            }
        },
        instance);
}

std::string GetClassRecordName(std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *> instance)
{
    return std::visit(
        [&](auto *object) {
            if (object == nullptr) {
                return std::string();
            }
            // Check if this is an AbckitCoreClass and handle impl variant safely
            if constexpr (std::is_same_v<std::decay_t<decltype(object)>, AbckitCoreClass *>) {
                // Check if the impl variant contains a valid AbckitArktsClass
                if (std::holds_alternative<std::unique_ptr<AbckitArktsClass>>(object->impl)) {
                    auto arktsImpl = std::get<std::unique_ptr<AbckitArktsClass>>(object->impl).get();
                    if (arktsImpl != nullptr) {
                        return arktsImpl->impl.GetStaticClass()->name;
                    }
                }
                return std::string();
            } else {
                // For AbckitCoreInterface or AbckitCoreEnum, use the original logic
                return object->GetArkTSImpl()->impl.GetStaticClass()->name;
            }
        },
        instance);
}

std::string GetArrayTypeNewName(AbckitFile *file, const std::string &oldName)
{
    std::string nameWithoutSuffix = oldName.substr(0, oldName.size() - 2);

    const auto prog = file->GetStaticProgram();
    auto recordEntry = prog->recordTable.find(nameWithoutSuffix);
    if (recordEntry != prog->recordTable.end()) {
        return recordEntry->second.name + ARRAY_SUFFIX;
    }

    auto it = file->nameToClass.find(nameWithoutSuffix);
    if (it == file->nameToClass.end()) {
        return oldName;
    }
    return GetClassRecordName(it->second) + ARRAY_SUFFIX;
}

std::string GetUnionTypeNewName(AbckitFile *file, const std::string &oldName)
{
    std::string newName = oldName;
    if (IsArray(oldName)) {
        newName = newName.substr(0, newName.size() - 2);
    }
    std::vector<std::string> types = ParseUnionTypes(newName);
    if (types.empty()) {
        return oldName;
    }
    for (auto &type : types) {
        if (file->nameToClass.find(type) != file->nameToClass.end()) {
            type = GetClassRecordName(file->nameToClass[type]);
        }
    }
    newName = JoinUnionTypes(types);
    if (IsArray(oldName)) {
        newName += ARRAY_SUFFIX;
    }
    auto it = file->nameToClass.find(oldName);
    if (it == file->nameToClass.end()) {
        return oldName;
    }
    const auto record = GetClassRecord(it->second);
    if (record == nullptr) {
        return oldName;
    }
    record->name = newName;
    return newName;
}
}  // namespace

void libabckit::InstModifier::EnumerateInst(const std::function<void(ark::pandasm::Ins &ins)> &visitor) const
{
    LIBABCKIT_LOG_FUNC;

    for (auto &[funcName, function] : this->file_->nameToFunctionStatic) {
        LIBABCKIT_LOG(DEBUG) << "Enumerate function inst for:" << funcName << std::endl;
        for (auto &ins : function->GetArkTSImpl()->GetStaticImpl()->ins) {
            visitor(ins);
        }
    }

    for (auto &[funcName, function] : this->file_->nameToFunctionInstance) {
        LIBABCKIT_LOG(DEBUG) << "Enumerate function inst for:" << funcName << std::endl;
        for (auto &ins : function->GetArkTSImpl()->GetStaticImpl()->ins) {
            visitor(ins);
        }
    }
}

AbckitCoreFunction *libabckit::InstModifier::GetFunction(const std::string &name) const
{
    if (this->file_->nameToFunctionStatic.find(name) != this->file_->nameToFunctionStatic.end()) {
        return this->file_->nameToFunctionStatic.at(name);
    }

    if (this->file_->nameToFunctionInstance.find(name) != this->file_->nameToFunctionInstance.end()) {
        return this->file_->nameToFunctionInstance.at(name);
    }

    return nullptr;
}

void libabckit::InstModifier::ModifyInstFunction(ark::pandasm::Ins &ins) const
{
    if (!ins.HasFlag(ark::pandasm::STATIC_METHOD_ID) && !ins.HasFlag(ark::pandasm::METHOD_ID) &&
        !InOpcodeList(ins, METHOD_INST_LIST)) {
        return;
    }

    LIBABCKIT_LOG_FUNC;

    auto oldName = ins.GetID(0);
    auto function = this->GetFunction(oldName);
    if (oldName.find("%%union_prop") != std::string::npos) {
        return;
    }

    // If GetFunction fails, it might be because the function name was changed
    // and the nameToFunction map key has been updated, or ProgramUpdateFunctionTableKey
    // has updated the functionTable key. In this case, we need to search through all
    // functions to find the one that matches the old name by checking if any function
    // in the functionTable (using any key) matches the instruction's old name context.
    // Since we can't directly match old names anymore, we'll search by iterating through
    // all functions and checking their underlying pandasm::Function pointers against
    // all entries in functionTable, looking for a match that would have had the old name.
    // However, a simpler approach: if GetFunction fails, it's likely the function
    // doesn't exist or is external, so we should just return (the instruction update
    // will be skipped). But if the function was renamed, the old name might still be
    // in the functionTable before ProgramUpdateFunctionTableKey, so try that first.
    if (function == nullptr) {
        const auto prog = this->file_->GetStaticProgram();
        // Try to find by oldName in functionTable (might still exist if ProgramUpdateFunctionTableKey
        // hasn't run, or if there's a timing issue)
        ark::pandasm::Function *pandasmFunc = nullptr;
        auto staticIt = prog->functionStaticTable.find(oldName);
        if (staticIt != prog->functionStaticTable.end()) {
            pandasmFunc = &staticIt->second;
        } else {
            auto instanceIt = prog->functionInstanceTable.find(oldName);
            if (instanceIt != prog->functionInstanceTable.end()) {
                pandasmFunc = &instanceIt->second;
            }
        }

        // If found in functionTable, find corresponding AbckitCoreFunction by pointer comparison
        if (pandasmFunc != nullptr) {
            for (const auto &[mapKey, func] : this->file_->nameToFunctionStatic) {
                if (func->GetArkTSImpl()->GetStaticImpl() == pandasmFunc) {
                    function = func;
                    break;
                }
            }
            if (function == nullptr) {
                for (const auto &[mapKey, func] : this->file_->nameToFunctionInstance) {
                    if (func->GetArkTSImpl()->GetStaticImpl() == pandasmFunc) {
                        function = func;
                        break;
                    }
                }
            }
        }
    }

    if (function == nullptr) {
        return;
    }

    if (FunctionIsExternalStatic(function)) {  // ignore external function
        return;
    }

    const auto newName = NameUtil::GetFullName(function);
    if (oldName == newName) {
        return;
    }

    LIBABCKIT_LOG(DEBUG) << "ins old function: " << oldName << std::endl;
    LIBABCKIT_LOG(DEBUG) << "ins new function: " << newName << std::endl;

    ins.SetID(0, newName);
}

void libabckit::InstModifier::ModifyInstField(ark::pandasm::Ins &ins) const
{
    if (!InOpcodeList(ins, FIELD_INST_LIST)) {
        return;
    }

    LIBABCKIT_LOG_FUNC;

    const auto oldName = ins.GetID(0);
    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    if (this->file_->nameToField.find(oldName) != this->file_->nameToField.end()) {
        std::visit(
            [&](auto *object) {
                const auto newName =
                    object->GetOwnerStaticImpl()->name + "." + object->GetArkTSImpl()->GetStaticImpl()->name;
                if (oldName == newName) {
                    return;
                }

                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "ins old fieldName: " << oldName << std::endl;
                LIBABCKIT_LOG_NO_FUNC(DEBUG) << funcName << "ins new fieldName: " << newName << std::endl;

                ins.SetID(0, newName);
            },
            this->file_->nameToField[oldName]);
    }
}

void libabckit::InstModifier::ModifyInstClass(ark::pandasm::Ins &ins) const
{
    if (!InOpcodeList(ins, CLASS_INST_LIST)) {
        return;
    }
    LIBABCKIT_LOG_FUNC;

    const std::string oldName = ins.GetID(0);
    const auto it = this->file_->nameToClass.find(oldName);

    // Handle ETSGLOBAL module remapping when class not found
    if (it == this->file_->nameToClass.end()) {
        auto lastDotPos = oldName.rfind('.');
        if (lastDotPos == std::string::npos) {
            return;
        }
        const std::string suffix = oldName.substr(lastDotPos + 1);
        if (suffix != "ETSGLOBAL") {
            return;
        }

        const std::string moduleName = oldName.substr(0, lastDotPos);

        // Try local modules first
        auto localModIt = this->file_->localModules.find(moduleName);
        if (localModIt != this->file_->localModules.end()) {
            const std::string newModuleName = localModIt->second->moduleName->impl.data();
            if (moduleName != newModuleName) {
                ins.SetID(0, newModuleName + "." + suffix);
            }
            return;
        }

        // Try external modules
        auto externalModIt = this->file_->externalModules.find(moduleName);
        if (externalModIt != this->file_->externalModules.end()) {
            const std::string newModuleName = externalModIt->second->moduleName->impl.data();
            if (moduleName != newModuleName) {
                ins.SetID(0, newModuleName + "." + suffix);
            }
            return;
        }
        return;
    }

    // Handle different type cases
    std::string newName;
    if (IsUnion(oldName)) {
        newName = GetUnionTypeNewName(this->file_, oldName);
        const auto record = GetClassRecord(it->second);
        if (record == nullptr) {
            return;
        }
        record->name = newName;
    } else if (IsArray(oldName)) {
        newName = GetArrayTypeNewName(this->file_, oldName);
        const auto prog = this->file_->GetStaticProgram();
        auto oldIt = prog->recordTable.find(oldName);
        if (oldIt != prog->recordTable.end() && prog->recordTable.find(newName) == prog->recordTable.end()) {
            auto entry = prog->recordTable.extract(oldName);
            entry.key() = newName;
            entry.mapped().name = newName;
            prog->recordTable.insert(std::move(entry));
        }
    } else {
        newName = GetClassRecordName(it->second);
    }

    if (newName.empty() || newName == oldName) {
        return;
    }

    LIBABCKIT_LOG(DEBUG) << "ins old className: " << oldName << std::endl;
    LIBABCKIT_LOG(DEBUG) << "ins new className: " << newName << std::endl;

    ins.SetID(0, newName);
}

void libabckit::InstModifier::RefreshFunctions(std::unordered_map<std::string, AbckitCoreFunction *> &nameToFunction)
{
    LIBABCKIT_LOG_FUNC;

    // Reserve space to prevent rehash during iteration which would invalidate iterators
    nameToFunction.reserve(nameToFunction.size() * RESERVE_CAPACITY_FACTOR);

    for (auto it = nameToFunction.begin(); it != nameToFunction.end();) {
        auto name = GetMangleFuncName(it->second);
        if (name != it->first) {
            LIBABCKIT_LOG(DEBUG) << "refresh file nameToFunction: " << it->first << " --> " << name << std::endl;
            auto node = nameToFunction.extract(it++);
            node.key() = name;
            nameToFunction.insert(std::move(node));
        } else {
            ++it;
        }
    }
}

void libabckit::InstModifier::RefreshFields()
{
    LIBABCKIT_LOG_FUNC;

    // Reserve space to prevent rehash during iteration which would invalidate iterators
    this->file_->nameToField.reserve(this->file_->nameToField.size() * RESERVE_CAPACITY_FACTOR);

    const auto funcName = StringUtil::GetFuncNameWithSquareBrackets(__func__);
    for (auto it = this->file_->nameToField.begin(); it != this->file_->nameToField.end();) {
        std::visit(
            [&](auto *object) {
                const auto name =
                    object->GetOwnerStaticImpl()->name + "." + object->GetArkTSImpl()->GetStaticImpl()->name;
                if (name != it->first) {
                    LIBABCKIT_LOG_NO_FUNC(DEBUG)
                        << funcName << "refresh file nameToField: " << it->first << " --> " << name << std::endl;
                    auto node = this->file_->nameToField.extract(it++);
                    node.key() = name;
                    this->file_->nameToField.insert(std::move(node));
                } else {
                    ++it;
                }
            },
            it->second);
    }
}

void libabckit::InstModifier::RefreshClasses()
{
    LIBABCKIT_LOG_FUNC;

    // Phase 1: Collect all updates (to avoid iterator invalidation)
    struct Update {
        std::string oldKey;
        std::string newKey;
        bool isUnion;
        bool isArray;
    };
    std::vector<Update> updates;

    for (const auto &[name, value] : this->file_->nameToClass) {
        // Skip partial and objectLiteral types
        if (name.find("%%partial-") != std::string::npos || name.find("$ObjectLiteral") != std::string::npos) {
            continue;
        }

        std::string newName;
        bool isUnion = IsUnion(name);
        bool isArray = IsArray(name);

        if (isUnion) {
            const auto prog = this->file_->GetStaticProgram();
            auto entry = prog->recordTable.find(name);
            if (entry != prog->recordTable.end()) {
                newName = entry->second.name;
            }
        } else if (isArray) {
            newName = GetArrayTypeNewName(this->file_, name);
        } else {
            const auto record = GetClassRecord(value);
            if (record != nullptr) {
                newName = record->name;
            }
        }

        if (!newName.empty() && newName != name) {
            updates.push_back({name, newName, isUnion, isArray});
        }
    }

    for (const auto &update : updates) {
        // Update nameToClass
        auto node = this->file_->nameToClass.extract(update.oldKey);
        if (!node.empty()) {
            node.key() = update.newKey;
            this->file_->nameToClass.insert(std::move(node));
        }

        // Update recordTable for Union types (Array types are already updated by modify_name_helper)
        if (update.isUnion) {
            const auto prog = this->file_->GetStaticProgram();
            auto entry = prog->recordTable.extract(update.oldKey);
            if (!entry.empty()) {
                entry.key() = update.newKey;
                prog->recordTable.insert(std::move(entry));
            }
        }
    }
}

void libabckit::InstModifier::Modify()
{
    LIBABCKIT_LOG_FUNC;

    ASSERT(this->file_ != nullptr);

    this->EnumerateInst([this](ark::pandasm::Ins &ins) {
        this->ModifyInstFunction(ins);
        this->ModifyInstField(ins);
        this->ModifyInstClass(ins);
    });

    RefreshFunctions(this->file_->nameToFunctionStatic);
    RefreshFunctions(this->file_->nameToFunctionInstance);

    this->RefreshFields();
    this->RefreshClasses();
}
