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

#include <string>

#include "ani.h"
#include "ets_runtime_interface.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "types/ets_method.h"

using namespace ark::ets::panda_file_items;

namespace ark::ets {
compiler::RuntimeInterface::ClassPtr EtsRuntimeInterface::GetClass(MethodPtr method, IdType id) const
{
    if (id == RuntimeInterface::MEM_PROMISE_CLASS_ID) {
        return PlatformTypes()->corePromise->GetRuntimeClass();
    }
    return PandaRuntimeInterface::GetClass(method, id);
}

compiler::RuntimeInterface::FieldPtr EtsRuntimeInterface::ResolveLookUpField(FieldPtr rawField, ClassPtr klass)
{
    ASSERT(rawField != nullptr);
    ASSERT(klass != nullptr);
    Class *current = ClassCast(klass);
    Field *raw = FieldCast(rawField);
    while (current != nullptr) {
        Field *field = LookupFieldByName(raw->GetName(), current);
        if (LIKELY(field != nullptr)) {
            return field;
        }
        current = current->GetBase();
    }
    return nullptr;
}

template <panda_file::Type::TypeId FIELD_TYPE>
compiler::RuntimeInterface::MethodPtr EtsRuntimeInterface::GetLookUpCall(FieldPtr rawField, ClassPtr klass,
                                                                         bool isSetter)
{
    Field *raw = FieldCast(rawField);
    Method *method = nullptr;
    Class *current = ClassCast(klass);
    while (current != nullptr) {
        if (isSetter) {
            method = LookupSetterByName<FIELD_TYPE>(raw->GetName(), current);
        } else {
            method = LookupGetterByName<FIELD_TYPE>(raw->GetName(), current);
        }
        if (LIKELY(method != nullptr)) {
            return method;
        }
        current = current->GetBase();
    }
    return method;
}

compiler::RuntimeInterface::MethodPtr EtsRuntimeInterface::ResolveLookUpCall(FieldPtr rawField, ClassPtr klass,
                                                                             bool isSetter)
{
    ASSERT(rawField != nullptr);
    ASSERT(klass != nullptr);
    switch (FieldCast(rawField)->GetTypeId()) {
        case panda_file::Type::TypeId::U1:
            return GetLookUpCall<panda_file::Type::TypeId::U1>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::U8:
            return GetLookUpCall<panda_file::Type::TypeId::U8>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::I8:
            return GetLookUpCall<panda_file::Type::TypeId::I8>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::I16:
            return GetLookUpCall<panda_file::Type::TypeId::I16>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::U16:
            return GetLookUpCall<panda_file::Type::TypeId::U16>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::I32:
            return GetLookUpCall<panda_file::Type::TypeId::I32>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::U32:
            return GetLookUpCall<panda_file::Type::TypeId::U32>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::I64:
            return GetLookUpCall<panda_file::Type::TypeId::I64>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::U64:
            return GetLookUpCall<panda_file::Type::TypeId::U64>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::F32:
            return GetLookUpCall<panda_file::Type::TypeId::F32>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::F64:
            return GetLookUpCall<panda_file::Type::TypeId::F64>(rawField, klass, isSetter);
        case panda_file::Type::TypeId::REFERENCE:
            return GetLookUpCall<panda_file::Type::TypeId::REFERENCE>(rawField, klass, isSetter);
        default: {
            UNREACHABLE();
            break;
        }
    }
    return nullptr;
}

uint64_t EtsRuntimeInterface::GetUniqueObject() const
{
    return ToUintPtr(PandaEtsVM::GetCurrent()->GetNullValue());
}

compiler::RuntimeInterface::InteropCallKind EtsRuntimeInterface::GetInteropCallKind(MethodPtr methodPtr) const
{
    auto className = GetClassNameFromMethod(methodPtr);
    auto classNameSuffix = className.substr(className.find_last_of('.') + 1);
    if (classNameSuffix == "$jsnew") {
        return InteropCallKind::NEW_INSTANCE;
    }
    if (classNameSuffix != "$jscall") {
        return InteropCallKind::UNKNOWN;
    }

    auto method = MethodCast(methodPtr);

    ASSERT(method->GetArgType(0).IsReference());  // arg0 is always a reference
    if (method->GetArgType(1).IsReference()) {
        auto pf = method->GetPandaFile();
        panda_file::ProtoDataAccessor pda(*pf, panda_file::MethodDataAccessor::GetProtoId(*pf, method->GetFileId()));
        ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto linkerCtx = method->GetClass()->GetLoadContext();
        uint32_t const argReftypeShift = method->GetReturnType().IsReference() ? 1 : 0;
        ScopedMutatorLock lock;
        auto cls = classLinker->GetClass(*pf, pda.GetReferenceType(1 + argReftypeShift), linkerCtx);
        ASSERT(cls != nullptr);
        if (!cls->IsStringClass()) {
            return InteropCallKind::CALL_BY_VALUE;
        }
    } else {
        // arg1 and arg2 are start position and length of qualified name
        ASSERT(method->GetArgType(1).GetId() == panda_file::Type::TypeId::I32);
        ASSERT(method->GetArgType(2U).GetId() == panda_file::Type::TypeId::I32);
    }
    return InteropCallKind::CALL;
}

char *EtsRuntimeInterface::GetFuncPropName(MethodPtr methodPtr, uint32_t strId) const
{
    auto method = MethodCast(methodPtr);
    auto pf = method->GetPandaFile();
    auto str = reinterpret_cast<const char *>(pf->GetStringData(ark::panda_file::File::EntityId(strId)).data);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return const_cast<char *>(std::strrchr(str, '.') + 1);
}

uint64_t EtsRuntimeInterface::GetFuncPropNameOffset(MethodPtr methodPtr, uint32_t strId) const
{
    auto pf = MethodCast(methodPtr)->GetPandaFile();
    auto str = GetFuncPropName(methodPtr, strId);
    return reinterpret_cast<uint64_t>(str) - reinterpret_cast<uint64_t>(pf->GetBase());
}

bool EtsRuntimeInterface::IsMethodStringConcat(MethodPtr method) const
{
    return method == PlatformTypes()->coreStringConcat;
}

bool EtsRuntimeInterface::IsMethodStringGetLength(MethodPtr method) const
{
    return method == PlatformTypes()->coreStringGetLength || method == PlatformTypes()->coreStringLength;
}

Field *EtsRuntimeInterface::GetFieldPtrByName(ClassPtr klass, std::string_view name) const
{
    if (klass == nullptr) {
        return nullptr;
    }
    auto etsClass = EtsClass::FromRuntimeClass(ClassCast(klass));
    auto fieldIndex = etsClass->GetFieldIndexByName(name.data());
    if (fieldIndex == -1U) {
        return nullptr;
    }
    return etsClass->GetFieldByIndex(fieldIndex)->GetRuntimeField();
}

bool EtsRuntimeInterface::IsMethodStringBuilderConstructorWithStringArg(MethodPtr method) const
{
    return method == PlatformTypes()->coreStringBuilderConstructorWithStringArg;
}

bool EtsRuntimeInterface::IsMethodStringBuilderConstructorWithCharArrayArg(MethodPtr method) const
{
    return method == PlatformTypes()->coreStringBuilderConstructorWithCharArrayArg;
}

bool EtsRuntimeInterface::IsMethodStringBuilderDefaultConstructor(MethodPtr method) const
{
    return method == PlatformTypes()->coreStringBuilderDefaultConstructor;
}

bool EtsRuntimeInterface::IsMethodStringBuilderToString(MethodPtr method) const
{
    return method == PlatformTypes()->coreStringBuilderToString;
}

bool EtsRuntimeInterface::IsMethodStringBuilderAppend(MethodPtr method) const
{
    return std::strcmp(GetMethodFullName(method, false).c_str(),
                       PlatformTypes()->coreStringBuilderAppendString->GetFullName().c_str()) == 0;
}

bool EtsRuntimeInterface::IsMethodStringBuilderGetStringLength([[maybe_unused]] MethodPtr method) const
{
    return method == PlatformTypes()->coreStringBuilderStringLength;
}

bool EtsRuntimeInterface::IsMethodInModuleScope(MethodPtr method) const
{
    return static_cast<EtsMethod *>(method)->GetClass()->IsModule();
}

bool EtsRuntimeInterface::IsClassStringBuilder(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) == PlatformTypes(PandaEtsVM::GetCurrent())->coreStringBuilder;
}

bool EtsRuntimeInterface::IsClassEscompatArray(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) == PlatformTypes(PandaEtsVM::GetCurrent())->escompatArray;
}

bool EtsRuntimeInterface::IsClassEscompatInt8Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) == PlatformTypes(PandaEtsVM::GetCurrent())->escompatInt8Array;
}

bool EtsRuntimeInterface::IsClassEscompatUint8Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) == PlatformTypes(PandaEtsVM::GetCurrent())->escompatUint8Array;
}

bool EtsRuntimeInterface::IsClassEscompatUint8ClampedArray(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) ==
           PlatformTypes(PandaEtsVM::GetCurrent())->escompatUint8ClampedArray;
}

bool EtsRuntimeInterface::IsClassEscompatInt16Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) == PlatformTypes(PandaEtsVM::GetCurrent())->escompatInt16Array;
}

bool EtsRuntimeInterface::IsClassEscompatUint16Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) == PlatformTypes(PandaEtsVM::GetCurrent())->escompatUint16Array;
}

bool EtsRuntimeInterface::IsClassEscompatInt32Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) == PlatformTypes(PandaEtsVM::GetCurrent())->escompatInt32Array;
}

bool EtsRuntimeInterface::IsClassEscompatUint32Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) == PlatformTypes(PandaEtsVM::GetCurrent())->escompatUint32Array;
}

bool EtsRuntimeInterface::IsClassEscompatFloat32Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) ==
           PlatformTypes(PandaEtsVM::GetCurrent())->escompatFloat32Array;
}

bool EtsRuntimeInterface::IsClassEscompatFloat64Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) ==
           PlatformTypes(PandaEtsVM::GetCurrent())->escompatFloat64Array;
}

bool EtsRuntimeInterface::IsClassEscompatBigInt64Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) ==
           PlatformTypes(PandaEtsVM::GetCurrent())->escompatBigInt64Array;
}

bool EtsRuntimeInterface::IsClassEscompatBigUint64Array(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass)) ==
           PlatformTypes(PandaEtsVM::GetCurrent())->escompatBigUint64Array;
}

bool EtsRuntimeInterface::IsClassEscompatTypedArray(ClassPtr klass) const
{
    return IsClassEscompatInt8Array(klass) || IsClassEscompatUint8Array(klass) ||
           IsClassEscompatUint8ClampedArray(klass) || IsClassEscompatInt16Array(klass) ||
           IsClassEscompatUint16Array(klass) || IsClassEscompatInt32Array(klass) ||
           IsClassEscompatUint32Array((klass)) || IsClassEscompatFloat32Array(klass) ||
           IsClassEscompatFloat64Array(klass) || IsClassEscompatBigInt64Array(klass) ||
           IsClassEscompatBigUint64Array(klass);
}

bool EtsRuntimeInterface::IsMethodTypedArrayCtor(MethodPtr method) const
{
    return MethodCast(method)->IsConstructor() && IsClassEscompatTypedArray(MethodCast(method)->GetClass());
}

bool EtsRuntimeInterface::IsFieldTypedArrayLengthInt(FieldPtr field) const
{
    return IsClassEscompatTypedArray(FieldCast(field)->GetClass()) && GetFieldName(field) == fields::LENGTH_INT;
}

uint32_t EtsRuntimeInterface::GetClassOffsetObjectsArray(MethodPtr method) const
{
    auto pf = MethodCast(method)->GetPandaFile();
    return pf->GetClassId(PlatformTypes()->coreObjectArray->GetRuntimeClass()->GetDescriptor()).GetOffset();
}

EtsRuntimeInterface::ClassPtr EtsRuntimeInterface::GetStringBuilderClass() const
{
    return PlatformTypes()->coreStringBuilder->GetRuntimeClass();
}

EtsRuntimeInterface::ClassPtr EtsRuntimeInterface::GetEscompatArrayClass() const
{
    return PlatformTypes(PandaEtsVM::GetCurrent())->escompatArray->GetRuntimeClass();
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetEscompatTypedArrayBuffer(ClassPtr klass) const
{
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::BUFFER.data()));
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetEscompatTypedArrayByteOffset(ClassPtr klass) const
{
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::BYTE_OFFSET.data()));
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetEscompatUnsignedTypedArrayByteOffsetInt(ClassPtr klass) const
{
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::BYTE_OFFSET_INT.data()));
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetEscompatTypedArrayLengthInt(ClassPtr klass) const
{
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::LENGTH_INT.data()));
}

EtsRuntimeInterface::ClassPtr EtsRuntimeInterface::GetEscompatArrayBufferClass() const
{
    return PlatformTypes(PandaEtsVM::GetCurrent())->coreArrayBuffer->GetRuntimeClass();
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetEscompatArrayBufferDataAddress(ClassPtr klass) const
{
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::DATA_ADDRESS.data()));
}

// NOTE(@volkovanton, #31053) Will be refactored as a follow-up.
EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetEscompatArrayBufferManagedData(ClassPtr klass) const
{
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::DATA.data()));
}

EtsRuntimeInterface::MethodPtr EtsRuntimeInterface::GetStringBuilderDefaultConstructor() const
{
    for (auto ctor : PlatformTypes()->coreStringBuilder->GetConstructors()) {
        if (IsMethodStringBuilderDefaultConstructor(ctor)) {
            return ctor;
        }
    }

    UNREACHABLE();
}

uint32_t EtsRuntimeInterface::GetMethodId(MethodPtr method) const
{
    ASSERT(method != nullptr);
    return static_cast<EtsMethod *>(method)->GetMethodId();
}

EtsRuntimeInterface::MethodPtr EtsRuntimeInterface::GetInstanceMethodByName(ClassPtr klass, std::string_view name) const
{
    if (klass != nullptr) {
        auto etsClass = EtsClass::FromRuntimeClass(ClassCast(klass));
        return etsClass->GetInstanceMethod(name.data(), nullptr);
    }
    return nullptr;
}

bool EtsRuntimeInterface::IsFieldBooleanFalse([[maybe_unused]] FieldPtr field) const
{
    return IsClassBoxedBoolean((FieldCast(field)->GetClass())) && GetFieldName(field) == fields::BOOLEAN_FALSE;
}

bool EtsRuntimeInterface::IsFieldBooleanTrue([[maybe_unused]] FieldPtr field) const
{
    return IsClassBoxedBoolean((FieldCast(field)->GetClass())) && GetFieldName(field) == fields::BOOLEAN_TRUE;
}

bool EtsRuntimeInterface::IsFieldBooleanValue([[maybe_unused]] FieldPtr field) const
{
    return IsClassBoxedBoolean((FieldCast(field)->GetClass())) && GetFieldName(field) == fields::VALUE;
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetFieldStringBuilderBuffer(ClassPtr klass) const
{
    ASSERT(IsClassStringBuilder(klass));
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::BUF.data()));
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetFieldStringBuilderIndex(ClassPtr klass) const
{
    ASSERT(IsClassStringBuilder(klass));
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::INDEX.data()));
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetFieldStringBuilderLength(ClassPtr klass) const
{
    ASSERT(IsClassStringBuilder(klass));
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::LENGTH.data()));
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetFieldStringBuilderCompress(ClassPtr klass) const
{
    ASSERT(IsClassStringBuilder(klass));
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::COMPRESS.data()));
}

EtsRuntimeInterface::MethodPtr EtsRuntimeInterface::GetGetterStringBuilderStringLength() const
{
    return PlatformTypes()->coreStringBuilderStringLength;
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetEscompatArrayActualLength(ClassPtr klass) const
{
    ASSERT(IsClassEscompatArray(klass));
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::ACTUAL_LENGTH.data()));
}

EtsRuntimeInterface::FieldPtr EtsRuntimeInterface::GetEscompatArrayBuffer(ClassPtr klass) const
{
    ASSERT(IsClassEscompatArray(klass));
    return ClassCast(klass)->GetInstanceFieldByName(utf::CStringAsMutf8(fields::BUFFER.data()));
}

bool EtsRuntimeInterface::IsFieldStringBuilderBuffer(FieldPtr field) const
{
    return IsClassStringBuilder(FieldCast(field)->GetClass()) && GetFieldName(field) == fields::BUF;
}

bool EtsRuntimeInterface::IsFieldStringBuilderIndex(FieldPtr field) const
{
    return IsClassStringBuilder(FieldCast(field)->GetClass()) && GetFieldName(field) == fields::INDEX;
}

bool EtsRuntimeInterface::IsIntrinsicStringBuilderToString(IntrinsicId id) const
{
    return id == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_TO_STRING;
}

bool EtsRuntimeInterface::IsIntrinsicStringBuilderAppendString(IntrinsicId id) const
{
    switch (id) {
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4:
            return true;
        default:
            return false;
    }
}

bool EtsRuntimeInterface::IsIntrinsicStringBuilderAppend(IntrinsicId id) const
{
    switch (id) {
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_FLOAT:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_DOUBLE:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_LONG:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_INT:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_BYTE:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_SHORT:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_CHAR:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_BOOL:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4:
            return true;
        default:
            return false;
    }
}

bool EtsRuntimeInterface::IsIntrinsicStringConcat(IntrinsicId id) const
{
    switch (id) {
        case IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT2:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT3:
            return true;
        case IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT4:
            return true;
        default:
            return false;
    }
}

EtsRuntimeInterface::IntrinsicId EtsRuntimeInterface::ConvertTypeToStringBuilderAppendIntrinsicId(
    compiler::DataType::Type type) const
{
    switch (type) {
        case compiler::DataType::BOOL:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_BOOL;
        case compiler::DataType::INT8:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_CHAR;
        case compiler::DataType::UINT8:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_BYTE;
        case compiler::DataType::INT16:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_SHORT;
        case compiler::DataType::INT32:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_INT;
        case compiler::DataType::INT64:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_LONG;
        case compiler::DataType::FLOAT64:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_DOUBLE;
        case compiler::DataType::FLOAT32:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_FLOAT;
        case compiler::DataType::REFERENCE:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING;
        default:
            UNREACHABLE();
    }
    return IntrinsicId::INVALID;
}

EtsRuntimeInterface::IntrinsicId EtsRuntimeInterface::GetStringConcatStringsIntrinsicId(size_t numArgs) const
{
    // NOLINTBEGIN(readability-magic-numbers)
    switch (numArgs) {
        case 2U:
            return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT2;
        case 3U:
            return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT3;
        case 4U:
            return IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT4;
        default:
            UNREACHABLE();
    }
    // NOLINTEND(readability-magic-numbers)
}

EtsRuntimeInterface::IntrinsicId EtsRuntimeInterface::GetStringIsCompressedIntrinsicId() const
{
    return IntrinsicId::INTRINSIC_STD_CORE_STRING_IS_COMPRESSED;
}

EtsRuntimeInterface::IntrinsicId EtsRuntimeInterface::GetStringBuilderAppendStringsIntrinsicId(size_t numArgs) const
{
    // NOLINTBEGIN(readability-magic-numbers)
    switch (numArgs) {
        case 1U:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING;
        case 2U:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2;
        case 3U:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3;
        case 4U:
            return IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4;
        default:
            UNREACHABLE();
    }
    // NOLINTEND(readability-magic-numbers)
}

EtsRuntimeInterface::IntrinsicId EtsRuntimeInterface::GetStringBuilderToStringIntrinsicId() const
{
    return IntrinsicId::INTRINSIC_STD_CORE_SB_TO_STRING;
}

bool EtsRuntimeInterface::IsClassValueTyped(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass))->IsValueTyped();
}

void *EtsRuntimeInterface::GetDoubleToStringCache() const
{
    return ark::ets::PandaEtsVM::GetCurrent()->GetDoubleToStringCache();
}

void *EtsRuntimeInterface::GetAsciiCharCache() const
{
    return PlatformTypes(PandaEtsVM::GetCurrent())->GetAsciiCacheTable();
}

bool EtsRuntimeInterface::IsStringCachesUsed() const
{
    return Runtime::GetOptions().IsUseStringCaches();
}

bool EtsRuntimeInterface::IsUseAllStrings() const
{
    return Runtime::GetCurrent()->GetOptions().IsUseAllStrings();
}

bool EtsRuntimeInterface::IsNativeMethodOptimizationEnabled() const
{
    return true;
}

uint32_t EtsRuntimeInterface::GetRuntimeClassOffset(Arch arch) const
{
    return ark::cross_values::GetEtsClassRuntimeClassOffset(arch);
}

bool EtsRuntimeInterface::IsBoxedClass(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass))->IsBoxed();
}

bool EtsRuntimeInterface::IsEnumClass(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass))->IsEtsEnum();
}

bool EtsRuntimeInterface::IsBigIntClass(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass))->IsBigInt();
}

bool EtsRuntimeInterface::IsFunctionReference(ClassPtr klass) const
{
    return EtsClass::FromRuntimeClass(ClassCast(klass))->IsFunctionReference();
}

bool EtsRuntimeInterface::IsClassBoxedBoolean(ClassPtr klass) const
{
    return PlatformTypes(PandaEtsVM::GetCurrent())->coreBoolean->GetRuntimeClass() == klass;
}

bool EtsRuntimeInterface::IsClassBoxedFloat(ClassPtr klass) const
{
    return PlatformTypes(PandaEtsVM::GetCurrent())->coreFloat->GetRuntimeClass() == klass;
}

bool EtsRuntimeInterface::IsClassBoxedDouble(ClassPtr klass) const
{
    return PlatformTypes(PandaEtsVM::GetCurrent())->coreDouble->GetRuntimeClass() == klass;
}

ark::compiler::DataType::Type EtsRuntimeInterface::GetBoxedClassDataType(ClassPtr klass) const
{
    auto platformTypes = PlatformTypes(PandaEtsVM::GetCurrent());
    if (platformTypes->coreBoolean->GetRuntimeClass() == klass) {
        return compiler::DataType::BOOL;
    }
    if (platformTypes->coreByte->GetRuntimeClass() == klass) {
        return compiler::DataType::INT8;
    }
    if (platformTypes->coreChar->GetRuntimeClass() == klass) {
        return compiler::DataType::UINT16;
    }
    if (platformTypes->coreShort->GetRuntimeClass() == klass) {
        return compiler::DataType::INT16;
    }
    if (platformTypes->coreInt->GetRuntimeClass() == klass) {
        return compiler::DataType::INT32;
    }
    if (platformTypes->coreLong->GetRuntimeClass() == klass) {
        return compiler::DataType::INT64;
    }
    if (platformTypes->coreFloat->GetRuntimeClass() == klass) {
        return compiler::DataType::FLOAT32;
    }
    if (platformTypes->coreDouble->GetRuntimeClass() == klass) {
        return compiler::DataType::FLOAT64;
    }
    return compiler::DataType::NO_TYPE;
}

size_t EtsRuntimeInterface::GetTlsNativeApiOffset(Arch arch) const
{
    return ark::cross_values::GetEtsCoroutineAniEnvOffset(arch);
}

}  // namespace ark::ets
