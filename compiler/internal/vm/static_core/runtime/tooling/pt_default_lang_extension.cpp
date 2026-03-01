/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "runtime/tooling/pt_default_lang_extension.h"

#include "include/class.h"
#include "include/coretypes/array-inl.h"
#include "include/coretypes/line_string.h"
#include "include/object_header.h"
#include "include/hclass.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/runtime.h"

namespace ark::tooling {

static TypedValue GetArrayElementValueStatic(const coretypes::Array &array, size_t offset, panda_file::Type type)
{
    switch (type.GetId()) {
        case panda_file::Type::TypeId::INVALID:
            return TypedValue::Invalid();
        case panda_file::Type::TypeId::VOID:
            return TypedValue::Void();
        case panda_file::Type::TypeId::U1:
            return TypedValue::U1(array.GetPrimitive<uint8_t>(offset) != 0);
        case panda_file::Type::TypeId::I8:
            return TypedValue::I8(array.GetPrimitive<int8_t>(offset));
        case panda_file::Type::TypeId::U8:
            return TypedValue::U8(array.GetPrimitive<uint8_t>(offset));
        case panda_file::Type::TypeId::I16:
            return TypedValue::I16(array.GetPrimitive<int16_t>(offset));
        case panda_file::Type::TypeId::U16:
            return TypedValue::U16(array.GetPrimitive<uint16_t>(offset));
        case panda_file::Type::TypeId::I32:
            return TypedValue::I32(array.GetPrimitive<int32_t>(offset));
        case panda_file::Type::TypeId::U32:
            return TypedValue::U32(array.GetPrimitive<uint32_t>(offset));
        case panda_file::Type::TypeId::F32:
            return TypedValue::F32(array.GetPrimitive<float>(offset));
        case panda_file::Type::TypeId::F64:
            return TypedValue::F64(array.GetPrimitive<double>(offset));
        case panda_file::Type::TypeId::I64:
            return TypedValue::I64(array.GetPrimitive<int64_t>(offset));
        case panda_file::Type::TypeId::U64:
            return TypedValue::U64(array.GetPrimitive<uint64_t>(offset));
        case panda_file::Type::TypeId::REFERENCE:
            return TypedValue::Reference(array.GetObject(offset));
        case panda_file::Type::TypeId::TAGGED:
        default:
            UNREACHABLE();
    }
    UNREACHABLE();
}

template <typename T>
static TypedValue GetFieldValueStatic(T object, const Field &field)
{
    switch (field.GetType().GetId()) {
        case panda_file::Type::TypeId::INVALID:
            return TypedValue::Invalid();
        case panda_file::Type::TypeId::VOID:
            return TypedValue::Void();
        case panda_file::Type::TypeId::U1:
            return TypedValue::U1(object->template GetFieldPrimitive<uint8_t>(field.GetOffset()) != 0);
        case panda_file::Type::TypeId::I8:
            return TypedValue::I8(object->template GetFieldPrimitive<int8_t>(field.GetOffset()));
        case panda_file::Type::TypeId::U8:
            return TypedValue::U8(object->template GetFieldPrimitive<uint8_t>(field.GetOffset()));
        case panda_file::Type::TypeId::I16:
            return TypedValue::I16(object->template GetFieldPrimitive<int16_t>(field.GetOffset()));
        case panda_file::Type::TypeId::U16:
            return TypedValue::U16(object->template GetFieldPrimitive<uint16_t>(field.GetOffset()));
        case panda_file::Type::TypeId::I32:
            return TypedValue::I32(object->template GetFieldPrimitive<int32_t>(field.GetOffset()));
        case panda_file::Type::TypeId::U32:
            return TypedValue::U32(object->template GetFieldPrimitive<uint32_t>(field.GetOffset()));
        case panda_file::Type::TypeId::F32:
            return TypedValue::F32(object->template GetFieldPrimitive<float>(field.GetOffset()));
        case panda_file::Type::TypeId::F64:
            return TypedValue::F64(object->template GetFieldPrimitive<double>(field.GetOffset()));
        case panda_file::Type::TypeId::I64:
            return TypedValue::I64(object->template GetFieldPrimitive<int64_t>(field.GetOffset()));
        case panda_file::Type::TypeId::U64:
            return TypedValue::U64(object->template GetFieldPrimitive<uint64_t>(field.GetOffset()));
        case panda_file::Type::TypeId::REFERENCE:
            return TypedValue::Reference(object->GetFieldObject(field.GetOffset()));
        case panda_file::Type::TypeId::TAGGED:
        default:
            UNREACHABLE();
    }
    UNREACHABLE();
}

std::string PtStaticDefaultExtension::GetClassName(const ObjectHeader *object)
{
    return ClassHelper::GetNameUndecorated(object->ClassAddr<Class>()->GetDescriptor());
}

std::optional<std::string> PtStaticDefaultExtension::GetAsString(const ObjectHeader *object)
{
    if (!object->ClassAddr<Class>()->IsStringClass()) {
        return {};
    }

    std::string value;

    auto string = coretypes::String::Cast(const_cast<ObjectHeader *>(object));
    for (auto index = 0U; index < string->GetLength(); ++index) {
        value.push_back(string->At(index));
    }
    return value;
}

std::optional<size_t> PtStaticDefaultExtension::GetLengthIfArray(const ObjectHeader *object)
{
    if (!object->ClassAddr<Class>()->IsArrayClass()) {
        return {};
    }
    return coretypes::Array::Cast(const_cast<ObjectHeader *>(object))->GetLength();
}

void PtStaticDefaultExtension::EnumerateProperties(const ObjectHeader *object, const PropertyHandler &handler)
{
    ASSERT(object != nullptr);
    auto cls = object->ClassAddr<Class>();
    ASSERT(cls != nullptr);
    if (cls->IsArrayClass()) {
        auto &array = *coretypes::Array::Cast(const_cast<ObjectHeader *>(object));
        auto type = cls->GetComponentType()->GetType();
        for (auto index = 0U; index < array.GetLength(); ++index) {
            auto offset = index * cls->GetComponentSize();
            handler(std::to_string(index), GetArrayElementValueStatic(array, offset, type), false, false);
        }
    } else if (cls->IsClassClass()) {
        auto runtimeCls = ark::Class::FromClassObject(object);
        ASSERT(runtimeCls != nullptr);
        for (auto &field : runtimeCls->GetStaticFields()) {
            handler(utf::Mutf8AsCString(field.GetName().data), GetFieldValueStatic(runtimeCls, field), field.IsFinal(),
                    false);
        }
    } else {
        do {
            for (const auto &field : cls->GetInstanceFields()) {
                handler(utf::Mutf8AsCString(field.GetName().data), GetFieldValueStatic(object, field), field.IsFinal(),
                        false);
            }
            cls = cls->GetBase();
        } while (cls != nullptr);
    }
}

void PtStaticDefaultExtension::EnumerateGlobals(const PropertyHandler &handler)
{
    auto classLinkerExtension = Runtime::GetCurrent()->GetClassLinker()->GetExtension(lang_);
    ASSERT(classLinkerExtension != nullptr);
    classLinkerExtension->EnumerateClasses([&handler](auto cls) {
        if (cls->IsInitialized() && cls->GetNumStaticFields() > 0) {
            handler(cls->GetName(), TypedValue::Reference(cls->GetManagedObject()), false, false);
        }
        return true;
    });
}

std::string PtDynamicDefaultExtension::GetClassName(const ObjectHeader *object)
{
    if (object->ClassAddr<HClass>()->IsString()) {
        return "String object";
    }
    return "Dynamic object";
}

std::optional<std::string> PtDynamicDefaultExtension::GetAsString([[maybe_unused]] const ObjectHeader *object)
{
    return {};
}

std::optional<size_t> PtDynamicDefaultExtension::GetLengthIfArray(const ObjectHeader *object)
{
    if (!object->ClassAddr<HClass>()->IsArray()) {
        return {};
    }
    auto length = coretypes::Array::Cast(const_cast<ObjectHeader *>(object))->GetLength();
    return length;
}

void PtDynamicDefaultExtension::EnumerateProperties(const ObjectHeader *object, const PropertyHandler &handler)
{
    ASSERT(object != nullptr);
    auto *cls = object->ClassAddr<HClass>();
    ASSERT(cls != nullptr);
    if (cls->IsNativePointer() || cls->IsString()) {
        return;
    }

    if (cls->IsArray()) {
        auto &array = *coretypes::Array::Cast(const_cast<ObjectHeader *>(object));
        ArraySizeT arrayLength = array.GetLength();
        for (coretypes::ArraySizeT i = 0; i < arrayLength; i++) {
            TaggedValue arrayElement(array.Get<TaggedType, false, true>(i));
            handler(std::to_string(i), TypedValue::Tagged(arrayElement), false, false);
        }
    } else if (cls->IsHClass()) {
        auto dataOffset = sizeof(ObjectHeader) + sizeof(HClass);
        auto objBodySize = cls->GetObjectSize() - dataOffset;
        ASSERT(objBodySize % TaggedValue::TaggedTypeSize() == 0);
        auto numOfFields = objBodySize / TaggedValue::TaggedTypeSize();
        for (size_t i = 0; i < numOfFields; i++) {
            auto fieldOffset = dataOffset + i * TaggedValue::TaggedTypeSize();
            auto taggedValue = ObjectAccessor::GetDynValue<TaggedValue>(object, fieldOffset);
            handler(std::to_string(i), TypedValue::Tagged(taggedValue), false, false);
        }
    } else {
        uint32_t objBodySize = cls->GetObjectSize() - ObjectHeader::ObjectHeaderSize();
        ASSERT(objBodySize % TaggedValue::TaggedTypeSize() == 0);
        uint32_t numOfFields = objBodySize / TaggedValue::TaggedTypeSize();
        size_t dataOffset = ObjectHeader::ObjectHeaderSize();
        for (uint32_t i = 0; i < numOfFields; i++) {
            size_t fieldOffset = dataOffset + i * TaggedValue::TaggedTypeSize();
            if (cls->IsNativeField(fieldOffset)) {
                continue;
            }
            auto taggedValue = ObjectAccessor::GetDynValue<TaggedValue>(object, fieldOffset);
            handler(std::to_string(i), TypedValue::Tagged(taggedValue), false, false);
        }
    }
}
}  // namespace ark::tooling
