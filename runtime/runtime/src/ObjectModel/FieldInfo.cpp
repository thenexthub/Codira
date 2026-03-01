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


#include "FieldInfo.h"
#include "Base/Log.h"
#include "Base/Globals.h"
#include "Loader/ILoader.h"
#include "Heap/Heap.h"
#include "ObjectModel/MObject.inline.h"
#include "ObjectManager.inline.h"
namespace MapleRuntime {

U32 InstanceFieldInfo::GetModifier() const { return modifier; }

const char* InstanceFieldInfo::GetName(TypeInfo* declaringTypeInfo) const
{
    ReflectInfo* reflectInfo = declaringTypeInfo->GetReflectInfo();
    CHECK(reflectInfo != nullptr);
    U32 idx = fieldIdx;
    TypeInfo* superTi = declaringTypeInfo->GetSuperTypeInfo();
    if (superTi != nullptr) {
        idx = fieldIdx - superTi->GetFieldNum();
    }
    return reflectInfo->GetFieldName(idx);
}

inline U32 InstanceFieldInfo::GetOffset(TypeInfo* declaringTypeInfo) const
{
    CHECK(!(fieldIdx > declaringTypeInfo->GetFieldNum()));
    return declaringTypeInfo->GetFieldOffsets(fieldIdx);
}

TypeInfo* InstanceFieldInfo::GetFieldType(TypeInfo* declaringTypeInfo)
{
    CHECK(!(fieldIdx > declaringTypeInfo->GetFieldNum()));
    return declaringTypeInfo->GetFieldTypeInfo(fieldIdx);
}

void* InstanceFieldInfo::GetValue(TypeInfo* declaringTi, ObjRef instanceObj)
{
    CHECK(!(fieldIdx > declaringTi->GetFieldNum()));
    Uptr fieldAddr = reinterpret_cast<Uptr>(instanceObj) + TYPEINFO_PTR_SIZE + GetOffset(declaringTi);
    TypeInfo* fieldTi = GetFieldType(declaringTi);
    if (fieldTi->IsRef()) {
        return Heap::GetBarrier().ReadReference(instanceObj,
            instanceObj->GetRefField(GetOffset(declaringTi) + TYPEINFO_PTR_SIZE));
    } else if (fieldTi->IsStruct() || fieldTi->IsTuple()) {
        MSize size = MRT_ALIGN(fieldTi->GetInstanceSize() + TYPEINFO_PTR_SIZE, TYPEINFO_PTR_SIZE);
        MSize fieldSize = fieldTi->GetInstanceSize();
        MObject* obj = ObjectManager::NewObject(fieldTi, size);
        void* tmp = malloc(fieldSize);
        Heap::GetBarrier().ReadStruct(reinterpret_cast<MAddress>(tmp), instanceObj, fieldAddr, fieldSize);
        Heap::GetBarrier().WriteStruct(obj, reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE, fieldSize,
            reinterpret_cast<MAddress>(tmp), fieldSize);
        free(tmp);
        return obj;
    } else if (fieldTi->IsPrimitiveType()) {
        MSize size = MRT_ALIGN(fieldTi->GetInstanceSize() + TYPEINFO_PTR_SIZE, TYPEINFO_PTR_SIZE);
        MObject* obj = ObjectManager::NewObject(fieldTi, size);
        if (memcpy_s(reinterpret_cast<void*>(reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE),
                     fieldTi->GetInstanceSize(),
                     reinterpret_cast<void*>(fieldAddr),
                     fieldTi->GetInstanceSize()) != EOK) {
            LOG(RTLOG_ERROR, "GetValue memcpy_s fail");
        }
        return obj;
    } else if (fieldTi->IsVArray()) {
        // VArray is only used to store value types,
        // so we can copy the memory directly
        MSize vArraySize = fieldTi->GetFieldNum() * fieldTi->GetComponentTypeInfo()->GetInstanceSize();
        MSize size = MRT_ALIGN(vArraySize + TYPEINFO_PTR_SIZE, TYPEINFO_PTR_SIZE);
        MObject* obj = ObjectManager::NewObject(fieldTi, size);
        if (memcpy_s(reinterpret_cast<void*>(reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE), vArraySize,
                     reinterpret_cast<void*>(fieldAddr), vArraySize) != EOK) {
            LOG(RTLOG_ERROR, "GetValue memcpy_s fail");
        }
        return obj;
    } else {
        LOG(RTLOG_FATAL, "%s not to supported", fieldTi->GetName());
    }
    return nullptr;
}

void InstanceFieldInfo::SetValue(TypeInfo* declaringTypeInfo, ObjRef instanceObj, ObjRef newValue)
{
    TypeInfo* fieldTi = GetFieldType(declaringTypeInfo);
    Uptr fieldAddr = reinterpret_cast<Uptr>(instanceObj) + TYPEINFO_PTR_SIZE + GetOffset(declaringTypeInfo);
    if (fieldTi->IsRef()) {
        Heap::GetBarrier().WriteReference(instanceObj,
            instanceObj->GetRefField(TYPEINFO_PTR_SIZE + GetOffset(declaringTypeInfo)), newValue);
    } else if (fieldTi->IsStruct() || fieldTi->IsTuple()) {
        MSize fieldSize = fieldTi->GetInstanceSize();
        void* tmp = malloc(fieldSize);
        Heap::GetBarrier().ReadStruct(reinterpret_cast<MAddress>(tmp), instanceObj,
            reinterpret_cast<Uptr>(newValue) + TYPEINFO_PTR_SIZE, fieldSize);
        Heap::GetBarrier().WriteStruct(instanceObj, fieldAddr, fieldSize,
            reinterpret_cast<MAddress>(tmp), fieldSize);
        free(tmp);
    } else if (fieldTi->IsPrimitiveType()) {
        MSize size = fieldTi->GetInstanceSize();
        if (memcpy_s(reinterpret_cast<void*>(fieldAddr), size,
                     reinterpret_cast<void*>(reinterpret_cast<Uptr>(newValue) + TYPEINFO_PTR_SIZE), size) != EOK) {
            LOG(RTLOG_ERROR, "SetValue memcpy_s fail");
        }
    } else if (fieldTi->IsVArray()) {
        // VArray is only used to store value types,
        // so we can copy the memory directly
        MSize vArraySize = fieldTi->GetFieldNum() * fieldTi->GetComponentTypeInfo()->GetInstanceSize();
        if (memcpy_s(reinterpret_cast<void*>(fieldAddr), vArraySize,
                     reinterpret_cast<void*>(reinterpret_cast<Uptr>(newValue) + TYPEINFO_PTR_SIZE),
                     vArraySize) != EOK) {
            LOG(RTLOG_ERROR, "GetValue memcpy_s fail");
        }
    } else {
        LOG(RTLOG_FATAL, "%s not to supported", fieldTi->GetName());
    }
}

void* InstanceFieldInfo::GetAnnotations(TypeInfo* arrayTi)
{
    CHECK_DETAIL(arrayTi != nullptr, "arrayTi is nullptr");
    U32 size = arrayTi->GetInstanceSize();
    MSize objSize = MRT_ALIGN(size + TYPEINFO_PTR_SIZE, TYPEINFO_PTR_SIZE);
    MObject* obj = ObjectManager::NewObject(arrayTi, objSize, AllocType::RAW_POINTER_OBJECT);
    if (obj == nullptr) {
        ExceptionManager::OutOfMemory();
        return nullptr;
    }
    if (annotationMethod == 0) {
        return obj;
    }
    ArgValue values;
    uintptr_t structRet[ARRAY_STRUCT_SIZE];
    values.AddReference(reinterpret_cast<BaseObject*>(structRet));
    uintptr_t threadData = MapleRuntime::MRT_GetThreadLocalData();
#if defined(__aarch64__)
    ApplyCodiraMethodStub(values.GetData(), reinterpret_cast<void*>(values.GetStackSize()),
                           reinterpret_cast<void*>(annotationMethod), reinterpret_cast<void*>(threadData), structRet);
#else
    ApplyCodiraMethodStub(values.GetData(), values.GetStackSize(), annotationMethod, threadData);
#endif
    Heap::GetBarrier().WriteStruct(obj, reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE,
        size, reinterpret_cast<Uptr>(structRet), size);
    AllocBuffer* buffer = AllocBuffer::GetAllocBuffer();
    if (buffer != nullptr) {
        buffer->CommitRawPointerRegions();
    }
    return obj;
}

void* StaticFieldInfo::GetValue()
{
    TypeInfo* fieldTi = GetFieldType();
    if (fieldTi->IsRef()) {
        RefField<false>* refField = reinterpret_cast<RefField<false>*>(addr);
        return Heap::GetBarrier().ReadStaticRef(*refField);
    } else if (fieldTi->IsStruct() || fieldTi->IsTuple()) {
        MSize size = MRT_ALIGN(fieldTi->GetInstanceSize() + TYPEINFO_PTR_SIZE, TYPEINFO_PTR_SIZE);
        MSize fieldSize = fieldTi->GetInstanceSize();
        MObject* obj = ObjectManager::NewObject(fieldTi, size);
        Heap::GetBarrier().WriteStruct(obj, reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE,
            fieldSize, addr, fieldSize);
        return obj;
    } else if (fieldTi->IsPrimitiveType()) {
        MSize size = MRT_ALIGN(fieldTi->GetInstanceSize() + TYPEINFO_PTR_SIZE, TYPEINFO_PTR_SIZE);
        MObject* obj = ObjectManager::NewObject(fieldTi, size);
        if (memcpy_s(reinterpret_cast<void*>(reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE),
                     fieldTi->GetInstanceSize(),
                     reinterpret_cast<void*>(addr),
                     fieldTi->GetInstanceSize()) != EOK) {
            LOG(RTLOG_ERROR, "GetValue memcpy_s fail");
        }
        return obj;
    } else if (fieldTi->IsVArray()) {
        // VArray is only used to store value types,
        // so we can copy the memory directly
        MSize vArraySize = fieldTi->GetFieldNum() * fieldTi->GetComponentTypeInfo()->GetInstanceSize();
        MSize size = MRT_ALIGN(fieldTi->GetInstanceSize() + vArraySize, TYPEINFO_PTR_SIZE);
        MObject* obj = ObjectManager::NewObject(fieldTi, size);
        if (memcpy_s(reinterpret_cast<void*>(reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE), vArraySize,
                     reinterpret_cast<void*>(addr), vArraySize) != EOK) {
            LOG(RTLOG_ERROR, "GetValue memcpy_s fail");
        }
        return obj;
    } else {
        LOG(RTLOG_FATAL, "%s not to supported", fieldTi->GetName());
    }
    return nullptr;
}

void StaticFieldInfo::SetValue(ObjRef newValue)
{
    TypeInfo* fieldTi = GetFieldType();
    if (fieldTi->IsRef()) {
        RefField<>* rootField = reinterpret_cast<RefField<>*>(addr);
        Heap::GetHeap().GetBarrier().WriteStaticRef(*rootField, newValue);
    } else if (fieldTi->IsStruct() || fieldTi->IsTuple()) {
        MSize fieldSize = fieldTi->GetInstanceSize();
        Heap::GetBarrier().WriteStaticStruct(addr, fieldSize,
            reinterpret_cast<Uptr>(newValue) + TYPEINFO_PTR_SIZE, fieldSize, fieldTypeInfo->GetGCTib());
    } else if (fieldTi->IsPrimitiveType()) {
        MSize size = fieldTi->GetInstanceSize();
        if (memcpy_s(reinterpret_cast<void*>(addr), size,
                     reinterpret_cast<void*>(reinterpret_cast<Uptr>(newValue) + TYPEINFO_PTR_SIZE), size) != EOK) {
            LOG(RTLOG_ERROR, "SetValue memcpy_s fail");
        }
    } else if (fieldTi->IsVArray()) {
        // VArray is only used to store value types,
        // so we can copy the memory directly
        MSize vArraySize = fieldTi->GetFieldNum() * fieldTi->GetComponentTypeInfo()->GetInstanceSize();
        if (memcpy_s(reinterpret_cast<void*>(addr), vArraySize,
                     reinterpret_cast<void*>(reinterpret_cast<Uptr>(newValue) + TYPEINFO_PTR_SIZE),
                     vArraySize) != EOK) {
            LOG(RTLOG_ERROR, "GetValue memcpy_s fail");
        }
    } else {
        LOG(RTLOG_FATAL, "%s not to supported", fieldTi->GetName());
    }
}

void* StaticFieldInfo::GetAnnotations(TypeInfo* arrayTi)
{
    CHECK_DETAIL(arrayTi != nullptr, "arrayTi is nullptr");
    U32 size = arrayTi->GetInstanceSize();
    MSize objSize = MRT_ALIGN(size + TYPEINFO_PTR_SIZE, TYPEINFO_PTR_SIZE);
    MObject* obj = ObjectManager::NewObject(arrayTi, objSize, AllocType::RAW_POINTER_OBJECT);
    if (obj == nullptr) {
        ExceptionManager::OutOfMemory();
        return nullptr;
    }
    if (annotationMethod == 0) {
        return obj;
    }
    ArgValue values;
    uintptr_t structRet[ARRAY_STRUCT_SIZE];
    values.AddReference(reinterpret_cast<BaseObject*>(structRet));
    uintptr_t threadData = MapleRuntime::MRT_GetThreadLocalData();
#if defined(__aarch64__)
    ApplyCodiraMethodStub(values.GetData(), reinterpret_cast<void*>(values.GetStackSize()),
                           reinterpret_cast<void*>(annotationMethod), reinterpret_cast<void*>(threadData), structRet);
#else
    ApplyCodiraMethodStub(values.GetData(), values.GetStackSize(), annotationMethod, threadData);
#endif

    Heap::GetBarrier().WriteStruct(obj, reinterpret_cast<Uptr>(obj) + TYPEINFO_PTR_SIZE,
        size, reinterpret_cast<Uptr>(structRet), size);
    AllocBuffer* buffer = AllocBuffer::GetAllocBuffer();
    if (buffer != nullptr) {
        buffer->CommitRawPointerRegions();
    }
    return obj;
}
} // namespace MapleRuntime
