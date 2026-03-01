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

#ifndef MRT_MAC_ALIAS_H
#define MRT_MAC_ALIAS_H

extern "C" MRT_EXPORT ObjRef CODE_MCC_OnFinalizerCreated(ObjRef ref);
__asm__(".global _CODE_MCC_OnFinalizerCreated\n\t.set _CODE_MCC_OnFinalizerCreated, _MCC_OnFinalizerCreated");
extern "C" MRT_EXPORT void CODE_MCC_WriteRefField(const ObjectPtr ref, const ObjectPtr obj, RefField<false>* field);
__asm__(".global _CODE_MCC_WriteRefField\n\t.set _CODE_MCC_WriteRefField, _MCC_WriteRefField");
extern "C" MRT_EXPORT void CODE_MCC_WriteStructField(const ObjectPtr obj, MAddress dst, size_t dstLen, MAddress src,
                                                   size_t srcLen);
__asm__(".global _CODE_MCC_WriteStructField\n\t.set _CODE_MCC_WriteStructField, _MCC_WriteStructField");
extern "C" MRT_EXPORT void CODE_MCC_WriteStaticRef(const ObjectPtr ref, RefField<false>* field);
__asm__(".global _CODE_MCC_WriteStaticRef\n\t.set _CODE_MCC_WriteStaticRef, _MCC_WriteStaticRef");
extern "C" MRT_EXPORT void CODE_MCC_WriteStaticStruct(MAddress dst, size_t dstLen, MAddress src, size_t srcLen,
                                                    const GCTib gcTib);
__asm__(".global _CODE_MCC_WriteStaticStruct\n\t.set _CODE_MCC_WriteStaticStruct, _MCC_WriteStaticStruct");

extern "C" MRT_EXPORT void CODE_MCC_AtomicWriteReference(const ObjectPtr ref, const ObjectPtr obj, RefField<true>* field,
                                                       MemoryOrder order);
__asm__(".global _CODE_MCC_AtomicWriteReference\n\t.set _CODE_MCC_AtomicWriteReference, _MCC_AtomicWriteReference");

extern "C" MRT_EXPORT ObjectPtr CODE_MCC_AtomicReadReference(const ObjectPtr ref, const ObjectPtr obj,
                                                           RefField<true>* field, MemoryOrder order);
__asm__(".global _CODE_MCC_AtomicReadReference\n\t.set _CODE_MCC_AtomicReadReference, _MCC_AtomicReadReference");

extern "C" MRT_EXPORT ObjectPtr CODE_MCC_AtomicSwapReference(const ObjectPtr ref, const ObjectPtr obj,
                                                           RefField<true>* field, MemoryOrder order);
__asm__(".global _CODE_MCC_AtomicSwapReference\n\t.set _CODE_MCC_AtomicSwapReference, _MCC_AtomicSwapReference");

extern "C" MRT_EXPORT bool CODE_MCC_AtomicCompareAndSwapReference(const ObjectPtr oldRef, const ObjectPtr newRef,
                                                                const ObjectPtr obj, RefField<true>* field,
                                                                MemoryOrder succOrder, MemoryOrder failOrder);
__asm__(
    ".global _CODE_MCC_AtomicCompareAndSwapReference\n\t.set _CODE_MCC_AtomicCompareAndSwapReference,"
    "_MCC_AtomicCompareSwapReference");
extern "C" MRT_EXPORT void CODE_MCC_InvokeGCImpl(bool sync);
__asm__(".global _CODE_MCC_InvokeGCImpl\n\t.set _CODE_MCC_InvokeGCImpl, _MCC_InvokeGCImpl");
extern "C" MRT_EXPORT ssize_t CODE_MCC_GetRealHeapSize();
__asm__(
    ".global _CODE_MCC_GetRealHeapSize\n\t.set _CODE_MCC_GetRealHeapSize, "
    "_MCC_GetRealHeapSize");
extern "C" MRT_EXPORT size_t CODE_MCC_GetAllocatedHeapSize();
__asm__(".global _CODE_MCC_GetAllocatedHeapSize\n\t.set _CODE_MCC_GetAllocatedHeapSize, _MCC_GetAllocatedHeapSize");
extern "C" MRT_EXPORT size_t CODE_MCC_GetMaxHeapSize();
__asm__(".global _CODE_MCC_GetMaxHeapSize\n\t.set _CODE_MCC_GetMaxHeapSize, _MCC_GetMaxHeapSize");
extern "C" MRT_EXPORT size_t CODE_MCC_GetCODEThreadNumber();
__asm__(".global _CODE_MCC_GetCODEThreadNumber\n\t.set _CODE_MCC_GetCODEThreadNumber, _MCC_GetCODEThreadNumber");
extern "C" MRT_EXPORT size_t CODE_MCC_GetBlockingCODEThreadNumber();
__asm__(
    ".global _CODE_MCC_GetBlockingCODEThreadNumber\n\t.set _CODE_MCC_GetBlockingCODEThreadNumber, "
    "_MCC_GetBlockingCODEThreadNumber");
extern "C" MRT_EXPORT size_t CODE_MCC_GetNativeThreadNumber();
__asm__(".global _CODE_MCC_GetNativeThreadNumber\n\t.set _CODE_MCC_GetNativeThreadNumber, _MCC_GetNativeThreadNumber");
extern "C" MRT_EXPORT size_t CODE_MCC_GetGCCount();
__asm__(".global _CODE_MCC_GetGCCount\n\t.set _CODE_MCC_GetGCCount, _MCC_GetGCCount");
extern "C" MRT_EXPORT uint64_t CODE_MCC_GetGCTimeUs();
__asm__(".global _CODE_MCC_GetGCTimeUs\n\t.set _CODE_MCC_GetGCTimeUs, _MCC_GetGCTimeUs");
extern "C" MRT_EXPORT size_t CODE_MCC_GetGCFreedSize();
__asm__(".global _CODE_MCC_GetGCFreedSize\n\t.set _CODE_MCC_GetGCFreedSize, _MCC_GetGCFreedSize");
extern "C" MRT_EXPORT size_t CODE_MCC_StartCpuProfiling();
__asm__(".global _CODE_MCC_StartCpuProfiling\n\t.set _CODE_MCC_StartCpuProfiling, _MCC_StartCpuProfiling");
extern "C" MRT_EXPORT size_t CODE_MCC_StopCpuProfiling(int fd);
__asm__(".global _CODE_MCC_StopCpuProfiling\n\t.set _CODE_MCC_StopCpuProfiling, _MCC_StopCpuProfiling");
extern "C" MRT_EXPORT void CODE_MCC_SetGCThreshold(uint64_t GCThreshold);
__asm__(".global _CODE_MCC_SetGCThreshold\n\t.set _CODE_MCC_SetGCThreshold, _MCC_SetGCThreshold");
extern "C" MRT_EXPORT void* CODE_MCC_PostThrowException(ExceptionWrapper* mExceptionWrapper);
__asm__(".global _CODE_MCC_PostThrowException\n\t.set _CODE_MCC_PostThrowException, _MCC_PostThrowException");
extern "C" MRT_EXPORT void CODE_MCC_ThrowException(ExceptionRef exception);
__asm__(".global _CODE_MCC_ThrowException\n\t.set _CODE_MCC_ThrowException, _MCC_ThrowException");
extern "C" MRT_EXPORT void CODE_MCC_RegisterImplicitExceptionRaisers(void* raiser);
__asm__(
    ".global _CODE_MCC_RegisterImplicitExceptionRaisers\n\t.set _CODE_MCC_RegisterImplicitExceptionRaisers, "
    "_MCC_RegisterImplicitExceptionRaisers");
extern "C" MRT_EXPORT void* CODE_MCC_GetExceptionWrapper();
__asm__(".global _CODE_MCC_GetExceptionWrapper\n\t.set _CODE_MCC_GetExceptionWrapper, _MCC_GetExceptionWrapper");
extern "C" MRT_EXPORT uint32_t CODE_MCC_GetExceptionTypeID();
__asm__(".global _CODE_MCC_GetExceptionTypeID\n\t.set _CODE_MCC_GetExceptionTypeID, _MCC_GetExceptionTypeID");
extern "C" MRT_EXPORT void CODE_MCC_EndCatch();
__asm__(".global _CODE_MCC_EndCatch\n\t.set _CODE_MCC_EndCatch, _MCC_EndCatch");
extern "C" MRT_EXPORT ArrayRef CODE_MCC_FillInStackTraceImpl(const TypeInfo* arrayInfo, const ArrayRef exceptionMessage);
__asm__(".global _CODE_MCC_FillInStackTraceImpl\n\t.set _CODE_MCC_FillInStackTraceImpl, _MCC_FillInStackTraceImpl");
extern "C" MRT_EXPORT void CODE_MCC_ReleaseRawData(ArrayRef array, void* rawPtr);
__asm__(".global _CODE_MCC_ReleaseRawData\n\t.set _CODE_MCC_ReleaseRawData, _MCC_ReleaseRawData");
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetObjClass(const ObjectPtr obj);
__asm__(".global _CODE_MCC_GetObjClass\n\t.set _CODE_MCC_GetObjClass, _MCC_GetObjClass");

extern "C" MRT_EXPORT TypeTemplate* CODE_MCC_GetTypeTemplate(char* name);
__asm__(".global _CODE_MCC_MCC_GetTypeTemplate\n\t.set _CODE_MCC_GetTypeTemplate, _MCC_GetTypeTemplate");

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetOrCreateTypeInfoForReflect(TypeTemplate* tt, void* args);
__asm__(".global _CODE_MCC_GetOrCreateTypeInfoForReflect\n\t.set _CODE_MCC_GetOrCreateTypeInfoForReflect, "
    "_MCC_GetOrCreateTypeInfoForReflect");

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetTypeForAny(const ObjectPtr obj);
__asm__(".global _CODE_MCC_GetTypeForAny\n\t.set _CODE_MCC_GetTypeForAny, _MCC_GetTypeForAny");

extern "C" MRT_EXPORT bool MCC_IsWrapperClassForAutoEnv(const TypeInfo* ti);
__asm__(".global _CODE_MCC_IsWrapperClassForAutoEnv\n\t.set _CODE_MCC_IsWrapperClassForAutoEnv, "
    "_MCC_IsWrapperClassForAutoEnv");

extern "C" MRT_EXPORT void* CODE_MCC_LoadPackage(const char* path);
__asm__(".global _CODE_MCC_LoadPackage\n\t.set _CODE_MCC_LoadPackage, _MCC_LoadPackage");

extern "C" MRT_EXPORT PackageInfo* CODE_MCC_GetPackageByQualifiedName(const char* packageName);
__asm__(".global _CODE_MCC_GetPackageByQualifiedName\n\t.set _CODE_MCC_GetPackageByQualifiedName, "
    "_MCC_GetPackageByQualifiedName");

extern "C" MRT_EXPORT const char* CODE_MCC_GetPackageVersion(PackageInfo* packageInfo);
__asm__(".global _CODE_MCC_GetPackageVersion\n\t.set _CODE_MCC_GetPackageVersion, _MCC_GetPackageVersion");

extern "C" MRT_EXPORT void* CODE_MCC_GetSubPackages(PackageInfo* packageInfo);
__asm__(".global _CODE_MCC_GetSubPackages\n\t.set _CODE_MCC_GetSubPackages, _MCC_GetSubPackages");

extern "C" MRT_EXPORT char* CODE_MCC_GetPackageName(PackageInfo* packageInfo);
__asm__(".global _CODE_MCC_GetPackageName\n\t.set _CODE_MCC_GetPackageName, _MCC_GetPackageName");
extern "C" MRT_EXPORT U32 CODE_MCC_GetPackageNumOfTypeInfos(PackageInfo* packageInfo);
__asm__(".global _CODE_MCC_GetPackageNumOfTypeInfos\n\t.set _CODE_MCC_GetPackageNumOfTypeInfos, _MCC_GetNumOfTypeInfos");
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetPackageTypeInfo(PackageInfo* packageInfo, U32 index);
__asm__(".global _CODE_MCC_GetPackageTypeInfo\n\t.set _CODE_MCC_GetPackageTypeInfo, _MCC_GetPackageTypeInfo");

extern "C" MRT_EXPORT U32 CODE_MCC_GetPackageNumOfGlobalMethodInfos(PackageInfo* packageInfo);
__asm__(
    ".global _CODE_MCC_GetPackageNumOfGlobalMethodInfos\n\t.set _CODE_MCC_GetPackageNumOfGlobalMethodInfos, "
    "_MCC_GetPackageNumOfGlobalMethodInfos");
extern "C" MRT_EXPORT MethodInfo* CODE_MCC_GetPackageGlobalMethodInfo(PackageInfo* packageInfo, U32 index);
__asm__(
    ".global _CODE_MCC_GetPackageGlobalMethodInfo\n\t.set _CODE_MCC_GetPackageGlobalMethodInfo, "
    "_MCC_GetPackageGlobalMethodInfo");

extern "C" MRT_EXPORT U32 CODE_MCC_GetPackageNumOfGlobalFieldInfos(PackageInfo* packageInfo);
__asm__(
    ".global _CODE_MCC_GetPackageNumOfGlobalFieldInfos\n\t.set _CODE_MCC_GetPackageNumOfGlobalFieldInfos, "
    "_MCC_GetPackageNumOfGlobalFieldInfos");
extern "C" MRT_EXPORT InstanceFieldInfo* CODE_MCC_GetPackageGlobalFieldInfo(PackageInfo* packageInfo, U32 index);
__asm__(
    ".global _CODE_MCC_GetPackageGlobalFieldInfo\n\t.set _CODE_MCC_GetPackageGlobalFieldInfo, "
    "_MCC_GetPackageGlobalFieldInfo");

// for reflect, method
extern "C" MRT_EXPORT const char* CODE_MCC_GetMethodName(MethodInfo* methodInfo);
__asm__(".global _CODE_MCC_GetMethodName\n\t.set _CODE_MCC_GetMethodName, _MCC_GetMethodName");
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetMethodReturnType(MethodInfo* methodInfo);
__asm__(".global _CODE_MCC_GetMethodReturnType\n\t.set _CODE_MCC_GetMethodReturnType, _MCC_GetMethodReturnType");
extern "C" MRT_EXPORT U32 CODE_MCC_GetMethodModifier(MethodInfo* methodInfo);
__asm__(".global _CODE_MCC_GetMethodModifier\n\t.set _CODE_MCC_GetMethodModifier, _MCC_GetMethodModifier");
extern "C" MRT_EXPORT U32 CODE_MCC_MethodEntryPointIsNull(MethodInfo* methodInfo);
__asm__(".global _CODE_MCC_MethodEntryPointIsNull\n\t.set _CODE_MCC_MethodEntryPointIsNull, _MCC_MethodEntryPointIsNull");

extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfActualParameters(MethodInfo* methodInfo);
__asm__(".global _CODE_MCC_GetNumOfActualParameters\n\t.set _CODE_MCC_GetNumOfActualParameters, "
    "_MCC_GetNumOfActualParameters");

extern "C" MRT_EXPORT ParameterInfoRef CODE_MCC_GetActualParameterInfo(MethodInfo* methodInfo, U32 index);
__asm__(".global _CODE_MCC_GetActualParameterInfo\n\t.set _CODE_MCC_GetActualParameterInfo, _MCC_GetActualParameterInfo");

extern "C" MRT_EXPORT GenericTypeInfo* CODE_MCC_GetGenericParameterInfo(MethodInfo* methodInfo, U32 index);
__asm__(".global _CODE_MCC_GetGenericParameterInfo\n\t.set _CODE_MCC_GetGenericParameterInfo, "
    "_MCC_GetGenericParameterInfo");

extern "C" MRT_EXPORT GenericTypeInfo* CODE_MCC_CheckMethodActualArgs(MethodInfo* methodInfo,
    void* genericArgs, void* args);
__asm__(".global _CODE_MCC_CheckMethodActualArgs\n\t.set _CODE_MCC_CheckMethodActualArgs, "
    "_MCC_CheckMethodActualArgs");

// for reflect, field
extern "C" MRT_EXPORT const char* CODE_MCC_GetInstanceFieldName(InstanceFieldInfo* fieldInfo);
__asm__(".global _CODE_MCC_GetInstanceFieldName\n\t.set _CODE_MCC_GetInstanceFieldName, _MCC_GetInstanceFieldName");

extern "C" MRT_EXPORT const char* CODE_MCC_GetStaticFieldName(StaticFieldInfo fieldInfo);
__asm__(".global _CODE_MCC_GetStaticFieldName\n\t.set _CODE_MCC_GetStaticFieldName, _MCC_GetStaticFieldName");

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetInstanceFieldType(InstanceFieldInfo* fieldInfo);
__asm__(".global _CODE_MCC_GetInstanceFieldType\n\t.set _CODE_MCC_GetInstanceFieldType, _MCC_GetInstanceFieldType");

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetStaticFieldType(InstanceFieldInfo* fieldInfo);
__asm__(".global _CODE_MCC_GetStaticFieldType\n\t.set _CODE_MCC_GetStaticFieldType, _MCC_GetStaticFieldType");

extern "C" MRT_EXPORT U32 CODE_MCC_GetInstanceFieldModifier(InstanceFieldInfo* fieldInfo);
__asm__(".global _CODE_MCC_GetInstanceFieldModifier\n\t.set _CODE_MCC_GetInstanceFieldModifier, "
    "_MCC_GetInstanceFieldModifier");

extern "C" MRT_EXPORT U32 CODE_MCC_GetStaticFieldModifier(InstanceFieldInfo* fieldInfo);
__asm__(".global _CODE_MCC_GetStaticFieldModifier\n\t.set _CODE_MCC_GetStaticFieldModifier, _MCC_GetStaticFieldModifier");

extern "C" MRT_EXPORT void CODE_MCC_SetInstanceFieldValue(InstanceFieldInfo* fieldInfo, ObjRef obj, ObjRef newValue);
__asm__(".global _CODE_MCC_SetInstanceFieldValue\n\t.set _CODE_MCC_SetInstanceFieldValue, _MCC_SetInstanceFieldValue");
extern "C" MRT_EXPORT void CODE_MCC_SetStaticFieldValue(InstanceFieldInfo* fieldInfo, ObjRef value);
__asm__(".global _CODE_MCC_SetStaticFieldValue\n\t.set _CODE_MCC_SetStaticFieldValue, _MCC_SetStaticFieldValue");

// reflect, parameter
extern "C" MRT_EXPORT const char* CODE_MCC_GetParameterName(ParameterInfoRef parameterInfo);
__asm__(".global _CODE_MCC_GetParameterName\n\t.set _CODE_MCC_GetParameterName, _MCC_GetParameterName");
extern "C" MRT_EXPORT U32 CODE_MCC_GetParameterIndex(ParameterInfoRef parameterInfo);
__asm__(".global _CODE_MCC_GetParameterIndex\n\t.set _CODE_MCC_GetParameterIndex, _MCC_GetParameterIndex");
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetParameterType(ParameterInfoRef parameterInfo);
__asm__(".global _CODE_MCC_GetParameterType\n\t.set _CODE_MCC_GetParameterType, _MCC_GetParameterType");

// for of<a: Any>
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetTypeForAny(const ObjectPtr obj);
__asm__(".global _CODE_MCC_GetTypeForAny\n\t.set _CODE_MCC_GetTypeForAny, _MCC_GetTypeForAny");

// reflect, class/struct
// for find
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_getTypeByQualifiedName(const char* qualifiedName);
__asm__(".global _CODE_MCC_getTypeByQualifiedName\n\t.set _CODE_MCC_getTypeByQualifiedName, _MCC_GetTypeByQualifiedName");

extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetTypeByMangledName(const char* mangledName);
__asm__(".global _CODE_MCC_GetTypeByMangledName\n\t.set _CODE_MCC_GetTypeByMangledName, _MCC_GetTypeByMangledName");

extern "C" MRT_EXPORT const char* CODE_MCC_GetTypeName(TypeInfo* ti);
__asm__(".global _CODE_MCC_GetTypeName\n\t.set _CODE_MCC_GetTypeName, _MCC_GetTypeName");

extern "C" MRT_EXPORT bool CODE_MCC_IsClass(TypeInfo* ti);
__asm__(".global _CODE_MCC_IsClass\n\t.set _CODE_MCC_IsClass, _MCC_IsClass");

extern "C" MRT_EXPORT bool CODE_MCC_IsInterface(TypeInfo* ti);
__asm__(".global _CODE_MCC_IsInterface\n\t.set _CODE_MCC_IsInterface, _MCC_IsInterface");

extern "C" MRT_EXPORT bool CODE_MCC_IsStruct(TypeInfo* ti);
__asm__(".global _CODE_MCC_IsStruct\n\t.set _CODE_MCC_IsStruct, _MCC_IsStruct");

extern "C" MRT_EXPORT bool CODE_MCC_IsPrimitive(TypeInfo* ti);
__asm__(".global _CODE_MCC_IsPrimitive\n\t.set _CODE_MCC_IsPrimitive, _MCC_IsPrimitive");

extern "C" MRT_EXPORT bool CODE_MCC_IsGeneric(TypeInfo* ti);
__asm__(".global _CODE_MCC_IsGeneric\n\t.set _CODE_MCC_IsGeneric, _MCC_IsGeneric");

extern "C" MRT_EXPORT bool CODE_MCC_IsReflectUnsupportedType(TypeInfo* ti);
__asm__(".global _CODE_MCC_IsReflectUnsupportedType\n\t.set _CODE_MCC_IsReflectUnsupportedType, "
    "_MCC_IsReflectUnsupportedType");

extern "C" MRT_EXPORT U32 CODE_MCC_GetQualifiedNameLength(TypeInfo* ti);
__asm__(".global _CODE_MCC_GetQualifiedNameLength\n\t.set _CODE_MCC_GetQualifiedNameLength, _MCC_GetQualifiedNameLength");
extern "C" MRT_EXPORT void CODE_MCC_GetQualifiedName(TypeInfo* ti, char* qualifiedName);
__asm__(".global _CODE_MCC_GetQualifiedName\n\t.set _CODE_MCC_GetQualifiedName, _MCC_GetQualifiedName");
extern "C" MRT_EXPORT U32 CODE_MCC_GetTypeInfoModifier(TypeInfo* ti);
__asm__(".global _CODE_MCC_GetTypeInfoModifier\n\t.set _CODE_MCC_GetTypeInfoModifier, _MCC_GetTypeInfoModifier");
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetSuperTypeInfo(TypeInfo* ti);
__asm__(".global _CODE_MCC_GetSuperTypeInfo\n\t.set _CODE_MCC_GetSuperTypeInfo, _MCC_GetSuperTypeInfo");

extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfInterface(TypeInfo* ti);
__asm__(".global _CODE_MCC_GetNumOfInterface\n\t.set _CODE_MCC_GetNumOfInterface, _MCC_GetNumOfInterface");
extern "C" MRT_EXPORT TypeInfo* CODE_MCC_GetInterface(TypeInfo* ti, U32 index);
__asm__(".global _CODE_MCC_GetInterface\n\t.set _CODE_MCC_GetInterface, _MCC_GetInterface");

// InstanceMemberFunctionInfos
extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfInstanceMethodInfos(TypeInfo* ti);
__asm__(
    ".global _CODE_MCC_GetNumOfInstanceMethodInfos\n\t.set "
    "_CODE_MCC_GetNumOfInstanceMethodInfos, _MCC_GetNumOfInstanceMethodInfos");
extern "C" MRT_EXPORT MethodInfo* CODE_MCC_GetInstanceMethodInfo(TypeInfo* ti, U32 index);
__asm__(
    ".global _CODE_MCC_GetInstanceMethodInfo\n\t.set _CODE_MCC_GetInstanceMethodInfo, "
    "_MCC_GetInstanceMethodInfo");

// StaticMemberFunctionInfos
extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfStaticMethodInfos(TypeInfo* ti);
__asm__(
    ".global _CODE_MCC_GetNumOfStaticMethodInfos\n\t.set _CODE_MCC_GetNumOfStaticMethodInfos, "
    "_MCC_GetNumOfStaticMethodInfos");
extern "C" MRT_EXPORT MethodInfo* CODE_MCC_GetStaticMethodInfo(TypeInfo* ti, U32 index);
__asm__(
    ".global _CODE_MCC_GetStaticMethodInfo\n\t.set _CODE_MCC_GetStaticMethodInfo, "
    "_MCC_GetStaticMethodInfo");

// InstanceMemberVariableInfos
extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfInstanceFieldInfos(TypeInfo* ti);
__asm__(
    ".global _CODE_MCC_GetNumOfInstanceFieldInfos\n\t.set "
    "_CODE_MCC_GetNumOfInstanceFieldInfos, _MCC_GetNumOfInstanceFieldInfos");
extern "C" MRT_EXPORT InstanceFieldInfo* CODE_MCC_GetInstanceFieldInfo(TypeInfo* ti, U32 index);
__asm__(
    ".global _CODE_MCC_GetInstanceFieldInfo\n\t.set _CODE_MCC_GetInstanceFieldInfo, "
    "_MCC_GetInstanceFieldInfo");

// StaticMemberVariableInfos
extern "C" MRT_EXPORT U32 CODE_MCC_GetNumOfStaticFieldInfos(TypeInfo* ti);
__asm__(
    ".global _CODE_MCC_GetNumOfStaticFieldInfos\n\t.set _CODE_MCC_GetNumOfStaticFieldInfos, "
    "_MCC_GetNumOfStaticFieldInfos");
extern "C" MRT_EXPORT StaticFieldInfo* CODE_MCC_GetStaticFieldInfo(TypeInfo* ti, U32 index);
__asm__(
    ".global _CODE_MCC_GetStaticFieldInfo\n\t.set _CODE_MCC_GetStaticFieldInfo, "
    "_MCC_GetStaticFieldInfo");

#endif
