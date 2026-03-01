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

#include "ets_coroutine.h"
#include "ets_vm.h"
#include "intrinsics.h"
#include "mem/vm_handle.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "types/ets_array.h"
#include "types/ets_box_primitive.h"
#include "types/ets_class.h"
#include "types/ets_method.h"
#include "types/ets_primitives.h"
#include "types/ets_type.h"
#include "types/ets_type_comptime_traits.h"
#include "types/ets_typeapi.h"
#include "types/ets_typeapi_field.h"

namespace ark::ets::intrinsics {

void ValueAPISetFieldObject(EtsObject *obj, EtsLong i, EtsObject *val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, obj->GetCoreType());
    VMHandle<EtsObject> valHandle(coroutine, val->GetCoreType());
    ASSERT(objHandle.GetPtr() != nullptr);
    auto typeClass = objHandle->GetClass();
    auto fieldObject = typeClass->GetFieldByIndex(i);
    objHandle->SetFieldObject(fieldObject, valHandle.GetPtr());
}

template <typename T>
void SetFieldValue(EtsObject *obj, EtsLong i, T val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, obj->GetCoreType());
    ASSERT(objHandle.GetPtr() != nullptr);
    auto typeClass = objHandle->GetClass();
    auto fieldObject = typeClass->GetFieldByIndex(i);
    if (fieldObject->GetType()->IsBoxed()) {
        auto *boxedVal = EtsBoxPrimitive<T>::Create(coroutine, val);
        objHandle->SetFieldObject(fieldObject, boxedVal);
        return;
    }
    objHandle->SetFieldPrimitive<T>(fieldObject, val);
}

void ValueAPISetFieldBoolean(EtsObject *obj, EtsLong i, EtsBoolean val)
{
    SetFieldValue(obj, i, val);
}

void ValueAPISetFieldByte(EtsObject *obj, EtsLong i, EtsByte val)
{
    SetFieldValue(obj, i, val);
}

void ValueAPISetFieldShort(EtsObject *obj, EtsLong i, EtsShort val)
{
    SetFieldValue(obj, i, val);
}

void ValueAPISetFieldChar(EtsObject *obj, EtsLong i, EtsChar val)
{
    SetFieldValue(obj, i, val);
}

void ValueAPISetFieldInt(EtsObject *obj, EtsLong i, EtsInt val)
{
    SetFieldValue(obj, i, val);
}

void ValueAPISetFieldLong(EtsObject *obj, EtsLong i, EtsLong val)
{
    SetFieldValue(obj, i, val);
}

void ValueAPISetFieldFloat(EtsObject *obj, EtsLong i, EtsFloat val)
{
    SetFieldValue(obj, i, val);
}

void ValueAPISetFieldDouble(EtsObject *obj, EtsLong i, EtsDouble val)
{
    SetFieldValue(obj, i, val);
}

void ValueAPISetFieldByNameObject(EtsObject *obj, EtsString *name, EtsObject *val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, obj->GetCoreType());
    VMHandle<EtsString> nameHandle(coroutine, name->GetCoreType());
    VMHandle<EtsObject> valHandle(coroutine, val->GetCoreType());
    ASSERT(objHandle.GetPtr() != nullptr);
    ASSERT(nameHandle.GetPtr() != nullptr);
    auto typeClass = objHandle->GetClass();
    auto fieldObject = ManglingUtils::GetFieldIDByDisplayName(typeClass, nameHandle->GetMutf8());
    objHandle->SetFieldObject(fieldObject, valHandle.GetPtr());
}

template <typename T>
void SetFieldByNameValue(EtsObject *obj, EtsString *name, T val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, obj->GetCoreType());
    VMHandle<EtsString> nameHandle(coroutine, name->GetCoreType());
    ASSERT(objHandle.GetPtr() != nullptr);
    ASSERT(nameHandle.GetPtr() != nullptr);
    auto typeClass = objHandle->GetClass();
    auto fieldObject = ManglingUtils::GetFieldIDByDisplayName(typeClass, nameHandle->GetMutf8());
    if (fieldObject->GetType()->IsBoxed()) {
        auto *boxedVal = EtsBoxPrimitive<T>::Create(coroutine, val);
        objHandle->SetFieldObject(fieldObject, boxedVal);
        return;
    }
    objHandle->SetFieldPrimitive<T>(fieldObject, val);
}

void ValueAPISetFieldByNameBoolean(EtsObject *obj, EtsString *name, EtsBoolean val)
{
    SetFieldByNameValue(obj, name, val);
}

void ValueAPISetFieldByNameByte(EtsObject *obj, EtsString *name, EtsByte val)
{
    SetFieldByNameValue(obj, name, val);
}

void ValueAPISetFieldByNameShort(EtsObject *obj, EtsString *name, EtsShort val)
{
    SetFieldByNameValue(obj, name, val);
}

void ValueAPISetFieldByNameChar(EtsObject *obj, EtsString *name, EtsChar val)
{
    SetFieldByNameValue(obj, name, val);
}

void ValueAPISetFieldByNameInt(EtsObject *obj, EtsString *name, EtsInt val)
{
    SetFieldByNameValue(obj, name, val);
}

void ValueAPISetFieldByNameLong(EtsObject *obj, EtsString *name, EtsLong val)
{
    SetFieldByNameValue(obj, name, val);
}

void ValueAPISetFieldByNameFloat(EtsObject *obj, EtsString *name, EtsFloat val)
{
    SetFieldByNameValue(obj, name, val);
}

void ValueAPISetFieldByNameDouble(EtsObject *obj, EtsString *name, EtsDouble val)
{
    SetFieldByNameValue(obj, name, val);
}

EtsObject *ValueAPIGetFieldObject(EtsObject *obj, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, obj->GetCoreType());
    ASSERT(objHandle.GetPtr() != nullptr);
    auto typeClass = objHandle->GetClass();
    auto fieldObject = typeClass->GetFieldByIndex(i);
    return objHandle->GetFieldObject(fieldObject);
}

template <typename T>
T GetFieldValue(EtsObject *obj, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, obj->GetCoreType());
    ASSERT(objHandle.GetPtr() != nullptr);
    auto typeClass = objHandle->GetClass();
    auto fieldObject = typeClass->GetFieldByIndex(i);
    if (fieldObject->GetType()->IsBoxed()) {
        return EtsBoxPrimitive<T>::FromCoreType(objHandle->GetFieldObject(fieldObject))->GetValue();
    }
    return objHandle->GetFieldPrimitive<T>(fieldObject);
}

EtsBoolean ValueAPIGetFieldBoolean(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsBoolean>(obj, i);
}

EtsByte ValueAPIGetFieldByte(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsByte>(obj, i);
}

EtsShort ValueAPIGetFieldShort(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsShort>(obj, i);
}

EtsChar ValueAPIGetFieldChar(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsChar>(obj, i);
}

EtsInt ValueAPIGetFieldInt(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsInt>(obj, i);
}

EtsLong ValueAPIGetFieldLong(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsLong>(obj, i);
}

EtsFloat ValueAPIGetFieldFloat(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsFloat>(obj, i);
}

EtsDouble ValueAPIGetFieldDouble(EtsObject *obj, EtsLong i)
{
    return GetFieldValue<EtsDouble>(obj, i);
}

EtsObject *ValueAPIGetFieldByNameObject(EtsObject *obj, EtsString *name)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, obj->GetCoreType());
    ASSERT(objHandle.GetPtr() != nullptr);
    auto typeClass = objHandle->GetClass();
    auto fieldObject = ManglingUtils::GetFieldIDByDisplayName(typeClass, name->GetMutf8());
    return objHandle->GetFieldObject(fieldObject);
}

template <typename T>
T GetFieldByNameValue(EtsObject *obj, EtsString *name)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObject> objHandle(coroutine, obj->GetCoreType());
    ASSERT(objHandle.GetPtr() != nullptr);
    auto typeClass = objHandle->GetClass();
    auto fieldObject = ManglingUtils::GetFieldIDByDisplayName(typeClass, name->GetMutf8());
    if (fieldObject->GetType()->IsBoxed()) {
        return EtsBoxPrimitive<T>::FromCoreType(objHandle->GetFieldObject(fieldObject))->GetValue();
    }
    return objHandle->GetFieldPrimitive<T>(fieldObject);
}

EtsBoolean ValueAPIGetFieldByNameBoolean(EtsObject *obj, EtsString *name)
{
    return GetFieldByNameValue<EtsBoolean>(obj, name);
}

EtsByte ValueAPIGetFieldByNameByte(EtsObject *obj, EtsString *name)
{
    return GetFieldByNameValue<EtsByte>(obj, name);
}

EtsShort ValueAPIGetFieldByNameShort(EtsObject *obj, EtsString *name)
{
    return GetFieldByNameValue<EtsShort>(obj, name);
}

EtsChar ValueAPIGetFieldByNameChar(EtsObject *obj, EtsString *name)
{
    return GetFieldByNameValue<EtsChar>(obj, name);
}

EtsInt ValueAPIGetFieldByNameInt(EtsObject *obj, EtsString *name)
{
    return GetFieldByNameValue<EtsInt>(obj, name);
}

EtsLong ValueAPIGetFieldByNameLong(EtsObject *obj, EtsString *name)
{
    return GetFieldByNameValue<EtsLong>(obj, name);
}

EtsFloat ValueAPIGetFieldByNameFloat(EtsObject *obj, EtsString *name)
{
    return GetFieldByNameValue<EtsFloat>(obj, name);
}

EtsDouble ValueAPIGetFieldByNameDouble(EtsObject *obj, EtsString *name)
{
    return GetFieldByNameValue<EtsDouble>(obj, name);
}

EtsLong ValueAPIGetArrayLength(EtsObject *obj)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsArray> arrHandle(coroutine, obj->GetCoreType());
    ASSERT(arrHandle.GetPtr() != nullptr);
    return arrHandle->GetLength();
}

void ValueAPISetElementObject(EtsObject *obj, EtsLong i, EtsObject *val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObjectArray> arrHandle(coroutine, obj->GetCoreType());
    VMHandle<EtsObject> valHandle(coroutine, val->GetCoreType());
    ASSERT(arrHandle.GetPtr() != nullptr);
    arrHandle->Set(i, valHandle.GetPtr());
}

template <typename P, typename T>
void SetElement(EtsObject *obj, EtsLong i, T val)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    auto typeClass = obj->GetClass();
    if (!typeClass->GetComponentType()->IsBoxed()) {
        VMHandle<P> arrHandle(coroutine, obj->GetCoreType());
        ASSERT(arrHandle.GetPtr() != nullptr);
        arrHandle->Set(i, val);
    } else {
        VMHandle<EtsObjectArray> arrHandle(coroutine, obj->GetCoreType());
        ASSERT(arrHandle.GetPtr() != nullptr);
        // Don't inline boxedVal.
        // In case it is inlined, the handle may be dereferenced before
        // call Create which leads to invalid raw pointer to the managed object.
        auto *boxedVal = EtsBoxPrimitive<T>::Create(coroutine, val);
        arrHandle->Set(i, boxedVal);
    }
}

void ValueAPISetElementBoolean(EtsObject *obj, EtsLong i, EtsBoolean val)
{
    SetElement<EtsBooleanArray>(obj, i, val);
}

void ValueAPISetElementByte(EtsObject *obj, EtsLong i, EtsByte val)
{
    SetElement<EtsByteArray>(obj, i, val);
}

void ValueAPISetElementShort(EtsObject *obj, EtsLong i, EtsShort val)
{
    SetElement<EtsShortArray>(obj, i, val);
}

void ValueAPISetElementChar(EtsObject *obj, EtsLong i, EtsChar val)
{
    SetElement<EtsCharArray>(obj, i, val);
}

void ValueAPISetElementInt(EtsObject *obj, EtsLong i, EtsInt val)
{
    SetElement<EtsIntArray>(obj, i, val);
}

void ValueAPISetElementLong(EtsObject *obj, EtsLong i, EtsLong val)
{
    SetElement<EtsLongArray>(obj, i, val);
}

void ValueAPISetElementFloat(EtsObject *obj, EtsLong i, EtsFloat val)
{
    SetElement<EtsFloatArray>(obj, i, val);
}

void ValueAPISetElementDouble(EtsObject *obj, EtsLong i, EtsDouble val)
{
    SetElement<EtsDoubleArray>(obj, i, val);
}

EtsObject *ValueAPIGetElementObject(EtsObject *obj, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    VMHandle<EtsObjectArray> arrHandle(coroutine, obj->GetCoreType());
    ASSERT(arrHandle.GetPtr() != nullptr);
    return arrHandle->Get(i);
}

template <typename P>
typename P::ValueType GetElement(EtsObject *obj, EtsLong i)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coroutine);
    auto typeClass = obj->GetClass();
    ASSERT(typeClass != nullptr);
    if (!typeClass->GetComponentType()->IsBoxed()) {
        VMHandle<P> arrHandle(coroutine, obj->GetCoreType());
        ASSERT(arrHandle.GetPtr() != nullptr);
        return arrHandle->Get(i);
    }
    VMHandle<EtsObjectArray> arrHandle(coroutine, obj->GetCoreType());
    ASSERT(arrHandle.GetPtr() != nullptr);
    auto value = EtsBoxPrimitive<typename P::ValueType>::FromCoreType(arrHandle->Get(i));
    return value->GetValue();
}

EtsBoolean ValueAPIGetElementBoolean(EtsObject *obj, EtsLong i)
{
    return GetElement<EtsBooleanArray>(obj, i);
}

EtsByte ValueAPIGetElementByte(EtsObject *obj, EtsLong i)
{
    return GetElement<EtsByteArray>(obj, i);
}

EtsShort ValueAPIGetElementShort(EtsObject *obj, EtsLong i)
{
    return GetElement<EtsShortArray>(obj, i);
}

EtsChar ValueAPIGetElementChar(EtsObject *obj, EtsLong i)
{
    return GetElement<EtsCharArray>(obj, i);
}

EtsInt ValueAPIGetElementInt(EtsObject *obj, EtsLong i)
{
    return GetElement<EtsIntArray>(obj, i);
}

EtsLong ValueAPIGetElementLong(EtsObject *obj, EtsLong i)
{
    return GetElement<EtsLongArray>(obj, i);
}

EtsFloat ValueAPIGetElementFloat(EtsObject *obj, EtsLong i)
{
    return GetElement<EtsFloatArray>(obj, i);
}

EtsDouble ValueAPIGetElementDouble(EtsObject *obj, EtsLong i)
{
    return GetElement<EtsDoubleArray>(obj, i);
}

}  // namespace ark::ets::intrinsics
