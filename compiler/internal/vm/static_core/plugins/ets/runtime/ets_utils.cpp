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

#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets {

bool IsEtsGlobalClassName(const std::string &descriptor)
{
    ASSERT(descriptor.length() >= 2U);  // L...;
    ASSERT('L' == descriptor[0]);
    ASSERT(';' == descriptor[descriptor.size() - 1]);

    constexpr size_t ETSGLOBAL_SEMICOLON_LENGTH = sizeof(ETSGLOBAL_CLASS_NAME);

    const auto etsGlobalSubstringPos = descriptor.rfind(ETSGLOBAL_CLASS_NAME);
    if (etsGlobalSubstringPos == std::string::npos) {
        return false;
    }
    const bool etsGlobalClass = (1 == etsGlobalSubstringPos) && descriptor.length() == 1 + ETSGLOBAL_SEMICOLON_LENGTH;
    const bool endsWithETSGLOBAL = descriptor.length() - ETSGLOBAL_SEMICOLON_LENGTH == etsGlobalSubstringPos;
    const bool etsGlobalClassInPackage = endsWithETSGLOBAL && '/' == descriptor[etsGlobalSubstringPos - 1];

    return etsGlobalClass || etsGlobalClassInPackage;
}

EtsObject *GetBoxedValue(EtsCoroutine *coro, Value value, EtsType type)
{
    switch (type) {
        case EtsType::BOOLEAN:
            return EtsBoxPrimitive<EtsBoolean>::Create(coro, value.GetAs<EtsBoolean>());
        case EtsType::BYTE:
            return EtsBoxPrimitive<EtsByte>::Create(coro, value.GetAs<EtsByte>());
        case EtsType::CHAR:
            return EtsBoxPrimitive<EtsChar>::Create(coro, value.GetAs<EtsChar>());
        case EtsType::SHORT:
            return EtsBoxPrimitive<EtsShort>::Create(coro, value.GetAs<EtsShort>());
        case EtsType::INT:
            return EtsBoxPrimitive<EtsInt>::Create(coro, value.GetAs<EtsInt>());
        case EtsType::LONG:
            return EtsBoxPrimitive<EtsLong>::Create(coro, value.GetAs<EtsLong>());
        case EtsType::FLOAT:
            return EtsBoxPrimitive<EtsFloat>::Create(coro, value.GetAs<EtsFloat>());
        case EtsType::DOUBLE:
            return EtsBoxPrimitive<EtsDouble>::Create(coro, value.GetAs<EtsDouble>());
        default:
            UNREACHABLE();
    }
}

Value GetUnboxedValue(EtsCoroutine *coro, EtsObject *obj)
{
    auto *cls = obj->GetClass();
    auto ptypes = PlatformTypes(coro);
    if (cls == ptypes->coreBoolean) {
        return Value((reinterpret_cast<EtsBoxPrimitive<EtsBoolean> *>(obj))->GetValue());
    }
    if (cls == ptypes->coreDouble) {
        return Value((reinterpret_cast<EtsBoxPrimitive<EtsDouble> *>(obj))->GetValue());
    }
    if (cls == ptypes->coreInt) {
        return Value((reinterpret_cast<EtsBoxPrimitive<EtsInt> *>(obj))->GetValue());
    }
    if (cls == ptypes->coreByte) {
        return Value((reinterpret_cast<EtsBoxPrimitive<EtsByte> *>(obj))->GetValue());
    }
    if (cls == ptypes->coreShort) {
        return Value((reinterpret_cast<EtsBoxPrimitive<EtsShort> *>(obj))->GetValue());
    }
    if (cls == ptypes->coreLong) {
        return Value((reinterpret_cast<EtsBoxPrimitive<EtsLong> *>(obj))->GetValue());
    }
    if (cls == ptypes->coreFloat) {
        return Value((reinterpret_cast<EtsBoxPrimitive<EtsFloat> *>(obj))->GetValue());
    }
    if (cls == ptypes->coreChar) {
        return Value((reinterpret_cast<EtsBoxPrimitive<EtsChar> *>(obj))->GetValue());
    }
    return Value(reinterpret_cast<ObjectHeader *>(obj));
}

void LambdaUtils::InvokeVoid(EtsCoroutine *coro, EtsObject *lambda)
{
    ASSERT(lambda != nullptr);
    EtsMethod *invoke = lambda->GetClass()->GetInstanceMethod(INVOKE_METHOD_NAME, nullptr);
    if (invoke == nullptr) {
        LOG(FATAL, RUNTIME) << "No method '$_invoke' found";
        return;
    }
    Value arg(lambda->GetCoreType());
    invoke->GetPandaMethod()->InvokeVoid(coro, &arg);
}

EtsString *ManglingUtils::GetDisplayNameStringFromField(EtsField *field)
{
    auto fieldNameData = field->GetCoreType()->GetName();
    auto fieldNameLength = fieldNameData.utf16Length;
    std::string_view fieldName(utf::Mutf8AsCString(fieldNameData.data), fieldNameLength);
    if (fieldName.rfind(PROPERTY, 0) == 0) {
        ASSERT(fieldNameLength >= PROPERTY_PREFIX_LENGTH);
        return EtsString::Resolve(
            fieldNameData.data + PROPERTY_PREFIX_LENGTH,  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            fieldNameLength - PROPERTY_PREFIX_LENGTH);
    }
    return EtsString::Resolve(fieldNameData.data, fieldNameData.utf16Length);
}

EtsField *ManglingUtils::GetFieldIDByDisplayName(EtsClass *klass, const PandaString &name, const char *sig)
{
    auto u8name = utf::CStringAsMutf8(name.c_str());
    auto *field = EtsField::FromRuntimeField(klass->GetRuntimeClass()->GetInstanceFieldByName(u8name));
    if (field == nullptr && !klass->GetRuntimeClass()->GetInterfaces().empty()) {
        auto mangledName = PandaString(PROPERTY) + name;
        u8name = utf::CStringAsMutf8(mangledName.c_str());
        field = EtsField::FromRuntimeField(klass->GetRuntimeClass()->GetInstanceFieldByName(u8name));
    }

    if (sig != nullptr && field != nullptr) {
        auto fieldTypeDescriptor = reinterpret_cast<const uint8_t *>(field->GetTypeDescriptor());
        if (utf::CompareMUtf8ToMUtf8(fieldTypeDescriptor, reinterpret_cast<const uint8_t *>(sig)) != 0) {
            return nullptr;
        }
    }

    return field;
}

EtsObject *GetPropertyValue(EtsCoroutine *coro, const EtsObject *etsObj, EtsField *field)
{
    EtsType type = field->GetEtsType();
    switch (type) {
        case EtsType::BOOLEAN: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsBoolean>(field);
            return reinterpret_cast<EtsObject *>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::BYTE: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsByte>(field);
            return reinterpret_cast<EtsObject *>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::CHAR: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsChar>(field);
            return reinterpret_cast<EtsObject *>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::SHORT: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsShort>(field);
            return reinterpret_cast<EtsObject *>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::INT: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsInt>(field);
            return reinterpret_cast<EtsObject *>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::LONG: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsLong>(field);
            return reinterpret_cast<EtsObject *>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::FLOAT: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsFloat>(field);
            return reinterpret_cast<EtsObject *>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::DOUBLE: {
            auto etsVal = etsObj->GetFieldPrimitive<EtsDouble>(field);
            return reinterpret_cast<EtsObject *>(GetBoxedValue(coro, ark::Value(etsVal), field->GetEtsType()));
        }
        case EtsType::OBJECT: {
            return reinterpret_cast<EtsObject *>(etsObj->GetFieldObject(field));
        }
        default:
            return nullptr;
    }
}

bool SetPropertyValue(EtsCoroutine *coro, EtsObject *etsObj, EtsField *field, EtsObject *valToSet)
{
    ark::Value etsVal = GetUnboxedValue(coro, valToSet);
    EtsType type = field->GetEtsType();
    switch (type) {
        case EtsType::BOOLEAN:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsBoolean>());
            break;
        case EtsType::BYTE:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsByte>());
            break;
        case EtsType::CHAR:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsChar>());
            break;
        case EtsType::SHORT:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsShort>());
            break;
        case EtsType::INT:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsInt>());
            break;
        case EtsType::LONG:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsLong>());
            break;
        case EtsType::FLOAT:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsFloat>());
            break;
        case EtsType::DOUBLE:
            etsObj->SetFieldPrimitive(field, etsVal.GetAs<EtsDouble>());
            break;
        case EtsType::OBJECT:
            etsObj->SetFieldObject(field, reinterpret_cast<EtsObject *>(valToSet));
            break;
        default:
            return false;
    }

    return true;
}
static void ExtractClassDescriptorsFromArray(const panda_file::File *pfile, const panda_file::ArrayValue &classesArray,
                                             std::vector<std::string> &outDescriptors)
{
    const uint32_t valCount = classesArray.GetCount();
    outDescriptors.resize(valCount);
    for (uint32_t j = 0; j < valCount; j++) {
        auto entityId = classesArray.Get<panda_file::File::EntityId>(j);
        panda_file::ClassDataAccessor classData(*pfile, entityId);
        outDescriptors[j] = utf::Mutf8AsCString(classData.GetDescriptor());
    }
}

// NOLINT(clang-analyzer-core) // false positive: this is a function, not a global var
bool GetExportedClassDescriptorsFromModule(ark::ets::EtsClass *etsGlobalClass, std::vector<std::string> &outDescriptors)
{
    ASSERT(etsGlobalClass != nullptr);

    const auto *runtimeClass = etsGlobalClass->GetRuntimeClass();
    const panda_file::File *pfile = runtimeClass->GetPandaFile();
    panda_file::ClassDataAccessor cda(*pfile, runtimeClass->GetFileId());

    bool found = false;
    cda.EnumerateAnnotation(panda_file_items::class_descriptors::ANNOTATION_MODULE.data(),
                            [&outDescriptors, &found, pfile](panda_file::AnnotationDataAccessor &annotationAccessor) {
                                const uint32_t count = annotationAccessor.GetCount();
                                for (uint32_t i = 0; i < count; i++) {
                                    auto elem = annotationAccessor.GetElement(i);
                                    std::string nameStr =
                                        utf::Mutf8AsCString(pfile->GetStringData(elem.GetNameId()).data);
                                    if (nameStr == panda_file_items::class_descriptors::ANNOTATION_MODULE_EXPORTED) {
                                        auto classesArray = elem.GetArrayValue();
                                        ExtractClassDescriptorsFromArray(pfile, classesArray, outDescriptors);
                                        found = true;
                                        break;
                                    }
                                }
                                return true;
                            });

    return found;
}

const std::map<PandaString, char> ClassPublicNameParser::primitiveNameMapping = {
    {"u16", 'C'}, {"i8", 'B'}, {"i16", 'S'}, {"i32", 'I'}, {"i64", 'J'}, {"f32", 'F'}, {"f64", 'D'}, {"u1", 'Z'}};

const std::map<char, PandaString> RuntimeDescriptorParser::primitiveNameMapping = {
    {'C', "u16"}, {'B', "i8"},  {'S', "i16"}, {'I', "i32"}, {'J', "i64"},
    {'F', "f32"}, {'D', "f64"}, {'Z', "u1"},  {'V', "void"}};

}  // namespace ark::ets
