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

#ifndef COMPILER_RUNTIME_INTERFACE_H
#define COMPILER_RUNTIME_INTERFACE_H

#include "assembler/assembly-literals.h"
#include "cross_values.h"
#include "datatype.h"
#include "ir-dyn-base-types.h"
#include "libarkbase/mem/gc_barrier.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/profiling/profiling.h"
#include "source_languages.h"
#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/span.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/mem/mem.h"

namespace ark {
class Thread;
}  // namespace ark

namespace ark::compiler {
enum class ClassType {
    UNRESOLVED_CLASS = 0,
    OBJECT_CLASS,
    ARRAY_CLASS,
    ARRAY_OBJECT_CLASS,
    INTERFACE_CLASS,
    OTHER_CLASS,
    FINAL_CLASS,
    UNION_CLASS,
    COUNT
};

enum class StringCtorType { UNKNOWN = 0, STRING, CHAR_ARRAY, COUNT };

class IClassHierarchyAnalysis;
class InlineCachesInterface;
class UnresolvedTypesInterface;

class RuntimeInterface {
public:
    using BinaryFilePtr = void *;
    using MethodPtr = void *;
    using FieldPtr = void *;
    using MethodId = uint32_t;
    using StringPtr = void *;
    using ClassPtr = void *;
    using ThreadPtr = void *;
    using IdType = uint32_t;
    using FieldId = uint32_t;
    using StringId = uint32_t;
    using LiteralArrayId = uint32_t;
    using MethodIndex = uint16_t;
    using FieldIndex = uint16_t;
    using TypeIndex = uint16_t;

    static const uintptr_t RESOLVE_STRING_AOT_COUNTER_LIMIT = PANDA_32BITS_HEAP_START_ADDRESS;
    static const uint32_t MEM_PROMISE_CLASS_ID = std::numeric_limits<uint32_t>::max() - 1;

    enum class NamedAccessProfileType {
        UNKNOWN = 0,
        FIELD,
        FIELD_INLINED,
        ACCESSOR,
        ELEMENT,
        ARRAY_ELEMENT,
        PROTOTYPE,
        PROTOTYPE_INLINED,
        TRANSITION,
        TRANSITION_INLINED,
        GLOBAL
    };

    struct NamedAccessProfileData {
        ClassPtr klass;
        uintptr_t cachedValue;
        uintptr_t key;
        uint32_t offset;
        NamedAccessProfileType type;
    };

    enum class InteropCallKind { UNKNOWN = 0, CALL, CALL_BY_VALUE, NEW_INSTANCE, COUNT };

    RuntimeInterface() = default;
    virtual ~RuntimeInterface() = default;

    virtual IClassHierarchyAnalysis *GetCha()
    {
        return nullptr;
    }

    virtual InlineCachesInterface *GetInlineCaches()
    {
        return nullptr;
    }

    virtual UnresolvedTypesInterface *GetUnresolvedTypes()
    {
        return nullptr;
    }

    virtual void *GetRuntimeEntry()
    {
        return nullptr;
    }

    virtual unsigned GetReturnReasonOk() const
    {
        return 0;
    }
    virtual unsigned GetReturnReasonDeopt() const
    {
        return 1;
    }

    virtual MethodId ResolveMethodIndex([[maybe_unused]] MethodPtr parentMethod,
                                        [[maybe_unused]] MethodIndex index) const
    {
        return 0;
    }

    virtual FieldId ResolveFieldIndex([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] FieldIndex index) const
    {
        return 0;
    }

    virtual IdType ResolveTypeIndex([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] TypeIndex index) const
    {
        return 0;
    }

    virtual size_t GetStackOverflowCheckOffset() const
    {
        return 0;
    }

    /// Binary file information
    virtual BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    virtual uint32_t GetAOTBinaryFileSnapshotIndexForMethod([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual BinaryFilePtr GetAOTBinaryFileBySnapshotIndex([[maybe_unused]] uint32_t index) const
    {
        return nullptr;
    }

    virtual uint32_t GetAOTBinaryFileSnapshotIndex([[maybe_unused]] BinaryFilePtr file) const
    {
        return 0;
    }

    // File offsets
    uint32_t GetBinaryFileBaseOffset(Arch arch) const
    {
        return cross_values::GetFileBaseOffset(arch);
    }

    /// Method information
    virtual MethodPtr GetMethodById([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] MethodId id) const
    {
        return nullptr;
    }

    virtual MethodPtr GetMethodByIdAndSaveJsFunction([[maybe_unused]] MethodPtr parentMethod,
                                                     [[maybe_unused]] MethodId id)
    {
        return nullptr;
    }

    virtual MethodPtr GetInstanceMethodByName([[maybe_unused]] ClassPtr klass,
                                              [[maybe_unused]] std::string_view name) const
    {
        return nullptr;
    }

    virtual MethodId GetMethodId([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual uint64_t GetUniqMethodId([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual MethodPtr GetMethodFromFunction([[maybe_unused]] uintptr_t function) const
    {
        return nullptr;
    }

    virtual MethodPtr ResolveVirtualMethod([[maybe_unused]] ClassPtr cls, [[maybe_unused]] MethodPtr id) const
    {
        return nullptr;
    }

    virtual MethodPtr ResolveInterfaceMethod([[maybe_unused]] ClassPtr cls, [[maybe_unused]] MethodPtr id) const
    {
        return nullptr;
    }

    virtual DataType::Type GetMethodReturnType([[maybe_unused]] MethodPtr method) const
    {
        return DataType::NO_TYPE;
    }

    virtual IdType GetMethodReturnTypeId([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual IdType GetMethodArgReferenceTypeId([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint16_t num) const
    {
        return 0;
    }

    virtual void CleanObjectHandles([[maybe_unused]] MethodPtr method) {}

    // Return this argument type for index == 0 in case of instance method
    virtual DataType::Type GetMethodTotalArgumentType([[maybe_unused]] MethodPtr method,
                                                      [[maybe_unused]] size_t index) const
    {
        return DataType::NO_TYPE;
    }
    // Return total arguments count including this for instance method
    virtual size_t GetMethodTotalArgumentsCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual DataType::Type GetMethodReturnType([[maybe_unused]] MethodPtr parentMethod,
                                               [[maybe_unused]] MethodId id) const
    {
        return DataType::NO_TYPE;
    }
    virtual DataType::Type GetMethodArgumentType([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] MethodId id,
                                                 [[maybe_unused]] size_t index) const
    {
        return DataType::NO_TYPE;
    }
    virtual size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] MethodId id) const
    {
        return 0;
    }
    virtual size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual size_t GetMethodRegistersCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual const uint8_t *GetMethodCode([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }
    virtual size_t GetMethodCodeSize([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual MethodPtr ResolveLookUpCall([[maybe_unused]] FieldPtr rawField, [[maybe_unused]] ClassPtr klass,
                                        [[maybe_unused]] bool isSetter)
    {
        return nullptr;
    }

    virtual SourceLanguage GetMethodSourceLanguage([[maybe_unused]] MethodPtr method) const
    {
        return SourceLanguage::PANDA_ASSEMBLY;
    }

    virtual void SetCompiledEntryPoint([[maybe_unused]] MethodPtr method, [[maybe_unused]] void *entryPoint) {}

    virtual bool TrySetOsrCode([[maybe_unused]] MethodPtr method, [[maybe_unused]] void *entryPoint)
    {
        return false;
    }

    virtual void *GetOsrCode([[maybe_unused]] MethodPtr method)
    {
        return nullptr;
    }

    virtual bool HasCompiledCode([[maybe_unused]] MethodPtr method)
    {
        return false;
    }

    virtual uint32_t GetAccessFlagAbstractMask() const
    {
        return 0;
    }

    virtual uint32_t GetVTableIndex([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual bool IsMethodExternal([[maybe_unused]] MethodPtr method, [[maybe_unused]] MethodPtr calleeMethod) const
    {
        return false;
    }

    virtual bool IsClassExternal([[maybe_unused]] MethodPtr method, [[maybe_unused]] ClassPtr calleeClass) const
    {
        return true;
    }

    virtual bool IsMethodAbstract([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodIntrinsic([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodIntrinsic([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] MethodId id) const
    {
        return false;
    }

    virtual MethodPtr GetMethodAsIntrinsic([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] MethodId id) const
    {
        return nullptr;
    }

    // return true if the method is Native with exception
    virtual bool HasNativeException([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodStatic([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] MethodId id) const
    {
        return false;
    }

    virtual bool IsMethodStatic([[maybe_unused]] MethodPtr method) const
    {
        return true;
    }

    virtual bool IsMethodNativeApi([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodInModuleScope([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsNecessarySwitchThreadState([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual uint8_t GetStackReferenceMask() const
    {
        return 0U;
    }

    virtual bool CanNativeMethodUseObjects([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsNativeMethodOptimizationEnabled() const
    {
        return false;
    }

    virtual void *GetMethodNativePointer([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    virtual uint32_t GetRuntimeClassOffset([[maybe_unused]] Arch arch) const
    {
        return 0U;
    }

    virtual bool IsMethodFinal([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual size_t ResolveInlinableNativeMethod([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual bool IsInlinableNativeMethod([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodCanBeInlined([[maybe_unused]] MethodPtr method) const
    {
        return true;
    }

    virtual bool IsMethodStaticConstructor([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodStringConcat([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodStringGetLength([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual Field *GetFieldPtrByName([[maybe_unused]] ClassPtr klass, [[maybe_unused]] std::string_view name) const
    {
        return nullptr;
    }

    virtual bool IsMethodStringBuilderConstructorWithStringArg([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodStringBuilderConstructorWithCharArrayArg([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodStringBuilderDefaultConstructor([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodStringBuilderToString([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodStringBuilderGetStringLength([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual FieldPtr GetInteropConstantPoolOffsetField([[maybe_unused]] ClassPtr klass) const
    {
        return nullptr;
    }

    virtual bool IsMethodStringBuilderAppend([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsClassEscompatArray([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassStringBuilder([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatInt8Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatUint8Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatUint8ClampedArray([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatInt16Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatUint16Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatInt32Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatUint32Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatFloat32Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatFloat64Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatBigInt64Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatBigUint64Array([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassEscompatTypedArray([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsFieldTypedArrayLengthInt([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool IsMethodTypedArrayCtor([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual uint32_t GetClassOffsetObjectsArray([[maybe_unused]] MethodPtr method) const
    {
        UNREACHABLE();
    }

    virtual bool IsFieldBooleanFalse([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool IsFieldBooleanTrue([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool IsFieldBooleanValue([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool IsFieldStringBuilderBuffer([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool IsFieldStringBuilderIndex([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual FieldPtr GetFieldStringBuilderBuffer([[maybe_unused]] ClassPtr klass) const
    {
        return nullptr;
    }

    virtual FieldPtr GetFieldStringBuilderIndex([[maybe_unused]] ClassPtr klass) const
    {
        return nullptr;
    }

    virtual FieldPtr GetFieldStringBuilderLength([[maybe_unused]] ClassPtr klass) const
    {
        return nullptr;
    }

    virtual FieldPtr GetFieldStringBuilderCompress([[maybe_unused]] ClassPtr klass) const
    {
        return nullptr;
    }

    virtual MethodPtr FindMethodByName([[maybe_unused]] const std::string &methodName) const
    {
        return nullptr;
    }

    virtual MethodPtr FindMethodByName([[maybe_unused]] const std::string &methodName,
                                       [[maybe_unused]] const std::string &methodSignature) const
    {
        return nullptr;
    }

    virtual MethodPtr GetGetterStringBuilderStringLength() const
    {
        return nullptr;
    }

    virtual MethodPtr GetMethodStringBuilderAppendString([[maybe_unused]] ClassPtr klass) const
    {
        return nullptr;
    }

    virtual std::string GetFileName([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownFile";
    }

    virtual std::string GetFullFileName([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownFile";
    }

    virtual std::string GetClassNameFromMethod([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownClass";
    }

    virtual std::string GetClassName([[maybe_unused]] ClassPtr klass) const
    {
        return "UnknownClass";
    }

    virtual ClassPtr GetClass([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    // returns Class for Field
    virtual ClassPtr GetClassForField([[maybe_unused]] FieldPtr field) const
    {
        return nullptr;
    }

    ClassPtr ResolveClassForField(MethodPtr method, size_t fieldId);

    virtual bool IsInstantiable([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsBoxedClass([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsEnumClass([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsBigIntClass([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsFunctionReference([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassBoxedBoolean([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassBoxedFloat([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsClassBoxedDouble([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual DataType::Type GetBoxedClassDataType([[maybe_unused]] ClassPtr klass) const
    {
        return DataType::NO_TYPE;
    }

    virtual std::string GetMethodName([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownMethod";
    }

    virtual std::string GetLineNumberAndSourceFile([[maybe_unused]] MethodPtr method,
                                                   [[maybe_unused]] uint32_t pc) const
    {
        return "UnknownSource";
    }

    virtual std::string GetExternalMethodName([[maybe_unused]] MethodPtr method,
                                              [[maybe_unused]] uint32_t externalId) const
    {
        return "UnknownExternalMethod";
    }

    virtual int64_t GetBranchTakenCounter([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t pc) const
    {
        return 0;
    }

    virtual int64_t GetBranchNotTakenCounter([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t pc) const
    {
        return 0;
    }

    virtual int64_t GetThrowTakenCounter([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t pc) const
    {
        return 0;
    }

    virtual bool IsConstructor([[maybe_unused]] MethodPtr method, [[maybe_unused]] SourceLanguage lang)
    {
        return false;
    }

    // returns true if need to encode memory barrier before return
    virtual bool IsMemoryBarrierRequired([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual std::string GetMethodFullName([[maybe_unused]] MethodPtr method, [[maybe_unused]] bool withSignature) const
    {
        return "UnknownMethod";
    }

    std::string GetMethodFullName(MethodPtr method) const
    {
        return GetMethodFullName(method, false);
    }

    virtual std::string GetMethodNameById([[maybe_unused]] size_t id) const
    {
        return "UnknownMethod";
    }

    virtual std::string GetBytecodeString([[maybe_unused]] MethodPtr method, [[maybe_unused]] uintptr_t pc) const
    {
        return std::string();
    }

    virtual ark::pandasm::LiteralArray GetLiteralArray([[maybe_unused]] MethodPtr method,
                                                       [[maybe_unused]] LiteralArrayId id) const
    {
        return ark::pandasm::LiteralArray();
    }

    virtual bool IsInterfaceMethod([[maybe_unused]] MethodPtr parentMethod, [[maybe_unused]] MethodId id) const
    {
        return false;
    }

    virtual bool IsInterfaceMethod([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsInstanceConstructor([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsDestroyed([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool CanThrowException([[maybe_unused]] MethodPtr method) const
    {
        return true;
    }

    virtual uint32_t FindCatchBlock([[maybe_unused]] MethodPtr method, [[maybe_unused]] ClassPtr cls,
                                    [[maybe_unused]] uint32_t pc) const
    {
        return panda_file::INVALID_OFFSET;
    }

    // Method offsets
    uint32_t GetAccessFlagsOffset(Arch arch) const
    {
        return ark::cross_values::GetMethodAccessFlagsOffset(arch);
    }
    uint32_t GetVTableIndexOffset(Arch arch) const
    {
        return ark::cross_values::GetMethodVTableIndexOffset(arch);
    }
    uint32_t GetClassOffset(Arch arch) const
    {
        return ark::cross_values::GetMethodClassOffset(arch);
    }
    uint32_t GetBaseClassFlagsOffset(Arch arch) const
    {
        return cross_values::GetBaseClassFlagsOffset(arch);
    }
    uint32_t GetCompiledEntryPointOffset(Arch arch) const
    {
        return ark::cross_values::GetMethodCompiledEntryPointOffset(arch);
    }
    uint32_t GetNativePointerOffset(Arch arch) const
    {
        return ark::cross_values::GetMethodNativePointerOffset(arch);
    }
    uint32_t GetPandaFileOffset(Arch arch) const
    {
        return ark::cross_values::GetMethodPandaFileOffset(arch);
    }

    virtual uint32_t GetCallableMask() const
    {
        return 0;
    }

    /// Exec state information
    size_t GetTlsFrameKindOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadFrameKindOffset(arch);
    }
    uint32_t GetFlagAddrOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadFlagOffset(arch);
    }
    size_t GetTlsFrameOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadFrameOffset(arch);
    }
    size_t GetExceptionOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadExceptionOffset(arch);
    }
    size_t GetTlsNativePcOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadNativePcOffset(arch);
    }
    size_t GetTlsCardTableAddrOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadCardTableAddrOffset(arch);
    }
    size_t GetTlsCardTableMinAddrOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadCardTableMinAddrOffset(arch);
    }
    size_t GetTlsPreWrbEntrypointOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadPreWrbEntrypointOffset(arch);
    }
    virtual size_t GetTlsPromiseClassPointerOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }
    virtual size_t GetTlsNativeApiOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }
    virtual uint64_t GetUniqueObject() const
    {
        return 0;
    }
    virtual size_t GetTlsUniqueObjectOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual ::ark::mem::BarrierType GetPreReadType() const
    {
        return ::ark::mem::BarrierType::PRE_RB_NONE;
    }

    virtual ::ark::mem::BarrierType GetPreType() const
    {
        return ::ark::mem::BarrierType::PRE_WRB_NONE;
    }

    virtual ::ark::mem::BarrierType GetPostType() const
    {
        return ::ark::mem::BarrierType::POST_WRB_NONE;
    }

    bool NeedsPreReadBarrier() const
    {
        auto type = GetPreReadType();
        return type == ::ark::mem::BarrierType::PRE_CMC_READ_BARRIER;
    }

    bool NeedsPreWriteBarrier() const
    {
        auto type = GetPreType();
        return type == ::ark::mem::BarrierType::PRE_SATB_BARRIER;
    }

    bool NeedsPostWriteBarrier() const
    {
        auto type = GetPostType();
        return type == ::ark::mem::BarrierType::POST_CMC_WRITE_BARRIER ||
               type == ::ark::mem::BarrierType::POST_INTERREGION_BARRIER;
    }

    virtual ::ark::mem::BarrierOperand GetBarrierOperand([[maybe_unused]] ::ark::mem::BarrierPosition barrierPosition,
                                                         [[maybe_unused]] std::string_view operandName) const
    {
        return ::ark::mem::BarrierOperand(::ark::mem::BarrierOperandType::PRE_WRITE_BARRIER_ADDRESS, false);
    }

    virtual uint32_t GetTlsGlobalObjectOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual uintptr_t GetGlobalVarAddress([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t id)
    {
        return 0;
    }

    /// Array information
    uintptr_t GetArrayU8ClassPointerTlsOffset(Arch arch) const
    {
        return cross_values::GetManagedThreadArrayU8ClassPtrOffset(arch);
    }

    uintptr_t GetArrayU16ClassPointerTlsOffset(Arch arch) const
    {
        return cross_values::GetManagedThreadArrayU16ClassPtrOffset(arch);
    }

    uint32_t GetClassArraySize(Arch arch) const
    {
        return ark::cross_values::GetCoretypesArrayClassSize(arch);
    }

    virtual uint32_t GetArrayElementSize([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return ARRAY_DEFAULT_ELEMENT_SIZE;
    }

    virtual uint32_t GetMaxArrayLength([[maybe_unused]] ClassPtr klass) const
    {
        return INT32_MAX;
    }

    virtual uintptr_t GetPointerToConstArrayData([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return 0;
    }

    virtual size_t GetOffsetToConstArrayData([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return 0;
    }

    virtual ClassPtr GetArrayU8Class([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    virtual ClassPtr GetArrayU16Class([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    // Array offsets
    uint32_t GetArrayDataOffset(Arch arch) const
    {
        return ark::cross_values::GetCoretypesArrayDataOffset(arch);
    }
    uint32_t GetArrayLengthOffset(Arch arch) const
    {
        return ark::cross_values::GetCoretypesArrayLengthOffset(arch);
    }

    /// String information
    virtual bool IsCompressedStringsEnabled() const
    {
        return true;
    }

    virtual uint32_t GetStringCompressionMask() const
    {
        return 1;
    }

    virtual ObjectPointerType GetNonMovableString([[maybe_unused]] MethodPtr method, [[maybe_unused]] StringId id) const
    {
        return 0;
    }

    virtual ClassPtr GetStringClass([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t *typeId) const
    {
        return nullptr;
    }

    virtual ClassPtr GetLineStringClass([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t *typeId) const
    {
        return nullptr;
    }

    virtual ClassPtr GetNumberClass([[maybe_unused]] MethodPtr method, [[maybe_unused]] const char *name,
                                    [[maybe_unused]] uint32_t *typeId) const
    {
        return nullptr;
    }

    virtual ClassPtr GetStringBuilderClass() const
    {
        return nullptr;
    }

    virtual MethodPtr GetStringBuilderDefaultConstructor() const
    {
        return nullptr;
    }

    // String offsets
    uint32_t GetStringDataOffset(Arch arch) const
    {
        return ark::cross_values::GetCoretypesStringDataOffset(arch);
    }
    uint32_t GetStringLengthOffset(Arch arch) const
    {
        return ark::cross_values::GetCoretypesStringLengthOffset(arch);
    }
    uintptr_t GetStringClassPointerTlsOffset(Arch arch) const
    {
        return cross_values::GetManagedThreadStringClassPtrOffset(arch);
    }

    // StringBuilder offsets: cross_values are not used as the core part
    // does not contain corresponding StringBuilder C++ representative.
    static constexpr uint32_t GetSbBufferOffset()
    {
        return ObjectHeader::ObjectHeaderSize();
    }
    static constexpr uint32_t GetSbIndexOffset()
    {
        return GetSbBufferOffset() + ark::OBJECT_POINTER_SIZE;
    }
    static constexpr uint32_t GetSbLengthOffset()
    {
        return GetSbIndexOffset() + sizeof(int32_t);
    }
    static constexpr uint32_t GetSbCompressOffset()
    {
        return GetSbLengthOffset() + sizeof(int32_t);
    }

    virtual std::string GetStringValue([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t id) const
    {
        return std::string("");
    }

    /// managed Thread object information

    uint32_t GetThreadObjectOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadObjectOffset(arch);
    }

    /// TLAB information

    virtual size_t GetTLABMaxSize() const
    {
        return 0;
    }

    virtual size_t GetTLABAlignment() const
    {
        return 1;
    }

    virtual bool IsTrackTlabAlloc() const
    {
        return false;
    }

    // TLAB offsets
    size_t GetCurrentTLABOffset(Arch arch) const
    {
        return ark::cross_values::GetManagedThreadTlabOffset(arch);
    }
    size_t GetTLABStartPointerOffset(Arch arch) const
    {
        return ark::cross_values::GetTlabMemoryStartAddrOffset(arch);
    }
    size_t GetTLABFreePointerOffset(Arch arch) const
    {
        return ark::cross_values::GetTlabCurFreePositionOffset(arch);
    }
    size_t GetTLABEndPointerOffset(Arch arch) const
    {
        return ark::cross_values::GetTlabMemoryEndAddrOffset(arch);
    }

    /// Object information
    virtual ClassPtr GetClass([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return nullptr;
    }

    virtual ClassType GetClassType([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return ClassType::UNRESOLVED_CLASS;
    }

    virtual ClassType GetClassType([[maybe_unused]] ClassPtr klassPtr) const
    {
        return ClassType::UNRESOLVED_CLASS;
    }

    virtual bool IsArrayClass([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return false;
    }

    virtual bool IsStringClass([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return false;
    }

    virtual bool IsStringClass([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual bool IsArrayClass([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual ClassPtr GetArrayElementClass([[maybe_unused]] ClassPtr cls) const
    {
        return nullptr;
    }

    virtual bool CheckStoreArray([[maybe_unused]] ClassPtr arrayCls, [[maybe_unused]] ClassPtr strCls) const
    {
        return false;
    }

    virtual bool IsAssignableFrom([[maybe_unused]] ClassPtr cls1, [[maybe_unused]] ClassPtr cls2) const
    {
        return false;
    }

    virtual size_t GetObjectHashedStatusBitNum() const
    {
        return 0;
    }

    virtual size_t GetObjectHashShift() const
    {
        return 0;
    }

    virtual size_t GetObjectHashMask() const
    {
        return 0;
    }

    // Offset of class in ObjectHeader
    uint32_t GetObjClassOffset(Arch arch) const
    {
        return ark::cross_values::GetObjectHeaderClassPointerOffset(arch);
    }

    // Offset of the managed object in the BaseClass
    uint32_t GetManagedClassOffset(Arch arch) const
    {
        return ark::cross_values::GetBaseClassManagedObjectOffset(arch);
    }

    // Offset of mark word in ObjectHeader
    uint32_t GetObjMarkWordOffset(Arch arch) const
    {
        return ark::cross_values::GetObjectHeaderMarkWordOffset(arch);
    }

    uint64_t GetTaggedTagMask() const
    {
        return coretypes::TaggedValue::TAG_MASK;
    }

    uint64_t GetTaggedSpecialMask() const
    {
        return coretypes::TaggedValue::TAG_SPECIAL_MASK;
    }

    virtual StringCtorType GetStringCtorType([[maybe_unused]] MethodPtr ctorMethod) const
    {
        return StringCtorType::UNKNOWN;
    }

    /// Class information

    // Returns Class Id for Field.
    // We don't get class id directly from the field's class, because we need a class id regarding to the current
    // file. Class id is used in codegen to initialize class in aot mode.
    virtual size_t GetClassIdForField([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t unused) const
    {
        return 0;
    }

    virtual size_t GetClassIdForField([[maybe_unused]] FieldPtr field) const
    {
        return 0;
    }

    // returns Class Id for Method
    virtual uint32_t GetClassIdForMethod([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    // returns Class Id for Method
    virtual uint32_t GetClassIdForMethod([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t unused) const
    {
        return 0;
    }

    virtual uint64_t GetUniqClassId([[maybe_unused]] ClassPtr klass) const
    {
        return 0;
    }

    virtual IdType GetClassIdWithinFile([[maybe_unused]] MethodPtr method, [[maybe_unused]] ClassPtr klass) const
    {
        return 0;
    }

    virtual IdType GetLiteralArrayClassIdWithinFile([[maybe_unused]] MethodPtr method,
                                                    [[maybe_unused]] panda_file::LiteralTag tag) const
    {
        return 0;
    }

    virtual bool CanUseTlabForClass([[maybe_unused]] ClassPtr klass) const
    {
        return true;
    }

    // returns class size
    virtual size_t GetClassSize([[maybe_unused]] ClassPtr klass) const
    {
        return 0;
    }

    virtual bool CanScalarReplaceObject([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    // Vtable offset in Class
    uint32_t GetVTableOffset(Arch arch) const
    {
        return ark::cross_values::GetClassVtableOffset(arch);
    }

    // returns base offset in Class(for array)
    uint32_t GetClassBaseOffset(Arch arch) const
    {
        return ark::cross_values::GetClassBaseOffset(arch);
    }

    // returns component type offset in Class(for array)
    uint32_t GetClassComponentTypeOffset(Arch arch) const
    {
        return ark::cross_values::GetClassComponentTypeOffset(arch);
    }

    // returns type offset in Class(for array)
    uint32_t GetClassTypeOffset(Arch arch) const
    {
        return ark::cross_values::GetClassTypeOffset(arch);
    }

    uint32_t GetClassStateOffset(Arch arch) const
    {
        return ark::cross_values::GetClassStateOffset(arch);
    }

    uint32_t GetClassMethodsOffset(Arch arch) const
    {
        return ark::cross_values::GetClassMethodsOffset(arch);
    }

    /// Field information

    /**
     * Try to resolve field.
     * @param method method to which the field belongs
     * @param id id of the field
     * @param allow_external allow fields defined in the external file, if false - return nullptr for external fields
     * @param class_id output variable where will be written a field's class
     * @return return field or nullptr if it cannot be resolved
     */
    virtual FieldPtr ResolveField([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t unused,
                                  [[maybe_unused]] bool isStatic, [[maybe_unused]] bool allowExternal,
                                  [[maybe_unused]] uint32_t *classId)
    {
        return nullptr;
    }

    virtual FieldPtr ResolveLookUpField([[maybe_unused]] FieldPtr rawField, [[maybe_unused]] ClassPtr klass)
    {
        return nullptr;
    }

    virtual DataType::Type GetFieldType([[maybe_unused]] FieldPtr field) const
    {
        return DataType::NO_TYPE;
    }

    virtual DataType::Type GetArrayComponentType([[maybe_unused]] ClassPtr klass) const
    {
        return DataType::NO_TYPE;
    }

    virtual DataType::Type GetFieldTypeById([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType unused) const
    {
        return DataType::NO_TYPE;
    }

    virtual IdType GetFieldValueTypeId([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return 0;
    }

    virtual size_t GetFieldOffset([[maybe_unused]] FieldPtr field) const
    {
        return 0;
    }

    virtual FieldPtr GetFieldByOffset([[maybe_unused]] size_t offset) const
    {
        return nullptr;
    }

    virtual uintptr_t GetFieldClass([[maybe_unused]] FieldPtr field) const
    {
        return 0;
    }

    virtual bool IsFieldVolatile([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool IsFieldFinal([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool IsFieldReadonly([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool HasFieldMetadata([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual uint64_t GetStaticFieldIntegerValue([[maybe_unused]] FieldPtr fieldPtr) const
    {
        return 0;
    }

    virtual double GetStaticFieldFloatValue([[maybe_unused]] FieldPtr fieldPtr) const
    {
        return 0;
    }

    virtual std::string GetFieldName([[maybe_unused]] FieldPtr field) const
    {
        return "UnknownField";
    }

    // Return offset of the managed object in the class
    uint32_t GetFieldClassOffset(Arch arch) const
    {
        return ark::cross_values::GetFieldClassOffset(arch);
    }

    // Return offset of the managed object in the class
    uint32_t GetFieldOffsetOffset(Arch arch) const
    {
        return ark::cross_values::GetFieldOffsetOffset(arch);
    }

    virtual size_t GetPropertyBoxOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual size_t GetElementsOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual size_t GetPropertiesOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual size_t GetHClassOffset([[maybe_unused]] Arch arch) const
    {
        UNREACHABLE();
    }

    virtual size_t GetHClassBitfieldTypeStartBit([[maybe_unused]] Arch arch) const
    {
        UNREACHABLE();
    }

    virtual uint64_t GetHClassBitfieldTypeMask([[maybe_unused]] Arch arch) const
    {
        UNREACHABLE();
    }

    virtual uint64_t GetJshclassBitfieldClassConstructorStartBit([[maybe_unused]] Arch arch) const
    {
        UNREACHABLE();
    }

    virtual size_t GetJstypeJsFunction([[maybe_unused]] Arch arch) const
    {
        UNREACHABLE();
    }

    virtual size_t GetPrototypeHolderOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual size_t GetPrototypeCellOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual size_t GetIsChangeFieldOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual size_t GetDynArrayLenthOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual FieldId GetFieldId([[maybe_unused]] FieldPtr field) const
    {
        return 0;
    }

    /// Type information
    virtual ClassPtr ResolveType([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t unused) const
    {
        return nullptr;
    }

    virtual bool IsClassInitialized([[maybe_unused]] uintptr_t unused) const
    {
        return false;
    }

    virtual bool IsClassFinal([[maybe_unused]] ClassPtr unused) const
    {
        return false;
    }

    virtual bool IsClassFinalOrString([[maybe_unused]] ClassPtr unused) const
    {
        return false;
    }

    virtual bool IsInterface([[maybe_unused]] ClassPtr unused) const
    {
        return false;
    }

    virtual uintptr_t GetManagedType([[maybe_unused]] uintptr_t unused) const
    {
        return 0;
    }

    virtual uint8_t GetClassInitializedValue() const
    {
        return 0;
    }

    virtual uint8_t GetReferenceTypeMask() const
    {
        return 0;
    }

    virtual size_t GetProtectedMemorySize() const
    {
        // Conservatively, we believe that one page is protected. There can't be less than one.
        return PANDA_PAGE_SIZE;
    }

    /// Entrypoints
#include "compiler_interface_extensions.inl.h"
#include <intrinsics_enum.inl>
#include <entrypoints_compiler.inl>
#include <entrypoints_compiler_checksum.inl>

    virtual IntrinsicId GetIntrinsicId([[maybe_unused]] MethodPtr method) const
    {
        return IntrinsicId::INVALID;
    }

    virtual IntrinsicId GetCompilerNullcheckIntrinsicId() const
    {
        return IntrinsicId::INVALID;
    }

    virtual uintptr_t GetIntrinsicAddress([[maybe_unused]] bool runtimeCall, [[maybe_unused]] SourceLanguage lang,
                                          [[maybe_unused]] IntrinsicId unused) const
    {
        return 0;
    }

    virtual bool IsIntrinsicStringBuilderToString([[maybe_unused]] IntrinsicId id) const
    {
        return false;
    }

    virtual bool IsIntrinsicStringBuilderAppendString([[maybe_unused]] IntrinsicId id) const
    {
        return false;
    }

    virtual bool IsIntrinsicStringBuilderAppend([[maybe_unused]] IntrinsicId id) const
    {
        return false;
    }

    virtual bool IsIntrinsicStringConcat([[maybe_unused]] IntrinsicId id) const
    {
        return false;
    }

    virtual IntrinsicId ConvertTypeToStringBuilderAppendIntrinsicId([[maybe_unused]] DataType::Type type) const
    {
        UNREACHABLE();
    }

    virtual IntrinsicId GetStringConcatStringsIntrinsicId([[maybe_unused]] size_t numArgs) const
    {
        UNREACHABLE();
    }

    virtual IntrinsicId GetStringIsCompressedIntrinsicId() const
    {
        UNREACHABLE();
    }

    virtual IntrinsicId GetStringBuilderAppendStringsIntrinsicId([[maybe_unused]] size_t numArgs) const
    {
        UNREACHABLE();
    }

    virtual IntrinsicId GetStringBuilderToStringIntrinsicId() const
    {
        UNREACHABLE();
    }

    virtual bool IsClassValueTyped([[maybe_unused]] ClassPtr klass) const
    {
        UNREACHABLE();
    }

    uintptr_t GetEntrypointsTablePointerTlsOffset(Arch arch) const
    {
        return cross_values::GetManagedThreadEntrypointsTableOffset(arch);
    }

    uintptr_t GetEntrypointOffset(Arch arch, EntrypointId id) const
    {
        return cross_values::GetEntrypointOffset(arch, ark::EntrypointId(id));
    }

    virtual EntrypointId GetGlobalVarEntrypointId()
    {
        return static_cast<EntrypointId>(0);
    }

    virtual InteropCallKind GetInteropCallKind([[maybe_unused]] MethodPtr method) const
    {
        return InteropCallKind::UNKNOWN;
    }

    virtual void GetInfoForInteropCallArgsConversion(
        [[maybe_unused]] MethodPtr methodPtr, [[maybe_unused]] uint32_t skipArgs,
        [[maybe_unused]] ArenaVector<std::pair<IntrinsicId, DataType::Type>> *intrinsics) const
    {
        UNREACHABLE();
    }

    virtual std::optional<std::pair<IntrinsicId, compiler::DataType::Type>> GetInfoForInteropCallRetValueConversion(
        [[maybe_unused]] MethodPtr methodPtr) const
    {
        UNREACHABLE();
    }

    virtual char *GetFuncPropName([[maybe_unused]] MethodPtr methodPtr, [[maybe_unused]] uint32_t strId) const
    {
        UNREACHABLE();
    }

    virtual uint64_t GetFuncPropNameOffset([[maybe_unused]] MethodPtr methodPtr, [[maybe_unused]] uint32_t strId) const
    {
        UNREACHABLE();
    }

    virtual uint32_t GetAnnotationElementUniqueIndex([[maybe_unused]] MethodPtr methodPtr,
                                                     [[maybe_unused]] const char *annotation,
                                                     [[maybe_unused]] uint32_t index)
    {
        UNREACHABLE();
    }

    virtual ClassPtr GetRetValueClass([[maybe_unused]] MethodPtr methodPtr) const
    {
        UNREACHABLE();
    }

    /// Dynamic object information

    virtual uint32_t GetFunctionTargetOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual uint64_t GetDynamicPrimitiveUndefined() const
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::Undefined().GetRawData());
    }

    virtual uint64_t GetPackConstantByPrimitiveType(compiler::AnyBaseType type, uint64_t imm) const
    {
        auto datatype = AnyBaseTypeToDataType(type);
        if (datatype == DataType::INT32) {
            return coretypes::TaggedValue::GetIntTaggedValue(imm);
        }
        if (datatype == DataType::FLOAT64) {
            return coretypes::TaggedValue::GetDoubleTaggedValue(imm);
        }
        if (datatype == DataType::BOOL) {
            return coretypes::TaggedValue::GetBoolTaggedValue(imm);
        }
        UNREACHABLE();
        return 0;
    }

    virtual uint64_t GetDynamicPrimitiveFalse() const
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::False().GetRawData());
    }

    virtual uint64_t GetDynamicPrimitiveTrue() const
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::True().GetRawData());
    }

    virtual uint64_t DynamicCastDoubleToInt([[maybe_unused]] double value, [[maybe_unused]] size_t bits) const
    {
        return 0;
    }

    virtual uint8_t GetDynamicNumFixedArgs() const
    {
        return 0;
    }

    virtual size_t GetLanguageExtensionSize([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual uint32_t GetNativePointerTargetOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    /**
     * Check if GC can be triggered during call.
     * This is possible when method A calling method B and waiting while B is compiling.
     */
    virtual bool HasSafepointDuringCall() const
    {
        return false;
    }

    /// Bytecode profiling
    using BytecodeProfile = uintptr_t;
    using MethodProfile = profiling::ProfileType;

    /**
     * Get profile information for a specific method.
     *
     * @param method method, which profile data we want to get
     * @param from_vector this flag indicates from where we should get profile: from method's profile vector or from
     *                    a profile file, which was added via `AddProfile`.
     *                    NB: This flag is a workaround and should be deleted. Problem is that Runtime stores methods
     *                    vector in different places for different languages. But Paoc uses only place for core part,
     *                    i.e. methods in Class objects, that is wrong for some languages.
     *                    NOTE: create interface in the runtime to enumerate all methods despite of VM language.
     * @return profile data for the given method
     */
    virtual MethodProfile GetMethodProfile([[maybe_unused]] MethodPtr method, [[maybe_unused]] bool fromVector) const
    {
        return nullptr;
    }

    virtual profiling::CallKind GetCallProfile([[maybe_unused]] MethodPtr profile, [[maybe_unused]] uint32_t pc,
                                               [[maybe_unused]] ArenaVector<uintptr_t> *methods,
                                               [[maybe_unused]] bool isAot)
    {
        return profiling::CallKind::UNKNOWN;
    }

    /**
     * Get profile for a specific bytecode. Usually profile is a memory buffer, that is treated in different ways,
     * according to the bytecode operation.
     *
     * @param prof Method profile, which should be retrived via `GetMethodProfile` method.
     * @param bc_inst pointer to the bytecode instruction
     * @param pc offset of the bytecode instruction from the bytecode buffer beginning.
     * @return return profile for `bc_inst`
     */
    virtual BytecodeProfile GetBytecodeProfile([[maybe_unused]] MethodProfile prof,
                                               [[maybe_unused]] const uint8_t *bcInst, [[maybe_unused]] size_t pc) const
    {
        return 0;
    }

    virtual bool CanInlineLdStObjByIndex([[maybe_unused]] const BytecodeInstruction *bcInst, [[maybe_unused]] size_t pc,
                                         [[maybe_unused]] MethodProfile methodProfile) const
    {
        return false;
    }

    virtual Expected<bool, const char *> AddProfile([[maybe_unused]] std::string_view fname)
    {
        return Unexpected("Not implemented");
    }

    virtual compiler::AnyBaseType GetProfilingAnyType([[maybe_unused]] RuntimeInterface::BytecodeProfile profile,
                                                      [[maybe_unused]] const BytecodeInstruction *bcInst,
                                                      [[maybe_unused]] unsigned index,
                                                      [[maybe_unused]] profiling::AnyInputType *allowedInputType,
                                                      [[maybe_unused]] bool *isTypeProfiled)
    {
        return compiler::AnyBaseType::UNDEFINED_TYPE;
    }

    virtual compiler::AnyBaseType ResolveSpecialAnyTypeByConstant([[maybe_unused]] coretypes::TaggedValue anyConst)
    {
        return compiler::AnyBaseType::UNDEFINED_TYPE;
    }

    virtual void *GetConstantPool([[maybe_unused]] MethodPtr method)
    {
        return nullptr;
    }

    virtual void *GetConstantPool([[maybe_unused]] uintptr_t funcAddress)
    {
        return nullptr;
    }

    virtual ThreadPtr CreateCompilerThread()
    {
        return nullptr;
    }

    virtual void DestroyCompilerThread([[maybe_unused]] ThreadPtr thread)
    {
        UNREACHABLE();
    }

    virtual void SetCurrentThread([[maybe_unused]] ThreadPtr thread) const
    {
        UNREACHABLE();
    }

    virtual ThreadPtr GetCurrentThread() const
    {
        UNREACHABLE();
    }

    virtual void *GetDoubleToStringCache() const
    {
        return nullptr;
    }

    virtual void *GetAsciiCharCache() const
    {
        return nullptr;
    }

    virtual bool IsStringCachesUsed() const
    {
        return false;
    }

    virtual bool CanUseStringFlatCheck() const
    {
        return false;
    }

    virtual bool IsUseAllStrings() const
    {
        return false;
    }

    NO_COPY_SEMANTIC(RuntimeInterface);
    NO_MOVE_SEMANTIC(RuntimeInterface);

private:
    static constexpr uint32_t ARRAY_DEFAULT_ELEMENT_SIZE = 4;
};

class IClassHierarchyAnalysis {
public:
    IClassHierarchyAnalysis() = default;
    virtual ~IClassHierarchyAnalysis() = default;

public:
    virtual RuntimeInterface::MethodPtr GetSingleImplementation(
        [[maybe_unused]] RuntimeInterface::MethodPtr method) = 0;
    virtual bool IsSingleImplementation([[maybe_unused]] RuntimeInterface::MethodPtr method) = 0;
    virtual void AddDependency([[maybe_unused]] RuntimeInterface::MethodPtr caller,
                               [[maybe_unused]] RuntimeInterface::MethodPtr callee) = 0;

    NO_COPY_SEMANTIC(IClassHierarchyAnalysis);
    NO_MOVE_SEMANTIC(IClassHierarchyAnalysis);
};

class InlineCachesInterface {
public:
    using ClassList = Span<RuntimeInterface::ClassPtr>;
    enum class CallKind { UNKNOWN, MONOMORPHIC, POLYMORPHIC, MEGAMORPHIC };

    virtual CallKind GetClasses(RuntimeInterface::MethodPtr method, uintptr_t unused,
                                ArenaVector<RuntimeInterface::ClassPtr> *classes) = 0;
    virtual ~InlineCachesInterface() = default;
    InlineCachesInterface() = default;

    DEFAULT_COPY_SEMANTIC(InlineCachesInterface);
    DEFAULT_MOVE_SEMANTIC(InlineCachesInterface);
};

class UnresolvedTypesInterface {
public:
    enum class SlotKind { UNKNOWN, CLASS, MANAGED_CLASS, METHOD, VIRTUAL_METHOD, FIELD, STATIC_FIELD_PTR };
    virtual bool AddTableSlot([[maybe_unused]] RuntimeInterface::MethodPtr method, [[maybe_unused]] uint32_t typeId,
                              [[maybe_unused]] SlotKind kind) = 0;
    virtual uintptr_t GetTableSlot([[maybe_unused]] RuntimeInterface::MethodPtr method,
                                   [[maybe_unused]] uint32_t typeId, [[maybe_unused]] SlotKind kind) const = 0;
    virtual ~UnresolvedTypesInterface() = default;
    UnresolvedTypesInterface() = default;

    DEFAULT_COPY_SEMANTIC(UnresolvedTypesInterface);
    DEFAULT_MOVE_SEMANTIC(UnresolvedTypesInterface);
};

enum class TraceId {
    METHOD_ENTER = 1U << 0U,
    METHOD_EXIT = 1U << 1U,
    PRINT_ARG = 1U << 2U,
};

enum class DeoptimizeType : uint8_t {
    INVALID = 0,
    INLINE_CHA,
    NULL_CHECK,
    BOUNDS_CHECK,
    ZERO_CHECK,
    NEGATIVE_CHECK,
    CHECK_CAST,
    ANY_TYPE_CHECK,
    OVERFLOW_TYPE,
    HOLE,
    NOT_NUMBER,
    NOT_SUPPORTED_NATIVE,
    CAUSE_METHOD_DESTRUCTION,
    NOT_SMALL_INT = CAUSE_METHOD_DESTRUCTION,
    BOUNDS_CHECK_WITH_DEOPT,
    DOUBLE_WITH_INT,
    INLINE_IC,
    INLINE_DYN,
    NOT_PROFILED,
    IFIMM_TRY,
    COUNT
};

inline constexpr auto DEOPT_COUNT = static_cast<uint8_t>(DeoptimizeType::COUNT);

inline constexpr std::array<const char *, DEOPT_COUNT> DEOPT_TYPE_NAMES = {"INVALID_TYPE",    "INLINE_CHA",
                                                                           "NULL_CHECK",      "BOUNDS_CHECK",
                                                                           "ZERO_CHECK",      "NEGATIVE_CHECK",
                                                                           "CHECK_CAST",      "ANY_TYPE_CHECK",
                                                                           "OVERFLOW",        "HOLE ",
                                                                           "NOT_NUMBER",      "NOT_SUPPORTED_NATIVE",
                                                                           "NOT_SMALL_INT",   "BOUNDS_CHECK_WITH_DEOPT",
                                                                           "DOUBLE_WITH_INT", "INLINE_IC",
                                                                           "INLINE_DYN",      "NOT_PROFILED",
                                                                           "IFIMM_TRY"};

inline const char *DeoptimizeTypeToString(DeoptimizeType deoptType)
{
    auto idx = static_cast<uint8_t>(deoptType);
    ASSERT(idx < DEOPT_COUNT);
    return DEOPT_TYPE_NAMES[idx];
}
}  // namespace ark::compiler

#endif  // COMPILER_RUNTIME_INTERFACE_H
