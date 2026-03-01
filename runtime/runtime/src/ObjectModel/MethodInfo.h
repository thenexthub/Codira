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

#ifndef MRT_METHOD_INFO_H
#define MRT_METHOD_INFO_H

#include "Common/TypeDef.h"
#include "Common/Dataref.h"
#include "ArgValue.h"
#include "ObjectModel/Flags.h"

namespace MapleRuntime {
class TypeInfo;
class GenericTypeInfo;
class ATTR_PACKED(4) ParameterInfo {
public:
    U32 GetIndex() { return idx; }
    const char* GetName() { return name.GetDataRef(); }
    void SetName(const char* paraName);
    TypeInfo* GetType() { return type; }
    void SetType(TypeInfo* ti) { type = ti; }
    void* GetAnnotations(TypeInfo* arrayTi);
    void SetGeneric() { isGeneric = 1; }
    bool IsGeneric() { return isGeneric == 1; }
private:
    DataRefOffset64<char> name;
    U32 idx;
    // The member does not exist in the IR, use the alignment bits to mark para is generic, likes "T"
    U32 isGeneric;
    TypeInfo* type;
    Uptr annotationMethod;
};

class ATTR_PACKED(4) GenericParameterInfo {
public:
    GenericTypeInfo* GetType() { return type; }
private:
    GenericTypeInfo* type;
};

struct CODERawArray {
    TypeInfo* ti;
    U64 len;
    void* data;
};

struct CODEArray {
    CODERawArray* rawPtr;
    U64 start;
    U64 length;
};

class ScopedAllocBuffer {
public:
    ScopedAllocBuffer() {}
    ~ScopedAllocBuffer();
};

class ATTR_PACKED(4) MethodInfo {
public:
    U32 GetModifier() { return modifier; }
    bool MethodEntryPointIsNull() { return entryPoint == 0; }
    const char* GetMethodName() { return name.GetDataRef(); }
    void SetMethodName(const char* methodName);
    ParameterInfo* GetActualParameterInfo(U32 index, bool isFromTypeTemplate = false);
    GenericTypeInfo* GetGenericParameterInfo(U32 index);
    void* GetAnnotations(TypeInfo* arrayTi);
    TypeInfo* GetReturnType();
    U16 GetNumOfActualParameterInfos() { return actualParametersCnt; }
    U16 GetNumOfGenericParameterInfos() { return genericParametersCnt; }
    void SetDeclaringTypeInfo(TypeInfo* declaringTypeInfo) { declaringTi = declaringTypeInfo; }
    void SetActualParameterInfos(Uptr actualParaInfosAddr);
    void SetGenericParameterInfos(Uptr genericParamInfosAddr);

    Uptr GetActualParameterInfos() { return reinterpret_cast<Uptr>(actualParameterInfos.GetDataRef()); }
    Uptr GetGenericParameterInfos() { return reinterpret_cast<Uptr>(genericParameterInfos.GetDataRef()); }

    void* ApplyCODEMethod(ObjRef instanceObj, void* genericArgs, void* actualArgs, TypeInfo* thisTi);
    bool CheckMethodActualArgs(void* genericArgsArray, void* actualArgsArray);
private:
    void* MemoryAlloc(size_t cnt, size_t size);
    void MemoryFree(void* ptr);
    void* GetEntryPoint();
    TypeInfo* GetActualTypeFromGenericType(GenericTypeInfo* genericTi, void* genericArgs);
    TypeInfo* GetActualTypeFromGenericTypeImpl(GenericTypeInfo* genericTi, void* genericArgs);
    void ApplyCODEMethodImpl(ArgValue* argValues, void* &sret, Value &ret);
    void PrepareSRet(ArgValue *argValues, void*& sret, TypeInfo* retType);
    void PrepareCODEMethodActualArgs(ArgValue* argValues, void* actualArgsArray, void *genericArgsArray);
    void PrepareCODEMethodGenericArgs(ArgValue* argValues, void *genericArgsArray);
    void AddCODEArg(ArgValue* argValues, TypeInfo* argType, ObjRef argObj);
    bool IsStaticMethod() { return static_cast<bool>(modifier & MODIFIER_STATIC); }
    bool HasSRet();
    bool HasSRetNotGeneric() { return static_cast<bool>(modifier & MODIFIER_HAS_SRET0); }
    bool HasSRetWithGeneric() { return static_cast<bool>(modifier & MODIFIER_HAS_SRET1); }
    bool HasSRetWithKnowGenericStruct() { return static_cast<bool>(modifier & MODIFIER_HAS_SRET2); }
    bool HasSRetWithUnknowGenericStruct() { return static_cast<bool>(modifier & MODIFIER_HAS_SRET3); }
    void* RetValueToAny(Value ret, void* sret, TypeInfo* retType);
    bool IsInitializer() { return CString(GetMethodName()) == "init"; }
    bool CheckGenericConstraint(GenericTypeInfo* genericTi, TypeInfo* ti, void* genericArgsArray);

    DataRefOffset64<char> name;
    U32 modifier;
    U16 actualParametersCnt;
    U16 genericParametersCnt;
    Uptr entryPoint;
    TypeInfo* returnType;
    Uptr annotationMethod;
    TypeInfo* declaringTi;
    DataRefOffset64<ParameterInfo> actualParameterInfos;
    DataRefOffset64<GenericParameterInfo> genericParameterInfos;
};
} // namespace MapleRuntime
#endif // MRT_METHOD_INFO_H
