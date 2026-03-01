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


#ifndef COMMON_ALIAS_SRC
#define COMMON_ALIAS_SRC

extern "C" MRT_EXPORT ObjRef CODE_MCC_OnFinalizerCreated(ObjRef ref) __attribute__((alias("MCC_OnFinalizerCreated")));
extern "C" MRT_EXPORT void CODE_MCC_WriteRefField(const ObjectPtr ref, const ObjectPtr obj, RefField<false>* field)
    __attribute__((alias("MCC_WriteRefField")));

extern "C" MRT_EXPORT void CODE_MCC_WriteStructField(const ObjectPtr obj, MAddress dst, size_t dstLen,
                                                   MAddress src, size_t srcLen)
    __attribute__((alias("MCC_WriteStructField")));

extern "C" MRT_EXPORT void CODE_MCC_WriteStaticRef(const ObjectPtr ref, RefField<false>* field)
    __attribute__((alias("MCC_WriteStaticRef")));
extern "C" MRT_EXPORT void CODE_MCC_WriteStaticStruct(MAddress dst, size_t dstLen, MAddress src, size_t srcLen,
                                                    const GCTib gcTib) __attribute__((alias("MCC_WriteStaticStruct")));
extern "C" MRT_EXPORT void CODE_MCC_AtomicWriteReference(const ObjectPtr ref, const ObjectPtr obj, RefField<true>* field,
                                                       MemoryOrder order)
    __attribute__((alias("MCC_AtomicWriteReference")));

extern "C" MRT_EXPORT __attribute__((alias("MCC_AtomicReadReference")))
    ObjectPtr CODE_MCC_AtomicReadReference(const ObjectPtr obj, RefField<true>* field, MemoryOrder order);

extern "C" MRT_EXPORT ObjectPtr CODE_MCC_AtomicSwapReference(const ObjectPtr ref, const ObjectPtr obj,
                                                           RefField<true>* field, MemoryOrder order)
    __attribute__((alias("MCC_AtomicSwapReference")));
extern "C" MRT_EXPORT bool CODE_MCC_AtomicCompareAndSwapReference(const ObjectPtr oldRef, const ObjectPtr newRef,
                                                                const ObjectPtr obj, RefField<true>* field,
                                                                MemoryOrder succOrder, MemoryOrder failOrder)
    __attribute__((alias("MCC_AtomicCompareSwapReference")));
extern "C" MRT_EXPORT void CODE_MCC_InvokeGCImpl(bool sync) __attribute__((alias("MCC_InvokeGCImpl")));
extern "C" MRT_EXPORT ssize_t CODE_MCC_GetRealHeapSize()
    __attribute__((alias("MCC_GetRealHeapSize")));
extern "C" MRT_EXPORT size_t CODE_MCC_GetAllocatedHeapSize() __attribute__((alias("MCC_GetAllocatedHeapSize")));
extern "C" MRT_EXPORT size_t CODE_MCC_GetMaxHeapSize() __attribute__((alias("MCC_GetMaxHeapSize")));
extern "C" MRT_EXPORT size_t CODE_MCC_GetCODEThreadNumber() __attribute__((alias("MCC_GetCODEThreadNumber")));
extern "C" MRT_EXPORT size_t CODE_MCC_GetBlockingCODEThreadNumber() __attribute__((alias("MCC_GetBlockingCODEThreadNumber")));
extern "C" MRT_EXPORT size_t CODE_MCC_GetNativeThreadNumber() __attribute__((alias("MCC_GetNativeThreadNumber")));
extern "C" MRT_EXPORT size_t CODE_MCC_GetGCCount() __attribute__((alias("MCC_GetGCCount")));
extern "C" MRT_EXPORT uint64_t CODE_MCC_GetGCTimeUs() __attribute__((alias("MCC_GetGCTimeUs")));
extern "C" MRT_EXPORT size_t CODE_MCC_GetGCFreedSize() __attribute__((alias("MCC_GetGCFreedSize")));
extern "C" MRT_EXPORT size_t CODE_MCC_StartCpuProfiling() __attribute__((alias("MCC_StartCpuProfiling")));
extern "C" MRT_EXPORT size_t CODE_MCC_StopCpuProfiling(int fd) __attribute__((alias("MCC_StopCpuProfiling")));
extern "C" MRT_EXPORT void CODE_MCC_SetGCThreshold(uint64_t GCThreshold) __attribute__((alias("MCC_SetGCThreshold")));
extern "C" MRT_EXPORT void* CODE_MCC_PostThrowException(ExceptionWrapper* mExceptionWrapper)
    __attribute__((alias("MCC_PostThrowException")));

#if !defined(ENABLE_BACKWARD_PTRAUTH_CFI)
extern "C" MRT_EXPORT MRT_OPTIONAL_BRANCH_PROTECT_NONE
void CODE_MCC_ThrowException(ExceptionRef exception) __attribute__((alias("MCC_ThrowException")));
#endif

extern "C" MRT_EXPORT void CODE_MCC_RegisterImplicitExceptionRaisers(void* raiser)
    __attribute__((alias("MCC_RegisterImplicitExceptionRaisers")));
extern "C" MRT_EXPORT void* CODE_MCC_GetExceptionWrapper() __attribute__((alias("MCC_GetExceptionWrapper")));
extern "C" MRT_EXPORT uint32_t CODE_MCC_GetExceptionTypeID() __attribute__((alias("MCC_GetExceptionTypeID")));
extern "C" MRT_EXPORT void CODE_MCC_EndCatch() __attribute__((alias("MCC_EndCatch")));

extern "C" MRT_EXPORT ArrayRef CODE_MCC_FillInStackTraceImpl(const TypeInfo* arrayInfo, const ArrayRef exceptionMessage)
    __attribute__((alias("MCC_FillInStackTraceImpl")));
extern "C" MRT_EXPORT void CODE_MCC_ReleaseRawData(ArrayRef array, void* rawPtr)
    __attribute__((alias("MCC_ReleaseRawData")));

extern "C" MRT_EXPORT TypeTemplate* CODE_MCC_GetTypeTemplate(char* name) __attribute__((alias("MCC_GetTypeTemplate")));
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetOrCreateTypeInfoForReflect(TypeTemplate* tt, void* args)
    __attribute__((alias("MCC_GetOrCreateTypeInfoForReflect")));
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetObjClass(const ObjectPtr obj) __attribute__((alias("MCC_GetObjClass")));

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetTypeForAny(const ObjectPtr obj) __attribute__((alias("MCC_GetTypeForAny")));

extern "C" MRT_EXPORT bool CODE_MCC_IsWrapperClassForAutoEnv(TypeInfo* ti)
    __attribute__((alias("MCC_IsWrapperClassForAutoEnv")));

// for dynamic loader
extern "C" MRT_EXPORT void* CODE_MCC_LoadPackage(const char* path) __attribute__((alias("MCC_LoadPackage")));

extern "C" MRT_EXPORT PackageInfo* CODE_MCC_GetPackageByQualifiedName(const char* packageName)
    __attribute__((alias("MCC_GetPackageByQualifiedName")));

extern "C" MRT_EXPORT const char* CODE_MCC_GetPackageVersion(PackageInfo* packageInfo)
    __attribute__((alias("MCC_GetPackageVersion")));

extern "C" MRT_EXPORT void* CODE_MCC_GetSubPackages(PackageInfo* packageInfo)
    __attribute__((alias("MCC_GetSubPackages")));

extern "C" MRT_EXPORT PackageInfo* CODE_MCC_GetRelatedPackageInfo(PackageInfo* packageInfo)
    __attribute__((alias("MCC_GetRelatedPackageInfo")));

extern "C" MRT_EXPORT char* CODE_MCC_GetPackageName(PackageInfo* packageInfo)
    __attribute__((alias("MCC_GetPackageName")));
extern "C" MRT_EXPORT U32 CODE_MCC_GetPackageNumOfTypeInfos(PackageInfo* packageInfo)
    __attribute__((alias("MCC_GetNumOfTypeInfos")));
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetPackageTypeInfo(PackageInfo* packageInfo, U32 index)
    __attribute__((alias("MCC_GetPackageTypeInfo")));

extern "C" MRT_EXPORT U32 CODE_MCC_GetPackageNumOfGlobalMethodInfos(PackageInfo* packageInfo)
    __attribute__((alias("MCC_GetPackageNumOfGlobalMethodInfos")));
extern "C" MRT_EXPORT MethodInfo* CODE_MCC_GetPackageGlobalMethodInfo(PackageInfo* packageInfo, U32 index)
    __attribute__((alias("MCC_GetPackageGlobalMethodInfo")));

extern "C" MRT_EXPORT U32 CODE_MCC_GetPackageNumOfGlobalFieldInfos(PackageInfo* packageInfo)
    __attribute__((alias("MCC_GetPackageNumOfGlobalFieldInfos")));
extern "C" MRT_EXPORT InstanceFieldInfo* CODE_MCC_GetPackageGlobalFieldInfo(PackageInfo* packageInfo, U32 index)
    __attribute__((alias("MCC_GetPackageGlobalFieldInfo")));

// for reflect, method
extern "C" MRT_EXPORT const char* CODE_MCC_GetMethodName(MethodInfo* methodInfo)
    __attribute__((alias("MCC_GetMethodName")));
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetMethodReturnType(MethodInfo* methodInfo)
    __attribute__((alias("MCC_GetMethodReturnType")));
extern "C" MRT_EXPORT U32 CODE_MCC_GetMethodModifier(MethodInfo* methodInfo)
    __attribute__((alias("MCC_GetMethodModifier")));
extern "C" MRT_EXPORT U32 CODE_MCC_MethodEntryPointIsNull(MethodInfo* methodInfo)
    __attribute__((alias("MCC_MethodEntryPointIsNull")));

extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfActualParameters(MethodInfo* methodInfo)
    __attribute__((alias("MCC_GetNumOfActualParameters")));

extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfGenericParameters(MethodInfo* methodInfo)
    __attribute__((alias("MCC_GetNumOfGenericParameters")));

extern "C" MRT_EXPORT ParameterInfoRef CODE_MCC_GetActualParameterInfo(MethodInfo* methodInfo, U32 index)
    __attribute__((alias("MCC_GetActualParameterInfo")));

extern "C" MRT_EXPORT GenericTypeInfo* CODE_MCC_GetGenericParameterInfo(MethodInfo* methodInfo, U32 index)
    __attribute__((alias("MCC_GetGenericParameterInfo")));

extern "C" MRT_EXPORT GenericTypeInfo* CODE_MCC_CheckMethodActualArgs(MethodInfo* methodInfo,
    void* genericArgs, void* args) __attribute__((alias("MCC_CheckMethodActualArgs")));

// for reflect, field
extern "C" MRT_EXPORT const char* CODE_MCC_GetInstanceFieldName(InstanceFieldInfo* fieldInfo)
    __attribute__((alias("MCC_GetInstanceFieldName")));
extern "C" MRT_EXPORT const char* CODE_MCC_GetStaticFieldName(StaticFieldInfo* fieldInfo)
    __attribute__((alias("MCC_GetStaticFieldName")));

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetInstanceFieldType(InstanceFieldInfo* fieldInfo)
    __attribute__((alias("MCC_GetInstanceFieldType")));

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetStaticFieldType(InstanceFieldInfo* fieldInfo)
    __attribute__((alias("MCC_GetStaticFieldType")));

extern "C" MRT_EXPORT U32 CODE_MCC_GetInstanceFieldModifier(InstanceFieldInfo* fieldInfo)
    __attribute__((alias("MCC_GetInstanceFieldModifier")));

extern "C" MRT_EXPORT U32 CODE_MCC_GetStaticFieldModifier(InstanceFieldInfo* fieldInfo)
    __attribute__((alias("MCC_GetStaticFieldModifier")));

extern "C" MRT_EXPORT void CODE_MCC_SetInstanceFieldValue(InstanceFieldInfo* fieldInfo, ObjRef obj, ObjRef newValue)
    __attribute__((alias("MCC_SetInstanceFieldValue")));
extern "C" MRT_EXPORT void CODE_MCC_SetStaticFieldValue(InstanceFieldInfo* fieldInfo, ObjRef value)
    __attribute__((alias("MCC_SetStaticFieldValue")));

// reflect, class/struct
// for find
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_getTypeByQualifiedName(const char* qualifiedName)
    __attribute__((alias("MCC_GetTypeByQualifiedName")));
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetTypeByMangledName(const char* mangledName)
    __attribute__((alias("MCC_GetTypeByMangledName")));
extern "C" MRT_EXPORT const char* CODE_MCC_GetTypeName(TypeInfo* ti) __attribute__((alias("MCC_GetTypeName")));
extern "C" MRT_EXPORT bool CODE_MCC_IsClass(TypeInfo* ti) __attribute__((alias("MCC_IsClass")));
extern "C" MRT_EXPORT bool CODE_MCC_IsInterface(TypeInfo* ti) __attribute__((alias("MCC_IsInterface")));
extern "C" MRT_EXPORT bool CODE_MCC_IsStruct(TypeInfo* ti) __attribute__((alias("MCC_IsStruct")));
extern "C" MRT_EXPORT bool CODE_MCC_IsPrimitive(TypeInfo* ti) __attribute__((alias("MCC_IsPrimitive")));
extern "C" MRT_EXPORT bool CODE_MCC_IsGeneric(TypeInfo* ti) __attribute__((alias("MCC_IsGeneric")));
extern "C" MRT_EXPORT bool CODE_MCC_IsReflectUnsupportedType(TypeInfo* ti)
    __attribute__((alias("MCC_IsReflectUnsupportedType")));

extern "C" MRT_EXPORT U32 CODE_MCC_GetQualifiedNameLength(TypeInfo* ti)
    __attribute__((alias("MCC_GetQualifiedNameLength")));
extern "C" MRT_EXPORT void CODE_MCC_GetQualifiedName(TypeInfo* ti, char* qualifiedName)
    __attribute__((alias("MCC_GetQualifiedName")));
extern "C" MRT_EXPORT U32 CODE_MCC_GetTypeInfoModifier(TypeInfo* ti) __attribute__((alias("MCC_GetTypeInfoModifier")));
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetSuperTypeInfo(TypeInfo* ti) __attribute__((alias("MCC_GetSuperTypeInfo")));

extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfInterface(TypeInfo* ti)
    __attribute__((alias("MCC_GetNumOfInterface")));

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetInterface(TypeInfo* ti, U32 index)
    __attribute__((alias("MCC_GetInterface")));

// InstanceMemberFunctionInfos
extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfInstanceMethodInfos(TypeInfo* cls)
    __attribute__((alias("MCC_GetNumOfInstanceMethodInfos")));
extern "C" MRT_EXPORT MethodInfo* CODE_MCC_GetInstanceMethodInfo(TypeInfo* cls, U32 index)
    __attribute__((alias("MCC_GetInstanceMethodInfo")));

// StaticMemberFunctionInfos
extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfStaticMethodInfos(TypeInfo* cls)
    __attribute__((alias("MCC_GetNumOfStaticMethodInfos")));
extern "C" MRT_EXPORT MethodInfo* CODE_MCC_GetStaticMethodInfo(TypeInfo* cls, U32 index)
    __attribute__((alias("MCC_GetStaticMethodInfo")));

// InstanceMemberVariableInfos
extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfInstanceFieldInfos(TypeInfo* cls)
    __attribute__((alias("MCC_GetNumOfInstanceFieldInfos")));
extern "C" MRT_EXPORT InstanceFieldInfo* CODE_MCC_GetInstanceFieldInfo(TypeInfo* cls, U32 index)
    __attribute__((alias("MCC_GetInstanceFieldInfo")));

// StaticMemberVariableInfos
extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfStaticFieldInfos(TypeInfo* cls)
    __attribute__((alias("MCC_GetNumOfStaticFieldInfos")));
extern "C" MRT_EXPORT StaticFieldInfo* CODE_MCC_GetStaticFieldInfo(TypeInfo* cls, U32 index)
    __attribute__((alias("MCC_GetStaticFieldInfo")));

// reflect, parameter
extern "C" MRT_EXPORT const char* CODE_MCC_GetParameterName(ParameterInfoRef parameterInfo)
    __attribute__((alias("MCC_GetParameterName")));
extern "C" MRT_EXPORT U32 CODE_MCC_GetParameterIndex(ParameterInfoRef parameterInfo)
    __attribute__((alias("MCC_GetParameterIndex")));
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetParameterType(ParameterInfoRef parameterInfo)
    __attribute__((alias("MCC_GetParameterType")));
#endif
