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
#ifndef PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H
#define PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H

#include "runtime/compiler.h"
#include "cross_values.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_platform_types.h"
namespace ark::ets {

class EtsRuntimeInterface : public PandaRuntimeInterface {
public:
    /// Object information
    ClassPtr GetClass(MethodPtr method, IdType id) const override;
    size_t GetTlsPromiseClassPointerOffset(Arch arch) const override
    {
        return ark::cross_values::GetEtsCoroutinePromiseClassOffset(arch);
    }
    size_t GetTlsNativeApiOffset(Arch arch) const override;
    size_t GetTlsUniqueObjectOffset(Arch arch) const override
    {
        return ark::cross_values::GetEtsCoroutineNullValueOffset(arch);
    }
    uint64_t GetUniqueObject() const override;
    InteropCallKind GetInteropCallKind(MethodPtr methodPtr) const override;
    char *GetFuncPropName(MethodPtr methodPtr, uint32_t strId) const override;
    uint64_t GetFuncPropNameOffset(MethodPtr methodPtr, uint32_t strId) const override;
    bool IsMethodStringConcat(MethodPtr method) const override;
    bool IsMethodStringGetLength(MethodPtr method) const override;
    Field *GetFieldPtrByName(ClassPtr klass, std::string_view name) const override;
    bool IsMethodStringBuilderConstructorWithStringArg(MethodPtr method) const override;
    bool IsMethodStringBuilderConstructorWithCharArrayArg(MethodPtr method) const override;
    bool IsMethodStringBuilderDefaultConstructor(MethodPtr method) const override;
    bool IsMethodStringBuilderToString(MethodPtr method) const override;
    bool IsMethodStringBuilderAppend(MethodPtr method) const override;
    bool IsMethodStringBuilderGetStringLength(MethodPtr method) const override;
    bool IsMethodInModuleScope([[maybe_unused]] MethodPtr method) const override;
    bool IsMethodTypedArrayCtor([[maybe_unused]] MethodPtr method) const override;
    bool IsClassStringBuilder(ClassPtr klass) const override;
    bool IsClassEscompatArray(ClassPtr klass) const override;
    bool IsClassEscompatInt8Array(ClassPtr klass) const override;
    bool IsClassEscompatUint8Array(ClassPtr klass) const override;
    bool IsClassEscompatUint8ClampedArray(ClassPtr klass) const override;
    bool IsClassEscompatInt16Array(ClassPtr klass) const override;
    bool IsClassEscompatUint16Array(ClassPtr klass) const override;
    bool IsClassEscompatInt32Array(ClassPtr klass) const override;
    bool IsClassEscompatUint32Array(ClassPtr klass) const override;
    bool IsClassEscompatFloat32Array(ClassPtr klass) const override;
    bool IsClassEscompatFloat64Array(ClassPtr klass) const override;
    bool IsClassEscompatBigInt64Array(ClassPtr klass) const override;
    bool IsClassEscompatBigUint64Array(ClassPtr klass) const override;
    bool IsClassEscompatTypedArray(ClassPtr klass) const override;
    bool IsFieldTypedArrayLengthInt(FieldPtr field) const override;
    uint32_t GetClassOffsetObjectsArray(MethodPtr method) const override;
    ClassPtr GetStringBuilderClass() const override;
    ClassPtr GetEscompatArrayClass() const override;
    MethodPtr GetStringBuilderDefaultConstructor() const override;
    uint32_t GetMethodId([[maybe_unused]] MethodPtr method) const override;
    MethodPtr GetInstanceMethodByName(ClassPtr klass, std::string_view name) const override;
    bool IsFieldBooleanFalse(FieldPtr field) const override;
    bool IsFieldBooleanTrue(FieldPtr field) const override;
    bool IsFieldBooleanValue(FieldPtr field) const override;
    bool IsFieldStringBuilderBuffer(FieldPtr field) const override;
    bool IsFieldStringBuilderIndex(FieldPtr field) const override;
    FieldPtr GetFieldStringBuilderBuffer(ClassPtr klass) const override;
    FieldPtr GetFieldStringBuilderIndex(ClassPtr klass) const override;
    FieldPtr GetFieldStringBuilderLength(ClassPtr klass) const override;
    FieldPtr GetFieldStringBuilderCompress(ClassPtr klass) const override;
    MethodPtr GetGetterStringBuilderStringLength() const override;
    FieldPtr GetEscompatArrayBuffer(ClassPtr klass) const override;
    FieldPtr GetEscompatArrayActualLength(ClassPtr klass) const override;
    FieldPtr GetEscompatTypedArrayBuffer(ClassPtr klass) const override;
    FieldPtr GetEscompatTypedArrayByteOffset(ClassPtr klass) const override;
    FieldPtr GetEscompatUnsignedTypedArrayByteOffsetInt(ClassPtr klass) const override;
    FieldPtr GetEscompatTypedArrayLengthInt(ClassPtr klass) const override;
    ClassPtr GetEscompatArrayBufferClass() const override;
    FieldPtr GetEscompatArrayBufferDataAddress(ClassPtr klass) const override;
    FieldPtr GetEscompatArrayBufferManagedData(ClassPtr klass) const override;
    bool IsIntrinsicStringBuilderToString(IntrinsicId id) const override;
    bool IsIntrinsicStringBuilderAppendString(IntrinsicId id) const override;
    bool IsIntrinsicStringBuilderAppend(IntrinsicId id) const override;
    bool IsIntrinsicStringConcat(IntrinsicId id) const override;
    IntrinsicId ConvertTypeToStringBuilderAppendIntrinsicId(compiler::DataType::Type type) const override;
    IntrinsicId GetStringConcatStringsIntrinsicId(size_t numArgs) const override;
    IntrinsicId GetStringIsCompressedIntrinsicId() const override;
    IntrinsicId GetStringBuilderAppendStringsIntrinsicId(size_t numArgs) const override;
    IntrinsicId GetStringBuilderToStringIntrinsicId() const override;
    bool IsClassValueTyped(ClassPtr klass) const override;
    void *GetDoubleToStringCache() const override;
    void *GetAsciiCharCache() const override;
    bool IsStringCachesUsed() const override;
    bool IsUseAllStrings() const override;
    bool IsNativeMethodOptimizationEnabled() const override;
    uint32_t GetRuntimeClassOffset(Arch arch) const override;
    bool IsBoxedClass(ClassPtr klass) const override;
    bool IsEnumClass(ClassPtr klass) const override;
    bool IsBigIntClass(ClassPtr klass) const override;
    bool IsFunctionReference(ClassPtr klass) const override;
    bool IsClassBoxedBoolean(ClassPtr klass) const override;
    bool IsClassBoxedFloat(ClassPtr klass) const override;
    bool IsClassBoxedDouble(ClassPtr klass) const override;
    compiler::DataType::Type GetBoxedClassDataType(ClassPtr klass) const override;

    FieldPtr ResolveLookUpField(FieldPtr rawField, ClassPtr klass) override;
    MethodPtr ResolveLookUpCall(FieldPtr rawField, ClassPtr klass, bool isSetter) override;

    template <panda_file::Type::TypeId FIELD_TYPE>
    compiler::RuntimeInterface::MethodPtr GetLookUpCall(FieldPtr rawField, ClassPtr klass, bool isSetter);

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/runtime/interop_js/ets_interop_runtime_interface-inl.h"
#endif
};
}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_RUNTIME_INTERFACE_H
