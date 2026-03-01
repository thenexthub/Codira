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

#include "assembly-emitter.h"
#include "libarkfile/file_items.h"
#include "libarkfile/file_writer.h"
#include "libarkfile/literal_data_accessor.h"
#include "mangling.h"
#include "libarkbase/os/file.h"
#include "runtime/include/profiling_gen.h"
#include "libarkfile/type_helper.h"

#include <algorithm>
#include <iostream>

namespace {

using ark::panda_file::AnnotationItem;
using ark::panda_file::ArrayValueItem;
using ark::panda_file::BaseClassItem;
using ark::panda_file::BaseFieldItem;
using ark::panda_file::BaseMethodItem;
using ark::panda_file::ClassItem;
using ark::panda_file::CodeItem;
using ark::panda_file::DebugInfoItem;
using ark::panda_file::FieldItem;
using ark::panda_file::FileWriter;
using ark::panda_file::ForeignClassItem;
using ark::panda_file::ForeignFieldItem;
using ark::panda_file::ForeignMethodItem;
using ark::panda_file::ItemContainer;
using ark::panda_file::MemoryBufferWriter;
using ark::panda_file::MethodHandleItem;
using ark::panda_file::MethodItem;
using ark::panda_file::MethodParamItem;
using ark::panda_file::ParamAnnotationsItem;
using ark::panda_file::PrimitiveTypeItem;
using ark::panda_file::ScalarValueItem;
using ark::panda_file::StringItem;
using ark::panda_file::Type;
using ark::panda_file::TypeItem;
using ark::panda_file::ValueItem;
using ark::panda_file::Writer;

std::unordered_map<Type::TypeId, PrimitiveTypeItem *> CreatePrimitiveTypes(ItemContainer *container)
{
    auto res = std::unordered_map<Type::TypeId, PrimitiveTypeItem *> {};
    res.insert({Type::TypeId::VOID, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID)});
    res.insert({Type::TypeId::U1, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U1)});
    res.insert({Type::TypeId::I8, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I8)});
    res.insert({Type::TypeId::U8, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U8)});
    res.insert({Type::TypeId::I16, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I16)});
    res.insert({Type::TypeId::U16, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U16)});
    res.insert({Type::TypeId::I32, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I32)});
    res.insert({Type::TypeId::U32, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U32)});
    res.insert({Type::TypeId::I64, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I64)});
    res.insert({Type::TypeId::U64, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U64)});
    res.insert({Type::TypeId::F32, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::F32)});
    res.insert({Type::TypeId::F64, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::F64)});
    res.insert({Type::TypeId::TAGGED, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::TAGGED)});
    return res;
}

template <class T>
typename T::mapped_type Find(const T &map, typename T::key_type key)
{
    auto res = map.find(key);
    ASSERT(res != map.end());
    return res != map.end() ? res->second : nullptr;
}

}  // anonymous namespace

namespace ark::pandasm {

/* static */
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::string AsmEmitter::lastError_ {};

static panda_file::Type::TypeId GetTypeId(Value::Type type)
{
    switch (type) {
        case Value::Type::U1:
            return panda_file::Type::TypeId::U1;
        case Value::Type::I8:
            return panda_file::Type::TypeId::I8;
        case Value::Type::U8:
            return panda_file::Type::TypeId::U8;
        case Value::Type::I16:
            return panda_file::Type::TypeId::I16;
        case Value::Type::U16:
            return panda_file::Type::TypeId::U16;
        case Value::Type::I32:
            return panda_file::Type::TypeId::I32;
        case Value::Type::U32:
            return panda_file::Type::TypeId::U32;
        case Value::Type::I64:
            return panda_file::Type::TypeId::I64;
        case Value::Type::U64:
            return panda_file::Type::TypeId::U64;
        case Value::Type::F32:
            return panda_file::Type::TypeId::F32;
        case Value::Type::F64:
            return panda_file::Type::TypeId::F64;
        case Value::Type::VOID:
            return panda_file::Type::TypeId::VOID;
        default:
            return panda_file::Type::TypeId::REFERENCE;
    }
}

/* static */
bool AsmEmitter::CheckValueType(Value::Type valueType, const Type &type, const Program &program)
{
    auto valueTypeId = GetTypeId(valueType);
    if (valueTypeId != type.GetId()) {
        SetLastError("Inconsistent element (" + AnnotationElement::TypeToString(valueType) +
                     ") and function's return type (" + type.GetName() + ")");
        return false;
    }

    switch (valueType) {
        case Value::Type::STRING:
        case Value::Type::RECORD:
        case Value::Type::ANNOTATION:
        case Value::Type::ENUM: {
            auto it = program.recordTable.find(type.GetName());
            if (it == program.recordTable.cend()) {
                SetLastError("Record " + type.GetName() + " not found");
                return false;
            }

            auto &record = it->second;
            if (valueType == Value::Type::ANNOTATION && !record.metadata->IsAnnotation() &&
                !record.metadata->IsRuntimeAnnotation() && !record.metadata->IsRuntimeTypeAnnotation() &&
                !record.metadata->IsTypeAnnotation()) {
                SetLastError("Record " + type.GetName() + " isn't annotation");
                return false;
            }

            if (valueType == Value::Type::ENUM && (record.metadata->GetAccessFlags() & ACC_ENUM) == 0) {
                SetLastError("Record " + type.GetName() + " isn't enum");
                return false;
            }

            break;
        }
        case Value::Type::ARRAY: {
            if (!type.IsArray()) {
                SetLastError("Inconsistent element (" + AnnotationElement::TypeToString(valueType) +
                             ") and function's return type (" + type.GetName() + ")");
                return false;
            }

            break;
        }
        default: {
            break;
        }
    }

    return true;
}

/* static */
std::string AsmEmitter::GetMethodSignatureFromProgram(const std::string &name, const Program &program)
{
    if (IsSignatureOrMangled(name)) {
        return name;
    }

    const auto itSynonym = program.functionSynonyms.find(name);
    const bool isMethodKnown = (itSynonym != program.functionSynonyms.end());
    const bool isSingleSynonym = (isMethodKnown && (itSynonym->second.size() == 1));
    if (isSingleSynonym) {
        return itSynonym->second[0];
    }
    SetLastError("More than one alternative for method " + name);
    return std::string("");
}

static panda_file::LiteralItem *CreateLiteralItemFromUint8(const Value *value,
                                                           std::vector<panda_file::LiteralItem> *out)
{
    auto v = value->GetAsScalar()->GetValue<uint8_t>();
    out->emplace_back(v);
    return &out->back();
}

static panda_file::LiteralItem *CreateLiteralItemFromUint16(const Value *value,
                                                            std::vector<panda_file::LiteralItem> *out)
{
    auto v = value->GetAsScalar()->GetValue<uint16_t>();
    out->emplace_back(v);
    return &out->back();
}

static panda_file::LiteralItem *CreateLiteralItemFromUint32(const Value *value,
                                                            std::vector<panda_file::LiteralItem> *out)
{
    auto v = value->GetAsScalar()->GetValue<uint32_t>();
    out->emplace_back(v);
    return &out->back();
}

static panda_file::LiteralItem *CreateLiteralItemFromUint64(const Value *value,
                                                            std::vector<panda_file::LiteralItem> *out)
{
    auto v = value->GetAsScalar()->GetValue<uint64_t>();
    out->emplace_back(v);
    return &out->back();
}

static panda_file::LiteralItem *CreateLiteralItemFromFloat(const Value *value,
                                                           std::vector<panda_file::LiteralItem> *out)
{
    auto v = bit_cast<uint32_t>(value->GetAsScalar()->GetValue<float>());
    out->emplace_back(v);
    return &out->back();
}

static panda_file::LiteralItem *CreateLiteralItemFromDouble(const Value *value,
                                                            std::vector<panda_file::LiteralItem> *out)
{
    auto v = bit_cast<uint64_t>(value->GetAsScalar()->GetValue<double>());
    out->emplace_back(v);
    return &out->back();
}

static panda_file::LiteralItem *CreateLiteralItemFromString(ItemContainer *container, const Value *value,
                                                            std::vector<panda_file::LiteralItem> *out)
{
    auto *stringItem = container->GetOrCreateStringItem(value->GetAsScalar()->GetValue<std::string>());
    out->emplace_back(stringItem);
    return &out->back();
}

/* static */
panda_file::LiteralItem *AsmEmitter::CreateLiteralItemFromMethod(const Value *value,
                                                                 std::vector<panda_file::LiteralItem> *out,
                                                                 const AsmEmitter::AsmEntityCollections &entities)
{
    auto name = value->GetAsScalar()->GetValue<std::string>();
    ASSERT(value->GetType() == Value::Type::METHOD);
    auto *methodItem = FindAmongAllMethods(name, entities, value);
    out->emplace_back(methodItem);
    return &out->back();
}

static panda_file::LiteralItem *CreateLiteralItemFromLiteralArray(const Value *value,
                                                                  std::vector<panda_file::LiteralItem> *out,
                                                                  const AsmEmitter::AsmEntityCollections &entities)
{
    auto key = value->GetAsScalar()->GetValue<std::string>();
    auto litItem = Find(entities.literalarrayItems, key);
    out->emplace_back(litItem);
    return &out->back();
}

// 重构后的 CreateLiteralItem 方法
panda_file::LiteralItem *AsmEmitter::CreateLiteralItem(ItemContainer *container, const Value *value,
                                                       std::vector<panda_file::LiteralItem> *out,
                                                       const AsmEmitter::AsmEntityCollections &entities)
{
    ASSERT(out != nullptr);

    auto valueType = value->GetType();

    switch (valueType) {
        case Value::Type::U1:
        case Value::Type::I8:
        case Value::Type::U8:
            return CreateLiteralItemFromUint8(value, out);
        case Value::Type::I16:
        case Value::Type::U16:
            return CreateLiteralItemFromUint16(value, out);
        case Value::Type::I32:
        case Value::Type::U32:
        case Value::Type::STRING_NULLPTR:
            return CreateLiteralItemFromUint32(value, out);
        case Value::Type::I64:
        case Value::Type::U64:
            return CreateLiteralItemFromUint64(value, out);
        case Value::Type::F32:
            return CreateLiteralItemFromFloat(value, out);
        case Value::Type::F64:
            return CreateLiteralItemFromDouble(value, out);
        case Value::Type::STRING:
            return CreateLiteralItemFromString(container, value, out);
        case Value::Type::METHOD:
            return CreateLiteralItemFromMethod(value, out, entities);
        case Value::Type::LITERALARRAY:
            return CreateLiteralItemFromLiteralArray(value, out, entities);
        default:
            return nullptr;
    }
}

/* static */
bool AsmEmitter::CheckValueRecordCase(const Value *value, const Program &program)
{
    auto t = value->GetAsScalar()->GetValue<Type>();
    if (!t.IsObject()) {
        return true;
    }

    auto recordName = t.GetName();
    bool isFound;
    if (t.IsArray()) {
        auto it = program.arrayTypes.find(t);
        isFound = it != program.arrayTypes.cend();
    } else {
        auto it = program.recordTable.find(recordName);
        isFound = it != program.recordTable.cend();
    }

    if (!isFound) {
        SetLastError("Incorrect value: record " + recordName + " not found");
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::CheckValueMethodCase(const Value *value, const Program &program)
{
    auto functionName = value->GetAsScalar()->GetValue<std::string>();
    const auto &functionTable =
        value->GetAsScalar()->IsStatic() ? program.functionStaticTable : program.functionInstanceTable;
    auto it = functionTable.find(functionName);
    if (it == functionTable.cend()) {
        SetLastError("Incorrect value: function " + functionName + " not found");
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::CheckValueEnumCase(const Value *value, const Type &type, const Program &program)
{
    auto enumValue = value->GetAsScalar()->GetValue<std::string>();
    auto recordName = GetOwnerName(enumValue);
    auto fieldName = GetItemName(enumValue);

    if (recordName != type.GetName()) {
        SetLastError("Incorrect value: Expected " + type.GetName() + " enum record");
        return false;
    }

    const auto &record = program.recordTable.find(recordName)->second;
    auto it = std::find_if(record.fieldList.cbegin(), record.fieldList.cend(),
                           [&fieldName](const Field &field) { return field.name == fieldName; });
    if (it == record.fieldList.cend()) {
        SetLastError("Incorrect value: Enum field " + enumValue + " not found");
        return false;
    }

    const auto &field = *it;
    if ((field.metadata->GetAccessFlags() & ACC_ENUM) == 0) {
        SetLastError("Incorrect value: Field " + enumValue + " isn't enum");
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::CheckValueArrayCase(const Value *value, const Type &type, const Program &program)
{
    auto componentType = type.GetComponentType();
    auto valueComponentType = value->GetAsArray()->GetComponentType();
    if (valueComponentType == Value::Type::VOID && value->GetAsArray()->GetValues().empty()) {
        return true;
    }

    if (!CheckValueType(valueComponentType, componentType, program)) {
        SetLastError("Incorrect array's component type: " + GetLastError());
        return false;
    }

    for (auto &elemValue : value->GetAsArray()->GetValues()) {
        if (!CheckValue(&elemValue, componentType, program)) {
            SetLastError("Incorrect array's element: " + GetLastError());
            return false;
        }
    }

    return true;
}

/* static */
bool AsmEmitter::CheckValue(const Value *value, const Type &type, const Program &program)
{
    auto valueType = value->GetType();
    if (!CheckValueType(valueType, type, program)) {
        SetLastError("Incorrect type: " + GetLastError());
        return false;
    }

    switch (valueType) {
        case Value::Type::RECORD: {
            if (!CheckValueRecordCase(value, program)) {
                return false;
            }

            break;
        }
        case Value::Type::METHOD: {
            if (!CheckValueMethodCase(value, program)) {
                return false;
            }

            break;
        }
        case Value::Type::ENUM: {
            if (!CheckValueEnumCase(value, type, program)) {
                return false;
            }

            break;
        }
        case Value::Type::ARRAY: {
            if (!CheckValueArrayCase(value, type, program)) {
                return false;
            }

            break;
        }
        default: {
            break;
        }
    }

    return true;
}

/* static */
ScalarValueItem *AsmEmitter::CreateScalarStringValueItem(ItemContainer *container, const Value *value,
                                                         std::vector<ScalarValueItem> *out)
{
    auto *stringItem = container->GetOrCreateStringItem(value->GetAsScalar()->GetValue<std::string>());
    if (out != nullptr) {
        out->emplace_back(stringItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(stringItem);
}

/* static */
ScalarValueItem *AsmEmitter::CreateScalarRecordValueItem(
    ItemContainer *container, const Value *value, std::vector<ScalarValueItem> *out,
    const std::unordered_map<std::string, BaseClassItem *> &classes)
{
    auto type = value->GetAsScalar()->GetValue<Type>();
    BaseClassItem *classItem;
    if (type.IsObject()) {
        auto name = type.GetName();
        auto it = classes.find(name);
        if (it == classes.cend()) {
            return nullptr;
        }

        classItem = it->second;
    } else {
        classItem = container->GetOrCreateForeignClassItem(type.GetDescriptor());
        ASSERT(classItem != nullptr);
    }

    if (out != nullptr) {
        out->emplace_back(classItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(classItem);
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
ScalarValueItem *AsmEmitter::CreateScalarMethodValueItem(ItemContainer *container, const Value *value,
                                                         std::vector<ScalarValueItem> *out, const Program &program,
                                                         const AsmEmitter::AsmEntityCollections &entities,
                                                         std::pair<bool, bool> searchInfo)
{
    auto name = value->GetAsScalar()->GetValue<std::string>();

    name = GetMethodSignatureFromProgram(name, program);

    BaseMethodItem *methodItem {nullptr};
    if (searchInfo.first) {
        auto &methods = searchInfo.second ? entities.staticMethodItems : entities.methodItems;
        auto it = methods.find(name);
        if (it != methods.cend()) {
            methodItem = it->second;
        }
    } else {
        methodItem = FindAmongAllMethods(name, entities, value);
    }
    if (methodItem == nullptr) {
        return nullptr;
    }

    if (out != nullptr) {
        out->emplace_back(methodItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(methodItem);
}

/* static */
ScalarValueItem *AsmEmitter::CreateScalarLiteralArrayItem(
    ItemContainer *container, const Value *value, std::vector<ScalarValueItem> *out,
    const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays)
{
    auto name = value->GetAsScalar()->GetValue<std::string>();
    auto it = literalarrays.find(name);
    ASSERT(it != literalarrays.end());
    auto *literalarrayItem = it->second;
    if (out != nullptr) {
        out->emplace_back(literalarrayItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(literalarrayItem);
}

/* static */
ScalarValueItem *AsmEmitter::CreateScalarEnumValueItem(ItemContainer *container, const Value *value,
                                                       std::vector<ScalarValueItem> *out,
                                                       const std::unordered_map<std::string, BaseFieldItem *> &fields)
{
    auto name = value->GetAsScalar()->GetValue<std::string>();
    auto it = fields.find(name);
    if (it == fields.cend()) {
        return nullptr;
    }

    auto *fieldItem = it->second;
    if (out != nullptr) {
        out->emplace_back(fieldItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(fieldItem);
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
ScalarValueItem *AsmEmitter::CreateScalarAnnotationValueItem(ItemContainer *container, const Value *value,
                                                             std::vector<ScalarValueItem> *out, const Program &program,
                                                             const AsmEmitter::AsmEntityCollections &entities)
{
    auto annotation = value->GetAsScalar()->GetValue<AnnotationData>();
    auto *annotationItem = CreateAnnotationItem(container, annotation, program, entities);
    if (annotationItem == nullptr) {
        return nullptr;
    }

    if (out != nullptr) {
        out->emplace_back(annotationItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(annotationItem);
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
ScalarValueItem *AsmEmitter::CreateScalarValueItem(ItemContainer *container, const Value *value,
                                                   std::vector<ScalarValueItem> *out, const Program &program,
                                                   const AsmEmitter::AsmEntityCollections &entities,
                                                   std::pair<bool, bool> searchInfo)
{
    auto valueType = value->GetType();

    switch (valueType) {
        case Value::Type::U1:
        case Value::Type::I8:
        case Value::Type::U8:
        case Value::Type::I16:
        case Value::Type::U16:
        case Value::Type::I32:
        case Value::Type::U32:
        case Value::Type::STRING_NULLPTR: {
            return CreateScalarPrimValueItem<uint32_t>(container, value, out);
        }
        case Value::Type::I64:
        case Value::Type::U64: {
            return CreateScalarPrimValueItem<uint64_t>(container, value, out);
        }
        case Value::Type::F32: {
            return CreateScalarPrimValueItem<float>(container, value, out);
        }
        case Value::Type::F64: {
            return CreateScalarPrimValueItem<double>(container, value, out);
        }
        case Value::Type::STRING: {
            return CreateScalarStringValueItem(container, value, out);
        }
        case Value::Type::RECORD: {
            return CreateScalarRecordValueItem(container, value, out, entities.classItems);
        }
        case Value::Type::METHOD: {
            return CreateScalarMethodValueItem(container, value, out, program, entities, searchInfo);
        }
        case Value::Type::ENUM: {
            return CreateScalarEnumValueItem(container, value, out, entities.fieldItems);
        }
        case Value::Type::ANNOTATION: {
            return CreateScalarAnnotationValueItem(container, value, out, program, entities);
        }
        case Value::Type::LITERALARRAY: {
            return CreateScalarLiteralArrayItem(container, value, out, entities.literalarrayItems);
        }
        default: {
            UNREACHABLE();
            return nullptr;
        }
    }
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
ValueItem *AsmEmitter::CreateValueItem(ItemContainer *container, const Value *value, const Program &program,
                                       const AsmEmitter::AsmEntityCollections &entities,
                                       std::pair<bool, bool> searchInfo)
{
    switch (value->GetType()) {
        case Value::Type::ARRAY: {
            std::vector<ScalarValueItem> elements;
            for (const auto &elemValue : value->GetAsArray()->GetValues()) {
                auto *item = CreateScalarValueItem(container, &elemValue, &elements, program, entities, searchInfo);
                if (item == nullptr) {
                    return nullptr;
                }
            }

            auto componentType = value->GetAsArray()->GetComponentType();
            return container->CreateItem<ArrayValueItem>(panda_file::Type(GetTypeId(componentType)),
                                                         std::move(elements));
        }
        default: {
            return CreateScalarValueItem(container, value, nullptr, program, entities, searchInfo);
        }
    }
}

/* static */
// CC-OFFNXT(G.FMT.10-CPP) project code style
std::optional<std::map<std::basic_string<char>, ark::pandasm::Function>::const_iterator>
AsmEmitter::GetMethodForAnnotationElement(const std::string &functionName, const Value *value, const Program &program)
{
    if (value->GetType() != Value::Type::METHOD) {
        return program.FindAmongAllFunctions(functionName);
    }
    const auto &functionTable =
        value->GetAsScalar()->IsStatic() ? program.functionStaticTable : program.functionInstanceTable;
    auto funcIt = functionTable.find(functionName);
    if (funcIt == functionTable.cend()) {
        return std::nullopt;
    }
    return funcIt;
}

/* static */
AnnotationItem *AsmEmitter::CreateAnnotationItem(ItemContainer *container, const AnnotationData &annotation,
                                                 const Program &program,
                                                 const AsmEmitter::AsmEntityCollections &entities)
{
    auto recordName = annotation.GetName();
    auto it = program.recordTable.find(recordName);
    if (it == program.recordTable.cend()) {
        SetLastError("Record " + recordName + " not found");
        return nullptr;
    }

    auto &record = it->second;
    if (!record.metadata->IsAnnotation()) {
        SetLastError("Record " + recordName + " isn't annotation");
        return nullptr;
    }

    std::vector<AnnotationItem::Elem> itemElements;
    std::vector<AnnotationItem::Tag> tagElements;

    for (const auto &element : annotation.GetElements()) {
        auto name = element.GetName();
        auto *value = element.GetValue();

        auto valueType = value->GetType();

        uint8_t tagType;

        if (valueType == Value::Type::ARRAY && !value->GetAsArray()->GetValues().empty()) {
            auto arrayElementType = value->GetAsArray()->GetComponentType();
            tagType = Value::GetArrayTypeAsChar(arrayElementType);
        } else {
            tagType = Value::GetTypeAsChar(valueType);
        }

        ASSERT(tagType != '0');

        auto functionName = GetMethodSignatureFromProgram(record.name + "." + name, program);

        auto foundFunc = GetMethodForAnnotationElement(functionName, value, program);
        if (record.HasImplementation() && !foundFunc.has_value()) {
            // Definitions of the system annotations may be absent.
            // So print message and continue if corresponding function isn't found.
            LOG(INFO, ASSEMBLER) << "Function " << functionName << " not found";
        } else if (record.HasImplementation() && !CheckValue(value, foundFunc.value()->second.returnType, program)) {
            SetLastError("Incorrect annotation element " + functionName + ": " + GetLastError());
            return nullptr;
        }
        ValueItem *item {nullptr};
        if (foundFunc.has_value()) {
            item = CreateValueItem(container, value, program, entities, {true, foundFunc.value()->second.IsStatic()});
        } else {
            item = CreateValueItem(container, value, program, entities);
        }

        if (item == nullptr) {
            SetLastError("Cannot create value item for annotation element " + functionName + ": " + GetLastError());
            return nullptr;
        }

        itemElements.emplace_back(container->GetOrCreateStringItem(name), item);
        tagElements.emplace_back(tagType);
    }

    auto *cls = entities.classItems.find(recordName)->second;
    return container->CreateItem<AnnotationItem>(cls, std::move(itemElements), std::move(tagElements));
}

MethodHandleItem *AsmEmitter::CreateMethodHandleItem(ItemContainer *container, const MethodHandle &mh,
                                                     const std::unordered_map<std::string, BaseFieldItem *> &fields,
                                                     const std::unordered_map<std::string, BaseMethodItem *> &methods)
{
    MethodHandleItem *item = nullptr;
    switch (mh.type) {
        case panda_file::MethodHandleType::PUT_STATIC:
        case panda_file::MethodHandleType::GET_STATIC:
        case panda_file::MethodHandleType::PUT_INSTANCE:
        case panda_file::MethodHandleType::GET_INSTANCE: {
            item = container->CreateItem<MethodHandleItem>(mh.type, fields.at(mh.itemName));
            break;
        }
        case panda_file::MethodHandleType::INVOKE_STATIC:
        case panda_file::MethodHandleType::INVOKE_INSTANCE:
        case panda_file::MethodHandleType::INVOKE_CONSTRUCTOR:
        case panda_file::MethodHandleType::INVOKE_DIRECT:
        case panda_file::MethodHandleType::INVOKE_INTERFACE: {
            item = container->CreateItem<MethodHandleItem>(mh.type, methods.at(mh.itemName));
            break;
        }
        default:
            UNREACHABLE();
            break;
    }
    return item;
}

/* static */
template <class T>
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::AddAnnotations(T *item, ItemContainer *container, const AnnotationMetadata &metadata,
                                const Program &program, const AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &annotation : metadata.GetAnnotations()) {
        auto *annotationItem = CreateAnnotationItem(container, annotation, program, entities);
        if (annotationItem == nullptr) {
            return false;
        }

        auto &record = program.recordTable.find(annotation.GetName())->second;
        if (record.metadata->IsRuntimeAnnotation()) {
            item->AddRuntimeAnnotation(annotationItem);
        } else if (record.metadata->IsAnnotation()) {
            item->AddAnnotation(annotationItem);
        } else if (record.metadata->IsRuntimeTypeAnnotation()) {
            item->AddRuntimeTypeAnnotation(annotationItem);
        } else if (record.metadata->IsTypeAnnotation()) {
            item->AddTypeAnnotation(annotationItem);
        }
    }

    return true;
}

template <class T>
static void AddBytecodeIndexDependencies(MethodItem *method, const Ins &insn,
                                         const std::unordered_map<std::string, T *> &items,
                                         const AsmEmitter::AsmEntityCollections &entities, bool lookupInStatic = true)
{
    ASSERT(!insn.IDEmpty());

    for (std::size_t i = 0U; i < insn.IDSize(); ++i) {
        const auto &id = insn.GetID(i);
        auto it = items.find(id);
        if (it == items.cend()) {
            if (lookupInStatic && insn.HasFlag(InstFlags::STATIC_METHOD_ID)) {
                AddBytecodeIndexDependencies(method, insn, entities.methodItems, entities, false);
                return;
            }
#ifdef PANDA_WITH_ECMASCRIPT
            AddBytecodeIndexDependencies(method, insn, entities.staticMethodItems, entities);
            return;
#endif
            if (insn.opcode == pandasm::Opcode::INITOBJ_SHORT || insn.opcode == pandasm::Opcode::INITOBJ_RANGE ||
                insn.opcode == pandasm::Opcode::INITOBJ) {
                AddBytecodeIndexDependencies(method, insn, entities.staticMethodItems, entities);
                return;
            }
            UNREACHABLE();
        }
        ASSERT_PRINT(it != items.cend(), "Symbol '" << id << "' not found");

        auto *item = it->second;
        ASSERT(item->GetIndexType() != panda_file::IndexType::NONE);
        method->AddIndexDependency(item);
    }
}

static void AddBytecodeIndexDependencies(MethodItem *method, const Function &func,
                                         const AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &insn : func.ins) {
        if (insn.opcode == Opcode::INVALID) {
            continue;
        }

        bool isDevirtualCall = insn.IsDevirtual() && insn.HasFlag(InstFlags::STATIC_METHOD_ID);
        if (insn.HasFlag(InstFlags::METHOD_ID) || isDevirtualCall) {
            AddBytecodeIndexDependencies(method, insn, entities.methodItems, entities);
            continue;
        }

        if (insn.HasFlag(InstFlags::STATIC_METHOD_ID)) {
            AddBytecodeIndexDependencies(method, insn, entities.staticMethodItems, entities);
            continue;
        }

        if (insn.HasFlag(InstFlags::FIELD_ID)) {
            AddBytecodeIndexDependencies(method, insn, entities.fieldItems, entities);
            continue;
        }

        if (insn.HasFlag(InstFlags::STATIC_FIELD_ID)) {
            AddBytecodeIndexDependencies(method, insn, entities.staticFieldItems, entities);
            continue;
        }

        if (insn.HasFlag(InstFlags::TYPE_ID)) {
            AddBytecodeIndexDependencies(method, insn, entities.classItems, entities);
            continue;
        }
    }

    for (const auto &catchBlock : func.catchBlocks) {
        if (catchBlock.exceptionRecord.empty()) {
            continue;
        }

        auto it = entities.classItems.find(catchBlock.exceptionRecord);
        ASSERT(it != entities.classItems.cend());
        auto *item = it->second;
        ASSERT(item->GetIndexType() != panda_file::IndexType::NONE);
        method->AddIndexDependency(item);
    }
}

/* static */
void AsmEmitter::MakeStringItems(ItemContainer *items, const Program &program,
                                 AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &s : program.strings) {
        auto *item = items->GetOrCreateStringItem(s);
        entities.stringItems.insert({s, item});
    }
}

template <Value::Type TYPE, typename CType = ValueTypeHelperT<TYPE>>
static ScalarValue CreateValue(const LiteralArray::Literal &literal)
{
    if constexpr (std::is_same_v<CType, ValueTypeHelperT<TYPE>> && std::is_integral_v<CType>) {
        return ScalarValue::Create<TYPE>(std::get<std::make_unsigned_t<CType>>(literal.value));
    } else {
        return ScalarValue::Create<TYPE>(std::get<CType>(literal.value));
    }
}

template <Value::Type TYPE, typename... T>
static ScalarValue CheckAndCreateArrayValue(const LiteralArray::Literal &literal,
                                            [[maybe_unused]] const Program &program)
{
    ASSERT(program.arrayTypes.find(Type(AnnotationElement::TypeToString(TYPE), 1)) != program.arrayTypes.end());
    return CreateValue<TYPE, T...>(literal);
}

static ScalarValue MakeLiteralItemArrayValue(const LiteralArray::Literal &literal, const Program &program)
{
    switch (literal.tag) {
        case panda_file::LiteralTag::ARRAY_U1:
            return CheckAndCreateArrayValue<Value::Type::U1, bool>(literal, program);
        case panda_file::LiteralTag::ARRAY_U8:
            return CheckAndCreateArrayValue<Value::Type::U8>(literal, program);
        case panda_file::LiteralTag::ARRAY_I8:
            return CheckAndCreateArrayValue<Value::Type::I8>(literal, program);
        case panda_file::LiteralTag::ARRAY_U16:
            return CheckAndCreateArrayValue<Value::Type::U16>(literal, program);
        case panda_file::LiteralTag::ARRAY_I16:
            return CheckAndCreateArrayValue<Value::Type::I16>(literal, program);
        case panda_file::LiteralTag::ARRAY_U32:
            return CheckAndCreateArrayValue<Value::Type::U32>(literal, program);
        case panda_file::LiteralTag::ARRAY_I32:
            return CheckAndCreateArrayValue<Value::Type::I32>(literal, program);
        case panda_file::LiteralTag::ARRAY_U64:
            return CheckAndCreateArrayValue<Value::Type::U64>(literal, program);
        case panda_file::LiteralTag::ARRAY_I64:
            return CheckAndCreateArrayValue<Value::Type::I64>(literal, program);
        case panda_file::LiteralTag::ARRAY_F32:
            return CheckAndCreateArrayValue<Value::Type::F32>(literal, program);
        case panda_file::LiteralTag::ARRAY_F64:
            return CheckAndCreateArrayValue<Value::Type::F64>(literal, program);
        case panda_file::LiteralTag::ARRAY_STRING: {
            [[maybe_unused]] auto stringType =
                Type::FromDescriptor(ark::panda_file::GetStringClassDescriptor(program.lang));
            // `arrayTypes` may contain class name both with / and . depending on source language (workaround
            // for #5776)
            ASSERT(program.arrayTypes.find(Type(stringType, 1)) != program.arrayTypes.end() ||
                   program.arrayTypes.find(Type(stringType.GetPandasmName(), 1)) != program.arrayTypes.end());
            return CreateValue<Value::Type::STRING, std::string>(literal);
        }
        default:
            UNREACHABLE();
    }
}

static ScalarValue MakeLiteralItemValue(const LiteralArray::Literal &literal, const Program &program)
{
    if (literal.IsArray()) {
        return MakeLiteralItemArrayValue(literal, program);
    }
    switch (literal.tag) {
        case panda_file::LiteralTag::TAGVALUE:
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE:
            return CreateValue<Value::Type::U8>(literal);
        case panda_file::LiteralTag::BOOL:
            return CreateValue<Value::Type::U8, bool>(literal);
        case panda_file::LiteralTag::METHODAFFILIATE:
            return CreateValue<Value::Type::U16>(literal);
        case panda_file::LiteralTag::INTEGER:
            return CreateValue<Value::Type::I32>(literal);
        case panda_file::LiteralTag::BIGINT: {
            return CreateValue<Value::Type::I64>(literal);
        }
        case panda_file::LiteralTag::FLOAT:
            return CreateValue<Value::Type::F32>(literal);
        case panda_file::LiteralTag::DOUBLE:
            return CreateValue<Value::Type::F64>(literal);
        case panda_file::LiteralTag::STRING:
            return CreateValue<Value::Type::STRING, std::string>(literal);
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCMETHOD:
            return CreateValue<Value::Type::METHOD, std::string>(literal);
        case panda_file::LiteralTag::LITERALARRAY:
            return CreateValue<Value::Type::LITERALARRAY, std::string>(literal);
        default:
            UNREACHABLE();
    }
}

static std::vector<std::pair<std::string, ark::pandasm::LiteralArray>> SortByLastWord(
    const std::map<std::string, ark::pandasm::LiteralArray> &literalarrayTable)
{
    std::vector<std::pair<std::string, ark::pandasm::LiteralArray>> items(literalarrayTable.begin(),
                                                                          literalarrayTable.end());

    std::sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
        size_t posA = a.first.rfind('-');
        size_t posB = b.first.rfind('-');

        std::string lastNumberA = (posA != std::string::npos) ? a.first.substr(posA + 1) : "";
        std::string lastNumberB = (posB != std::string::npos) ? b.first.substr(posB + 1) : "";

        auto stringToInt = [](const std::string &str) {
            int num = 0;
            std::stringstream ss(str);
            ss >> num;
            return num;
        };

        return stringToInt(lastNumberA) < stringToInt(lastNumberB);
    });

    return items;
}

/* static */
void AsmEmitter::MakeLiteralItems(ItemContainer *items, const Program &program,
                                  AsmEmitter::AsmEntityCollections &entities)
{
    // The insertion order of the literal arrays has been disrupted by the map, but this order is crucial
    // (as the root array stores the offsets of the sub-arrays). Therefore, we utilize a sorted vector
    // to restore the insertion order for iteration.
    auto sortedTable = SortByLastWord(program.literalarrayTable);
    for (const auto &[id, l] : sortedTable) {
        auto literalArrayItem = items->GetOrCreateLiteralArrayItem(id);
        std::vector<panda_file::LiteralItem> literalArray;

        for (auto &literal : l.literals) {
            auto value = MakeLiteralItemValue(literal, program);
            // the return pointer of vector element should not be rewrited
            CreateLiteralItem(items, &value, &literalArray, entities);
        }

        literalArrayItem->AddItems(literalArray);
        entities.literalarrayItems.insert({id, literalArrayItem});
    }
}

/* static */
void AsmEmitter::MakeArrayTypeItems(ItemContainer *items, const Program &program,
                                    AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &t : program.arrayTypes) {
        auto *foreignRecord = items->GetOrCreateForeignClassItem(t.GetDescriptor());
        ASSERT(foreignRecord != nullptr);
        entities.classItems.insert({t.GetName(), foreignRecord});
    }
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleRecordAsForeign(
    ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes, const std::string &name,
    const Record &rec)
{
    Type recordType = Type::FromName(name);
    auto *foreignRecord = items->GetOrCreateForeignClassItem(recordType.GetDescriptor(rec.conflict));
    if (foreignRecord == nullptr) {
        SetLastError("'" + name + "' is already defined.");
        return false;
    }
    entities.classItems.insert({name, foreignRecord});
    for (const auto &f : rec.fieldList) {
        ASSERT(f.metadata->IsForeign());
        auto *fieldName = items->GetOrCreateStringItem(pandasm::DeMangleName(f.name));
        std::string fullFieldName = name + "." + f.name;
        if (!f.metadata->IsForeign()) {
            SetLastError("External record " + name + " has a non-external field " + f.name);
            return false;
        }
        auto *typeItem = GetTypeItem(items, primitiveTypes, f.type, program);
        if (typeItem == nullptr) {
            SetLastError("Field " + fullFieldName + " has undefined type");
            return false;
        }
        auto *field =
            items->CreateItem<ForeignFieldItem>(foreignRecord, fieldName, typeItem, f.metadata->GetAccessFlags());
        UpdateFieldList(entities, fullFieldName, field);
    }
    return true;
}

/* static */
bool AsmEmitter::HandleBaseRecord(ItemContainer *items, const Program &program, const std::string &name,
                                  const Record &baseRec, ClassItem *record)
{
    auto baseName = baseRec.metadata->GetBase();
    if (!baseName.empty()) {
        auto it = program.recordTable.find(baseName);
        if (it == program.recordTable.cend()) {
            SetLastError("Base record " + baseName + " is not defined for record " + name);
            return false;
        }
        auto &rec = it->second;
        Type baseType(baseName, 0);
        if (rec.metadata->IsForeign()) {
            auto item = items->GetOrCreateForeignClassItem(baseType.GetDescriptor(rec.conflict));
            ASSERT(item != nullptr);
            record->SetSuperClass(item);
        } else {
            record->SetSuperClass(items->GetOrCreateClassItem(baseType.GetDescriptor(rec.conflict)));
        }
    }
    return true;
}

/* static */
bool AsmEmitter::HandleInterfaces(ItemContainer *items, const Program &program, const std::string &name,
                                  const Record &rec, ClassItem *record)
{
    auto ifaces = rec.metadata->GetInterfaces();
    for (const auto &item : ifaces) {
        auto it = program.recordTable.find(item);
        if (it == program.recordTable.cend()) {
            std::stringstream error;
            error << "Interface record " << item << " is not defined for record " << name;
            SetLastError(error.str());
            return false;
        }
        auto &iface = it->second;
        Type ifaceType(item, 0);
        if (iface.metadata->IsForeign()) {
            auto foreignItem = items->GetOrCreateForeignClassItem(ifaceType.GetDescriptor(iface.conflict));
            ASSERT(foreignItem);
            record->AddInterface(foreignItem);
        } else {
            record->AddInterface(items->GetOrCreateClassItem(ifaceType.GetDescriptor(iface.conflict)));
        }
    }
    return true;
}

/* static */
void AsmEmitter::UpdateFieldList(AsmEmitter::AsmEntityCollections &entities, const std::string &fullFieldName,
                                 panda_file::BaseFieldItem *field)
{
    if (field->IsStatic()) {
        entities.staticFieldItems.emplace(fullFieldName, field);
    } else {
        entities.fieldItems.emplace(fullFieldName, field);
    }
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleFields(ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
                              const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes,
                              const std::string &name, const Record &rec, ClassItem *record)
{
    for (const auto &f : rec.fieldList) {
        auto *fieldName = items->GetOrCreateStringItem(pandasm::DeMangleName(f.name));
        std::string fullFieldName = name + '.' + f.name;
        auto *typeItem = GetTypeItem(items, primitiveTypes, f.type, program);
        if (typeItem == nullptr) {
            SetLastError("Field " + fullFieldName + " has undefined type");
            return false;
        }
        BaseFieldItem *field;
        if (f.metadata->IsForeign()) {
            field = items->CreateItem<ForeignFieldItem>(record, fieldName, typeItem, f.metadata->GetAccessFlags());
        } else {
            field = record->AddField(fieldName, typeItem, f.metadata->GetAccessFlags());
        }
        UpdateFieldList(entities, fullFieldName, field);
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleRecord(ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
                              const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes,
                              const std::string &name, const Record &rec)
{
    Type recordType = Type::FromName(name);
    auto *record = items->GetOrCreateClassItem(recordType.GetDescriptor(rec.conflict));
    entities.classItems.insert({name, record});
    if (rec.metadata != nullptr) {
        record->SetAccessFlags(rec.metadata->GetAccessFlags());
    }
    record->SetSourceLang(rec.language);

    if (!rec.sourceFile.empty()) {
        auto *sourceFileItem = items->GetOrCreateStringItem(rec.sourceFile);
        record->SetSourceFile(sourceFileItem);
    }

    if (!HandleBaseRecord(items, program, name, rec, record)) {
        return false;
    }

    if (!HandleInterfaces(items, program, name, rec, record)) {
        return false;
    }

    if (!HandleFields(items, program, entities, primitiveTypes, name, rec, record)) {
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::MakeRecordItems(
    ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes)
{
    for (const auto &[name, rec] : program.recordTable) {
        if (rec.metadata->IsForeign()) {
            if (!HandleRecordAsForeign(items, program, entities, primitiveTypes, name, rec)) {
                return false;
            }
        } else {
            if (!HandleRecord(items, program, entities, primitiveTypes, name, rec)) {
                return false;
            }
        }
    }
    return true;
}

/* static */
StringItem *AsmEmitter::GetMethodName(ItemContainer *items, const Function &func, const std::string &name)
{
    if (func.metadata->IsCtor()) {
        return items->GetOrCreateStringItem(ark::panda_file::GetCtorName(func.language));
    }

    if (func.metadata->IsCctor()) {
        return items->GetOrCreateStringItem(ark::panda_file::GetCctorName(func.language));
    }

    return items->GetOrCreateStringItem(GetItemName(name));
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleAreaForInner(ItemContainer *items, const Program &program, ClassItem **area,
                                    ForeignClassItem **foreignArea, const std::string &name,
                                    const std::string &recordOwnerName)
{
    auto iter = program.recordTable.find(recordOwnerName);
    if (iter != program.recordTable.end()) {
        auto &rec = iter->second;
        Type recordOwnerType = Type::FromName(recordOwnerName);
        auto descriptor = recordOwnerType.GetDescriptor(rec.conflict);
        if (rec.metadata->IsForeign()) {
            *foreignArea = items->GetOrCreateForeignClassItem(descriptor);
            if (*foreignArea == nullptr) {
                SetLastError("Unable to create external record " + iter->first);
                return false;
            }
        } else {
            *area = items->GetOrCreateClassItem(descriptor);
            (*area)->SetAccessFlags(rec.metadata->GetAccessFlags());
        }
    } else {
        SetLastError("Function " + name + " is bound to undefined record " + recordOwnerName);
        return false;
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleRecordOnwer(ItemContainer *items, const Program &program, ClassItem **area,
                                   ForeignClassItem **foreignArea, const std::string &name,
                                   const std::string &recordOwnerName)
{
    if (recordOwnerName.empty()) {
        *area = items->GetOrCreateGlobalClassItem();
        (*area)->SetAccessFlags(ACC_PUBLIC);
        (*area)->SetSourceLang(program.lang);
    } else {
        if (!HandleAreaForInner(items, program, area, foreignArea, name, recordOwnerName)) {
            return false;
        }
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleFunctionParams(
    ItemContainer *items, const Program &program, size_t idx, const std::string &name, const Function &func,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes,
    std::vector<MethodParamItem> &params)
{
    for (size_t i = idx; i < func.params.size(); i++) {
        const auto &p = func.params[i].type;
        auto *typeItem = GetTypeItem(items, primitiveTypes, p, program);
        if (typeItem == nullptr) {
            SetLastError("Argument " + std::to_string(i) + " of function " + name + " has undefined type");
            return false;
        }
        params.emplace_back(typeItem);
    }
    return true;
}

/* static */
bool AsmEmitter::HandleFunctionLocalVariables(ItemContainer *items, const Function &func, const std::string &name)
{
    for (const auto &v : func.localVariableDebug) {
        if (v.name.empty()) {
            SetLastError("Function '" + name + "' has an empty local variable name");
            return false;
        }
        if (v.signature.empty()) {
            SetLastError("Function '" + name + "' has an empty local variable signature");
            return false;
        }
        items->GetOrCreateStringItem(v.name);
        // Skip signature and signature type for parameters
        ASSERT(v.reg >= 0);
        if (func.IsParameter(v.reg)) {
            continue;
        }
        items->GetOrCreateStringItem(v.signature);
        if (!v.signatureType.empty()) {
            items->GetOrCreateStringItem(v.signatureType);
        }
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::CreateMethodItem(ItemContainer *items, AsmEmitter::AsmEntityCollections &entities,
                                  const Function &func, TypeItem *typeItem, ClassItem *area,
                                  ForeignClassItem *foreignArea, StringItem *methodName, const std::string &mangledName,
                                  const std::string &name, std::vector<MethodParamItem> &params)
{
    auto accessFlags = func.metadata->GetAccessFlags();
    auto *proto = items->GetOrCreateProtoItem(typeItem, params);
    BaseMethodItem *method;
    if (foreignArea == nullptr) {
        if (func.metadata->IsForeign()) {
            method = items->CreateItem<ForeignMethodItem>(area, methodName, proto, accessFlags);
        } else {
            method = area->AddMethod(methodName, proto, accessFlags, params);
        }
    } else {
        if (!func.metadata->IsForeign()) {
            SetLastError("Non-external function " + name + " is bound to external record");
            return false;
        }
        method = items->CreateItem<ForeignMethodItem>(foreignArea, methodName, proto, accessFlags);
    }
    if (func.IsStatic()) {
        entities.staticMethodItems.insert({mangledName, method});
    } else {
        entities.methodItems.insert({mangledName, method});
    }
    if (!func.metadata->IsForeign() && func.metadata->HasImplementation()) {
        if (!func.sourceFile.empty()) {
            items->GetOrCreateStringItem(func.sourceFile);
        }
        if (!func.sourceCode.empty()) {
            items->GetOrCreateStringItem(func.sourceCode);
        }
    }
    return true;
}

/* static */
bool AsmEmitter::MakeFunctionItems(
    ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes, bool emitDebugInfo,
    bool isStatic)
{
    auto &functionTable = isStatic ? program.functionStaticTable : program.functionInstanceTable;
    for (const auto &f : functionTable) {
        const auto &[mangled_name, func] = f;

        auto name = pandasm::DeMangleName(mangled_name);

        StringItem *methodName = GetMethodName(items, func, name);

        ClassItem *area = nullptr;
        ForeignClassItem *foreignArea = nullptr;

        std::string recordOwnerName = GetOwnerName(name);
        if (!HandleRecordOnwer(items, program, &area, &foreignArea, name, recordOwnerName)) {
            return false;
        }

        auto params = std::vector<MethodParamItem> {};

        ASSERT(func.IsStatic() == isStatic);
        if (func.params.empty() || func.params[0].type.GetName() != GetOwnerName(func.name) ||
            func.metadata->IsCctor()) {
            ASSERT(func.IsStatic());
        }

        size_t idx = isStatic ? 0 : 1;

        if (!HandleFunctionParams(items, program, idx, name, func, primitiveTypes, params)) {
            return false;
        }

        if (emitDebugInfo && !HandleFunctionLocalVariables(items, func, name)) {
            return false;
        }

        auto *typeItem = GetTypeItem(items, primitiveTypes, func.returnType, program);
        if (typeItem == nullptr) {
            SetLastError("Function " + name + " has undefined return type");
            return false;
        }

        if (!CreateMethodItem(items, entities, func, typeItem, area, foreignArea, methodName, mangled_name, name,
                              params)) {
            return false;
        }
    }
    return true;
}

/* static */
bool AsmEmitter::MakeFunctionItems(
    ItemContainer *items, Program &program, AsmEmitter::AsmEntityCollections &entities,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes, bool emitDebugInfo)
{
    std::vector<std::string> funcsShouldBeStatic;
    for (auto &[name, func] : program.functionInstanceTable) {
        if (func.params.empty() || func.params[0].type.GetName() != GetOwnerName(name) || func.metadata->IsCctor()) {
            func.metadata->SetAccessFlags(func.metadata->GetAccessFlags() | ACC_STATIC);
            funcsShouldBeStatic.push_back(name);
        }
    }
    for (auto &funcName : funcsShouldBeStatic) {
        auto node = program.functionInstanceTable.extract(funcName);
        program.functionStaticTable.insert(std::move(node));
    }

    if (!MakeFunctionItems(items, program, entities, primitiveTypes, emitDebugInfo, true)) {
        return false;
    }

    if (!MakeFunctionItems(items, program, entities, primitiveTypes, emitDebugInfo, false)) {
        return false;
    }

    AddFakeIndexDependenciesForUnusedItems(entities);

    return true;
}

/// NOTE: (mivanov) Method below will be removed after #30937 implemented
/*static*/
void AsmEmitter::AddFakeIndexDependenciesForUnusedItems(AsmEmitter::AsmEntityCollections &entities)
{
    static std::vector<std::pair<std::string, std::vector<std::string>>> UNUSED_ITEM_KEYS_TO_EMIT = {
        {
            "std.core.String.%%get-length:i32;",
            {"std.core.StringBuilder.%%get-stringLength:i32;"},
        },
    };

    for (auto &[key, values] : UNUSED_ITEM_KEYS_TO_EMIT) {
        auto dependant = entities.methodItems.find(key);
        if (dependant == entities.methodItems.end()) {
            continue;
        }

        for (auto &value : values) {
            auto dependency = entities.methodItems.find(value);
            if (dependency == entities.methodItems.end()) {
                continue;
            }
            dependant->second->AddIndexDependency(dependency->second);
        }
    }
}

/* static */
bool AsmEmitter::MakeRecordAnnotations(ItemContainer *items, const Program &program,
                                       const AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &[name, record] : program.recordTable) {
        if (record.metadata->IsForeign()) {
            continue;
        }

        auto *classItem = static_cast<ClassItem *>(Find(entities.classItems, name));
        if (!AddAnnotations(classItem, items, *record.metadata, program, entities)) {
            SetLastError("Cannot emit annotations for record " + record.name + ": " + GetLastError());
            return false;
        }

        for (const auto &field : record.fieldList) {
            std::string fieldName = record.name + '.' + field.name;
            auto *fieldItem = !field.IsStatic() ? static_cast<FieldItem *>(Find(entities.fieldItems, fieldName))
                                                : static_cast<FieldItem *>(Find(entities.staticFieldItems, fieldName));
            if (!AddAnnotations(fieldItem, items, *field.metadata, program, entities)) {
                SetLastError("Cannot emit annotations for field " + fieldName + ": " + GetLastError());
                return false;
            }

            auto res = field.metadata->GetValue();
            if (res) {
                auto value = res.value();
                auto *item = CreateValueItem(items, &value, program, entities, {true, field.IsStatic()});
                fieldItem->SetValue(item);
            }
        }
    }
    return true;
}

/* static */
void AsmEmitter::SetCodeAndDebugInfo(ItemContainer *items, MethodItem *method, const Function &func, bool emitDebugInfo)
{
    auto *code = items->CreateItem<CodeItem>();
    method->SetCode(code);
    code->AddMethod(method);  // we need it for Profile-Guided optimization

    if (!emitDebugInfo && !func.CanThrow()) {
        return;
    }

    auto *lineNumberProgram = items->CreateLineNumberProgramItem();
    auto *debugInfo = items->CreateItem<DebugInfoItem>(lineNumberProgram);
    if (emitDebugInfo) {
        for (const auto &v : func.localVariableDebug) {
            ASSERT(v.reg >= 0);
            if (func.IsParameter(v.reg)) {
                debugInfo->AddParameter(items->GetOrCreateStringItem(v.name));
            }
        }
    } else {
        auto nparams = method->GetParams().size();
        for (size_t i = 0; i < nparams; i++) {
            debugInfo->AddParameter(nullptr);
        }
    }
    method->SetDebugInfo(debugInfo);
}

/* static */
void AsmEmitter::SetMethodSourceLang(const Program &program, MethodItem *method, const Function &func,
                                     const std::string &name)
{
    std::string recordName = GetOwnerName(name);
    if (recordName.empty()) {
        method->SetSourceLang(func.language);
        return;
    }

    auto &rec = program.recordTable.find(recordName)->second;
    if (rec.language != func.language) {
        method->SetSourceLang(func.language);
    }
}

/* static */
bool AsmEmitter::AddMethodAndParamsAnnotations(ItemContainer *items, const Program &program,
                                               const AsmEmitter::AsmEntityCollections &entities, MethodItem *method,
                                               const Function &func)
{
    if (!AddAnnotations(method, items, *func.metadata, program, entities)) {
        SetLastError("Cannot emit annotations for function " + func.name + ": " + GetLastError());
        return false;
    }

    auto &paramItems = method->GetParams();
    for (size_t protoIdx = 0; protoIdx < paramItems.size(); protoIdx++) {
        size_t paramIdx = method->IsStatic() ? protoIdx : protoIdx + 1;
        auto &param = func.params[paramIdx];
        auto &paramItem = paramItems[protoIdx];
        if (!AddAnnotations(&paramItem, items, *param.GetOrCreateMetadata(), program, entities)) {
            SetLastError("Cannot emit annotations for parameter a" + std::to_string(paramIdx) + " of function " +
                         func.name + ": " + GetLastError());
            return false;
        }
    }

    if (method->HasRuntimeParamAnnotations()) {
        items->CreateItem<ParamAnnotationsItem>(method, true);
    }

    if (method->HasParamAnnotations()) {
        items->CreateItem<ParamAnnotationsItem>(method, false);
    }

    return true;
}

/* static */
ark::panda_file::MethodItem *AsmEmitter::FindMethod(const Function &func, const std::string &name,
                                                    const AsmEmitter::AsmEntityCollections &entities)
{
    if (func.IsStatic()) {
        return static_cast<MethodItem *>(Find(entities.staticMethodItems, name));
    }
    return static_cast<MethodItem *>(Find(entities.methodItems, name));
}

/* static */
ark::panda_file::MethodItem *AsmEmitter::FindAmongAllMethods(const std::string &name,
                                                             const AsmEmitter::AsmEntityCollections &entities,
                                                             const Value *value)
{
    bool isStatic = value->GetAsScalar()->IsStatic();
    auto &primaryMap = isStatic ? entities.staticMethodItems : entities.methodItems;
    auto &secondaryMap = isStatic ? entities.methodItems : entities.staticMethodItems;

    auto res = primaryMap.find(name);
    if (res != primaryMap.cend()) {
        return static_cast<MethodItem *>(res->second);
    }

    res = secondaryMap.find(name);
    return res == secondaryMap.cend() ? nullptr : static_cast<MethodItem *>(res->second);
}

/* static */
bool AsmEmitter::MakeFunctionDebugInfoAndAnnotations(ItemContainer *items, const Program &program,
                                                     const std::map<std::string, Function> &functionTable,
                                                     const AsmEmitter::AsmEntityCollections &entities,
                                                     bool emitDebugInfo)
{
    for (const auto &[name, func] : functionTable) {
        if (func.metadata->IsForeign()) {
            continue;
        }

        auto *method = FindMethod(func, name, entities);
        if (method == nullptr) {
            return false;
        }

        if (func.metadata->HasImplementation()) {
            SetCodeAndDebugInfo(items, method, func, emitDebugInfo);
            AddBytecodeIndexDependencies(method, func, entities);
        }

        SetMethodSourceLang(program, method, func, name);

        if (!AddMethodAndParamsAnnotations(items, program, entities, method, func)) {
            return false;
        }
        if (func.profileSize <= MethodItem::MAX_PROFILE_SIZE) {
            method->SetProfileSize(func.profileSize);
        }
    }
    return true;
}

/* static */
bool AsmEmitter::MakeFunctionDebugInfoAndAnnotations(ItemContainer *items, const Program &program,
                                                     const AsmEmitter::AsmEntityCollections &entities,
                                                     bool emitDebugInfo)
{
    if (!MakeFunctionDebugInfoAndAnnotations(items, program, program.functionStaticTable, entities, emitDebugInfo)) {
        return false;
    }
    if (!MakeFunctionDebugInfoAndAnnotations(items, program, program.functionInstanceTable, entities, emitDebugInfo)) {
        return false;
    }

    return true;
}

/* static */
void AsmEmitter::FillMap(PandaFileToPandaAsmMaps *maps, AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &[name, method] : entities.methodItems) {
        maps->methods.insert({method->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, method] : entities.staticMethodItems) {
        maps->methods.insert({method->GetFileId().GetOffset(), std::string(name)});
        maps->staticMethods.insert(method->GetFileId().GetOffset());
    }

    for (const auto &[name, field] : entities.fieldItems) {
        maps->fields.insert({field->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, field] : entities.staticFieldItems) {
        maps->fields.insert({field->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, cls] : entities.classItems) {
        maps->classes.insert({cls->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, str] : entities.stringItems) {
        maps->strings.insert({str->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, arr] : entities.literalarrayItems) {
        maps->literalarrays.emplace(arr->GetFileId().GetOffset(), name);
    }
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
void AsmEmitter::EmitDebugInfo(ItemContainer *items, const Program &program, const std::vector<uint8_t> *bytes,
                               const MethodItem *method, const Function &func, const std::string &name,
                               bool emitDebugInfo)
{
    auto *debugInfo = method->GetDebugInfo();
    if (debugInfo == nullptr) {
        return;
    }

    auto *lineNumberProgram = debugInfo->GetLineNumberProgram();
    auto *constantPool = debugInfo->GetConstantPool();

    std::string recordName = GetOwnerName(name);
    std::string recordSourceFile;
    if (!recordName.empty()) {
        auto &rec = program.recordTable.find(recordName)->second;
        recordSourceFile = rec.sourceFile;
    }
    if (!func.sourceFile.empty() && func.sourceFile != recordSourceFile) {
        auto *sourceFileItem = items->GetOrCreateStringItem(func.sourceFile);
        ASSERT(sourceFileItem->GetOffset() != 0);
        lineNumberProgram->EmitSetFile(constantPool, sourceFileItem);
    }

    if (!func.sourceCode.empty()) {
        auto *sourceCodeItem = items->GetOrCreateStringItem(func.sourceCode);
        ASSERT(sourceCodeItem->GetOffset() != 0);
        lineNumberProgram->EmitSetSourceCode(constantPool, sourceCodeItem);
    }

    func.BuildLineNumberProgram(debugInfo, *bytes, items, constantPool, emitDebugInfo);
}

/* static */
bool AsmEmitter::EmitFunctions(ItemContainer *items, const Program &program,
                               const AsmEmitter::AsmEntityCollections &entities, bool emitDebugInfo, bool isStatic)
{
    const auto &functionTable = isStatic ? program.functionStaticTable : program.functionInstanceTable;
    for (const auto &f : functionTable) {
        const auto &[name, func] = f;

        if (func.metadata->IsForeign() || !func.metadata->HasImplementation()) {
            continue;
        }

        auto emitter = BytecodeEmitter {};
        auto *method = isStatic ? static_cast<MethodItem *>(Find(entities.staticMethodItems, name))
                                : static_cast<MethodItem *>(Find(entities.methodItems, name));
        if (!func.Emit(emitter, method, entities.methodItems, entities.staticMethodItems, entities.fieldItems,
                       entities.staticFieldItems, entities.classItems, entities.stringItems,
                       entities.literalarrayItems)) {
            SetLastError("Internal error during emitting function: " + func.name);
            return false;
        }

        auto *code = method->GetCode();
        code->SetNumVregs(func.regsNum);
        code->SetNumArgs(func.GetParamsNum());

        auto numIns = static_cast<uint32_t>(std::count_if(func.ins.cbegin(), func.ins.cend(),
                                                          [](auto const &it) { return it.opcode != Opcode::INVALID; }));
        code->SetNumInstructions(numIns);

        auto *bytes = code->GetInstructions();
        auto status = emitter.Build(static_cast<std::vector<unsigned char> *>(bytes));
        if (status != BytecodeEmitter::ErrorCode::SUCCESS) {
            SetLastError("Internal error during emitting binary code, status=" +
                         std::to_string(static_cast<int>(status)));
            return false;
        }
        auto tryBlocks = func.BuildTryBlocks(method, entities.classItems, *bytes);
        for (auto &tryBlock : tryBlocks) {
            code->AddTryBlock(tryBlock);
        }

        EmitDebugInfo(items, program, bytes, method, func, name, emitDebugInfo);
    }
    return true;
}

/* static */
bool AsmEmitter::EmitFunctions(ItemContainer *items, const Program &program,
                               const AsmEmitter::AsmEntityCollections &entities, bool emitDebugInfo)
{
    if (!EmitFunctions(items, program, entities, emitDebugInfo, true)) {
        return false;
    }
    if (!EmitFunctions(items, program, entities, emitDebugInfo, false)) {
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::Emit(ItemContainer *items, Program &program, PandaFileToPandaAsmMaps *maps, bool emitDebugInfo,
                      ark::panda_file::pgo::ProfileOptimizer *profileOpt)
{
    auto primitiveTypes = CreatePrimitiveTypes(items);

    auto entities = AsmEmitter::AsmEntityCollections {};

    SetLastError("");

    MakeStringItems(items, program, entities);

    MakeArrayTypeItems(items, program, entities);

    if (!MakeRecordItems(items, program, entities, primitiveTypes)) {
        return false;
    }

    if (!MakeFunctionItems(items, program, entities, primitiveTypes, emitDebugInfo)) {
        return false;
    }

    MakeLiteralItems(items, program, entities);

    // Add annotations for records and fields
    if (!MakeRecordAnnotations(items, program, entities)) {
        return false;
    }

    // Add Code and DebugInfo items last due to they have variable size that depends on bytecode
    if (!MakeFunctionDebugInfoAndAnnotations(items, program, entities, emitDebugInfo)) {
        return false;
    }

    if (profileOpt != nullptr) {
        items->ReorderItems(profileOpt);
    }

    items->ComputeLayout();

    if (maps != nullptr) {
        FillMap(maps, entities);
    }

    return EmitFunctions(items, program, entities, emitDebugInfo);
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::Emit(Writer *writer, Program &program, std::map<std::string, size_t> *stat,
                      PandaFileToPandaAsmMaps *maps, bool debugInfo, ark::panda_file::pgo::ProfileOptimizer *profileOpt)
{
    auto items = ItemContainer {};
    if (!Emit(&items, program, maps, debugInfo, profileOpt)) {
        return false;
    }

    if (stat != nullptr) {
        *stat = items.GetStat();
    }

    return items.Write(writer);
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::Emit(const std::string &filename, Program &program, std::map<std::string, size_t> *stat,
                      PandaFileToPandaAsmMaps *maps, bool debugInfo, ark::panda_file::pgo::ProfileOptimizer *profileOpt)
{
    auto writer = FileWriter(filename);
    if (!writer) {
        SetLastError("Unable to open " + filename + " for writing");
        return false;
    }
    return Emit(&writer, program, stat, maps, debugInfo, profileOpt);
}

std::unique_ptr<const panda_file::File> AsmEmitter::Emit(Program &program, PandaFileToPandaAsmMaps *maps)
{
    auto items = ItemContainer {};
    if (!Emit(&items, program, maps)) {
        return nullptr;
    }

    size_t size = items.ComputeLayout();
    auto *buffer = new std::byte[size];

    auto writer = MemoryBufferWriter(reinterpret_cast<uint8_t *>(buffer), size);
    if (!items.Write(&writer)) {
        delete[] buffer;
        return nullptr;
    }

    os::mem::ConstBytePtr ptr(
        buffer, size, [](std::byte *bufferPtr, [[maybe_unused]] size_t paramSize) noexcept { delete[] bufferPtr; });
    return panda_file::File::OpenFromMemory(std::move(ptr));
}

/* static */
bool AsmEmitter::AssignProfileInfo(std::unordered_map<size_t, std::vector<Ins *>> &instMap,
                                   std::map<std::string, pandasm::Function> &functionTable)
{
    constexpr auto SIZES = profiling::GetOrderedProfileSizes();
    for (auto &func : functionTable) {
        for (auto &inst : func.second.ins) {
            auto profSize = INST_PROFILE_SIZES[static_cast<unsigned>(inst.opcode)];
            if (profSize != 0) {
                instMap[profSize].push_back(&inst);
            }
        }
        size_t index = 0;
        for (auto it = SIZES.rbegin(); it != SIZES.rend(); ++it) {
            std::vector<Ins *> &vec = instMap[*it];
            for (auto inst : vec) {
                inst->profileId = index;
                index += *it;
            }
            vec.clear();
        }

        functionTable.at(func.first).profileSize = index;
    }
    return true;
}

bool AsmEmitter::AssignProfileInfo(Program *program)
{
    std::unordered_map<size_t, std::vector<Ins *>> instMap;
    /**
     * Since elements in the profile vector should be grouped by its size and ordered in
     * descending order, we first save instructions in map, where key is a profile size.
     * Then we iterate over this map in descending order - from biggest profile element size
     * to smallest. And assign profile index to all instructions with same size.
     */

    AssignProfileInfo(instMap, program->functionStaticTable);
    AssignProfileInfo(instMap, program->functionInstanceTable);

    return true;
}

TypeItem *AsmEmitter::GetTypeItem(
    ItemContainer *items, const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes,
    const Type &type, const Program &program)
{
    if (!type.IsObject()) {
        return Find(primitiveTypes, type.GetId());
    }

    if (type.IsArray() || type.IsUnion()) {
        auto item = items->GetOrCreateForeignClassItem(type.GetDescriptor());
        ASSERT(item != nullptr);
        return item;
    }

    const auto &name = type.GetName();
    auto iter = program.recordTable.find(name);
    if (iter == program.recordTable.end()) {
        return nullptr;
    }

    auto &rec = iter->second;

    if (rec.metadata->IsForeign()) {
        auto item = items->GetOrCreateForeignClassItem(type.GetDescriptor());
        ASSERT(item != nullptr);
        return item;
    }

    return items->GetOrCreateClassItem(type.GetDescriptor());
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool Function::Emit(BytecodeEmitter &emitter, panda_file::MethodItem *method,
                    const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods,
                    const std::unordered_map<std::string, panda_file::BaseMethodItem *> &staticMethods,
                    const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields,
                    const std::unordered_map<std::string, panda_file::BaseFieldItem *> &staticFields,
                    const std::unordered_map<std::string, panda_file::BaseClassItem *> &classes,
                    const std::unordered_map<std::string_view, panda_file::StringItem *> &strings,
                    const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays) const
{
    auto labels = std::unordered_map<std::string_view, ark::Label> {};

    for (const auto &insn : ins) {
        if (insn.HasLabel()) {
            labels.insert_or_assign(insn.Label(), emitter.CreateLabel());
        }
    }

    for (const auto &insn : ins) {
        if (insn.HasLabel()) {
            auto search = labels.find(insn.Label());
            ASSERT(search != labels.end());
            emitter.Bind(search->second);
        }

        if (insn.opcode != Opcode::INVALID) {
            if (!insn.Emit(emitter, method, methods, staticMethods, fields, staticFields, classes, strings,
                           literalarrays, labels)) {
                return false;
            }
        }
    }

    return true;
}

static void TryEmitPc(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool, uint32_t &pcInc)
{
    if (pcInc != 0U) {
        program->EmitAdvancePc(constantPool, pcInc);
        pcInc = 0;
    }
}

void Function::EmitLocalVariable(panda_file::LineNumberProgramItem *program, ItemContainer *container,
                                 std::vector<uint8_t> *constantPool, uint32_t &pcInc, size_t instructionNumber,
                                 size_t variableIndex) const
{
    ASSERT(variableIndex < localVariableDebug.size());
    const auto &v = localVariableDebug[variableIndex];
    ASSERT(!IsParameter(v.reg));
    if (instructionNumber == v.start) {
        TryEmitPc(program, constantPool, pcInc);
        StringItem *variableName = container->GetOrCreateStringItem(v.name);
        StringItem *variableType = container->GetOrCreateStringItem(v.signature);
        if (v.signatureType.empty()) {
            program->EmitStartLocal(constantPool, v.reg, variableName, variableType);
        } else {
            StringItem *typeSignature = container->GetOrCreateStringItem(v.signatureType);
            program->EmitStartLocalExtended(constantPool, v.reg, variableName, variableType, typeSignature);
        }
    }

    if (instructionNumber == (v.start + v.length)) {
        TryEmitPc(program, constantPool, pcInc);
        program->EmitEndLocal(v.reg);
    }
}

size_t Function::GetLineNumber(size_t i) const
{
    return static_cast<size_t>(ins[i].insDebug.LineNumber());
}

uint32_t Function::GetColumnNumber(size_t i) const
{
    return ins[i].insDebug.ColumnNumber();
}

void Function::EmitNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                          uint32_t pcInc, int32_t lineInc) const
{
    if (!program->EmitSpecialOpcode(pcInc, lineInc)) {
        if (pcInc != 0) {
            program->EmitAdvancePc(constantPool, pcInc);
            if (!program->EmitSpecialOpcode(0, lineInc)) {
                program->EmitAdvanceLine(constantPool, lineInc);
                program->EmitSpecialOpcode(0, 0);
            }
        } else {
            program->EmitAdvanceLine(constantPool, lineInc);
            program->EmitSpecialOpcode(0, 0);
        }
    }
}

void Function::EmitLineNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                              uint32_t &prevLineNumber, uint32_t &pcInc, size_t instructionNumber) const
{
    auto lineInc = GetLineNumber(instructionNumber) - prevLineNumber;
    if (lineInc != 0) {
        prevLineNumber = GetLineNumber(instructionNumber);
        EmitNumber(program, constantPool, pcInc, lineInc);
        pcInc = 0;
    }
}

void Function::EmitColumnNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                                uint32_t &prevColumnNumber, uint32_t &pcInc, size_t instructionNumber) const
{
    uint32_t cn = GetColumnNumber(instructionNumber);
    if (cn != prevColumnNumber) {
        program->EmitColumn(constantPool, pcInc, cn);
        pcInc = 0;
        prevColumnNumber = cn;
    }
}

void Function::CollectLocalVariable(std::vector<Function::LocalVariablePair> &localVariableInfo) const
{
    for (size_t i = 0; i < localVariableDebug.size(); i++) {
        const auto &v = localVariableDebug[i];
        if (IsParameter(v.reg)) {
            continue;
        }
        localVariableInfo.emplace_back(v.start, i);
        localVariableInfo.emplace_back(v.start + v.length, i);
    }

    // clang-format off
    std::sort(localVariableInfo.begin(), localVariableInfo.end(),
        [](const Function::LocalVariablePair &a, const Function::LocalVariablePair &b) {
            return a.insnOrder < b.insnOrder;
        });
    // clang-format on
}

void Function::BuildLineNumberProgram(panda_file::DebugInfoItem *debugItem, const std::vector<uint8_t> &bytecode,
                                      ItemContainer *container, std::vector<uint8_t> *constantPool,
                                      bool emitDebugInfo) const
{
    auto *program = debugItem->GetLineNumberProgram();

    if (ins.empty()) {
        program->EmitEnd();
        return;
    }

    uint32_t pcInc = 0;
    uint32_t prevLineNumber = GetLineNumber(0);
    uint32_t prevColumnNumber = std::numeric_limits<uint32_t>::max();
    BytecodeInstruction bi(bytecode.data());
    debugItem->SetLineNumber(prevLineNumber);

    std::vector<Function::LocalVariablePair> localVariableInfo;
    if (emitDebugInfo) {
        CollectLocalVariable(localVariableInfo);
    }
    const size_t numIns = ins.size();
    size_t start = 0;
    auto iter = localVariableInfo.begin();
    do {
        size_t end = emitDebugInfo && iter != localVariableInfo.end() ? iter->insnOrder : numIns;
        for (size_t i = start; i < end && i < numIns; i++) {
            /**
             * If you change the continue condition of this loop, you need to synchronously modify the same condition
             * of the BuildMapFromPcToIns method in the optimizer-bytecode.cpp.
             */
            if (ins[i].opcode == Opcode::INVALID) {
                continue;
            }
            if (emitDebugInfo || ins[i].CanThrow()) {
                EmitLineNumber(program, constantPool, prevLineNumber, pcInc, i);
            }
            if (ark::panda_file::IsDynamicLanguage(language) && emitDebugInfo) {
                EmitColumnNumber(program, constantPool, prevColumnNumber, pcInc, i);
            }
            pcInc += bi.GetSize();
            bi = bi.GetNext();
        }
        if (iter == localVariableInfo.end() || end >= numIns) {
            break;
        }
        if (emitDebugInfo) {
            EmitLocalVariable(program, container, constantPool, pcInc, end, iter->variableIndex);
        }
        start = end;
        iter++;
    } while (true);

    program->EmitEnd();
}

Function::TryCatchInfo Function::MakeOrderAndOffsets(const std::vector<uint8_t> &bytecode) const
{
    std::unordered_map<std::string_view, size_t> tryCatchLabels {};
    std::unordered_map<std::string, std::vector<const CatchBlock *>> tryCatchMap {};
    std::vector<std::string> tryCatchOrder {};

    for (auto &catchBlock : catchBlocks) {
        tryCatchLabels.insert_or_assign(catchBlock.tryBeginLabel, 0);
        tryCatchLabels.insert_or_assign(catchBlock.tryEndLabel, 0);
        tryCatchLabels.insert_or_assign(catchBlock.catchBeginLabel, 0);
        tryCatchLabels.insert_or_assign(catchBlock.catchEndLabel, 0);

        std::string tryKey = catchBlock.tryBeginLabel + ":" + catchBlock.tryEndLabel;
        auto it = tryCatchMap.find(tryKey);
        if (it == tryCatchMap.cend()) {
            std::tie(it, std::ignore) = tryCatchMap.try_emplace(tryKey);
            tryCatchOrder.push_back(tryKey);
        }
        it->second.push_back(&catchBlock);
    }

    BytecodeInstruction bi(bytecode.data());
    size_t pcOffset = 0;

    for (const auto &i : ins) {
        if (i.HasLabel()) {
            auto it = tryCatchLabels.find(i.Label());
            if (it != tryCatchLabels.cend()) {
                tryCatchLabels[i.Label()] = pcOffset;
            }
        }
        if (i.opcode == Opcode::INVALID) {
            continue;
        }

        pcOffset += bi.GetSize();
        bi = bi.GetNext();
    }

    return Function::TryCatchInfo {std::move(tryCatchLabels), std::move(tryCatchMap), std::move(tryCatchOrder)};
}

std::vector<CodeItem::TryBlock> Function::BuildTryBlocks(
    MethodItem *method, const std::unordered_map<std::string, BaseClassItem *> &classItems,
    const std::vector<uint8_t> &bytecode) const
{
    std::vector<CodeItem::TryBlock> tryBlocks;

    if (ins.empty()) {
        return tryBlocks;
    }

    Function::TryCatchInfo tcs = MakeOrderAndOffsets(bytecode);

    for (const auto &tKey : tcs.tryCatchOrder) {
        auto kv = tcs.tryCatchMap.find(tKey);
        ASSERT(kv != tcs.tryCatchMap.cend());
        auto &tryCatchBlocks = kv->second;

        ASSERT(!tryCatchBlocks.empty());

        std::vector<CodeItem::CatchBlock> catchBlockItems {};

        for (auto *catchBlock : tryCatchBlocks) {
            auto className = catchBlock->exceptionRecord;

            BaseClassItem *classItem = nullptr;
            if (!className.empty()) {
                auto it = classItems.find(className);
                ASSERT(it != classItems.cend());
                classItem = it->second;
            }

            auto handlerPcOffset = tcs.tryCatchLabels[catchBlock->catchBeginLabel];
            auto handlerCodeSize = tcs.tryCatchLabels[catchBlock->catchEndLabel] - handlerPcOffset;
            catchBlockItems.emplace_back(method, classItem, handlerPcOffset, handlerCodeSize);
        }

        auto tryStartPcOffset = tcs.tryCatchLabels[tryCatchBlocks[0]->tryBeginLabel];
        auto tryEndPcOffset = tcs.tryCatchLabels[tryCatchBlocks[0]->tryEndLabel];
        ASSERT(tryEndPcOffset >= tryStartPcOffset);
        tryBlocks.emplace_back(tryStartPcOffset, tryEndPcOffset - tryStartPcOffset, catchBlockItems);
    }

    return tryBlocks;
}

void Function::DebugDump() const
{
    std::cerr << "name: " << name << std::endl;
    for (const auto &i : ins) {
        std::cerr << i.ToString("\n", true, regsNum);
    }
}

std::string GetOwnerName(std::string name)
{
    name = DeMangleName(name);
    auto superPos = name.find_last_of(PARSE_AREA_MARKER);
    if (superPos == std::string::npos) {
        return "";
    }

    return name.substr(0, superPos);
}

std::string GetItemName(std::string name)
{
    name = DeMangleName(name);
    auto superPos = name.find_last_of(PARSE_AREA_MARKER);
    if (superPos == std::string::npos) {
        return name;
    }

    return name.substr(superPos + 1);
}

}  // namespace ark::pandasm
