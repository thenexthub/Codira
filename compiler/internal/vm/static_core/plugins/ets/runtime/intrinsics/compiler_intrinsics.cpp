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

#include "intrinsics.h"

#include "plugins/ets/runtime/ets_exceptions.h"
#include "runtime/include/exceptions.h"
#include "compiler/optimizer/ir/constants.h"

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"
namespace ark::ets::intrinsics {

template <panda_file::Type::TypeId FIELD_TYPE, bool IS_GETTER>
Field *TryGetField(ark::Method *method, Field *rawField, uint32_t pc, ark::Class *klass)
{
    bool useIc = pc != ark::compiler::INVALID_PC;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto address = method->GetInstructions() + (useIc ? pc : 0);
    ASSERT(EtsCoroutine::GetCurrent() != nullptr);
    InterpreterCache *cache = EtsCoroutine::GetCurrent()->GetInterpreterCache();
    return GetFieldByName<IS_GETTER>(cache->GetEntry(address), method, rawField, address, klass);
}

template <panda_file::Type::TypeId FIELD_TYPE, bool IS_GETTER>
ark::Method *TryGetCallee(ark::Method *method, Field *rawField, uint32_t pc, ark::Class *klass)
{
    bool useIc = pc != ark::compiler::INVALID_PC;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto address = method->GetInstructions() + (useIc ? pc : 0);
    ASSERT(EtsCoroutine::GetCurrent() != nullptr);
    InterpreterCache *cache = EtsCoroutine::GetCurrent()->GetInterpreterCache();
    return GetAccessorByName<FIELD_TYPE, IS_GETTER>(cache->GetEntry(address), method, rawField, address, klass);
}

template <panda_file::Type::TypeId FIELD_TYPE, class T>
static T GetFieldPrimitiveType(Field *field, const VMHandle<ObjectHeader> &handleObj)
{
    ASSERT(handleObj.GetPtr() != nullptr);
    switch (field->GetTypeId()) {
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::U8: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            return handleObj.GetPtr()->template GetFieldPrimitive<uint8_t>(*field);
        }
        case panda_file::Type::TypeId::I8: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            return handleObj.GetPtr()->template GetFieldPrimitive<int8_t>(*field);
        }
        case panda_file::Type::TypeId::I16: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            return handleObj.GetPtr()->template GetFieldPrimitive<int16_t>(*field);
        }
        case panda_file::Type::TypeId::U16: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            return handleObj.GetPtr()->template GetFieldPrimitive<uint16_t>(*field);
        }
        case panda_file::Type::TypeId::I32: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            return handleObj.GetPtr()->template GetFieldPrimitive<int32_t>(*field);
        }
        case panda_file::Type::TypeId::U32: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            return handleObj.GetPtr()->template GetFieldPrimitive<uint32_t>(*field);
        }
        case panda_file::Type::TypeId::I64: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
            return handleObj.GetPtr()->template GetFieldPrimitive<int64_t>(*field);
        }
        case panda_file::Type::TypeId::U64: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
            return handleObj.GetPtr()->template GetFieldPrimitive<uint64_t>(*field);
        }
        case panda_file::Type::TypeId::F32: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F32);
            return handleObj.GetPtr()->template GetFieldPrimitive<float>(*field);
        }
        case panda_file::Type::TypeId::F64: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F64);
            return handleObj.GetPtr()->template GetFieldPrimitive<double>(*field);
        }
        default:
            UNREACHABLE();
            return handleObj.GetPtr()->template GetFieldPrimitive<T>(*field);
    }
}

template <panda_file::Type::TypeId FIELD_TYPE, class T>
T CompilerEtsLdObjByName(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj)
{
    ASSERT(method != nullptr);
    ark::Class *klass;
    ark::Field *rawField;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handleObj(thread, obj);
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        ASSERT(handleObj.GetPtr() != nullptr);
        klass = static_cast<ark::Class *>(handleObj.GetPtr()->ClassAddr<ark::BaseClass>());
        rawField = classLinker->GetField(*method, panda_file::File::EntityId(id), false);

        Field *field = TryGetField<FIELD_TYPE, true>(method, rawField, pc, klass);
        if (field != nullptr) {
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                return handleObj.GetPtr()->GetFieldObject(*field);
            } else {
                return GetFieldPrimitiveType<FIELD_TYPE, T>(field, handleObj);
            }
        }

        auto callee = TryGetCallee<FIELD_TYPE, true>(method, rawField, pc, klass);
        if (callee != nullptr) {
            Value val(handleObj.GetPtr());
            Value result = callee->Invoke(Coroutine::GetCurrent(), &val);
            return result.GetAs<T>();
        }
    }
    LookUpException<true>(klass, rawField);
    HandlePendingException();
    UNREACHABLE();
}

template <panda_file::Type::TypeId FIELD_TYPE, class T>
/* CC-OFFNXT(G.FUN.01-CPP, huge_method) big switch-case */
static void SetTypedFieldPrimitive(Field *field, const VMHandle<ObjectHeader> &handleObj, T storeValue)
{
    ASSERT(handleObj.GetPtr() != nullptr);
    switch (field->GetTypeId()) {
        case panda_file::Type::TypeId::U1:
        case panda_file::Type::TypeId::U8: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint8_t>(storeValue));
            return;
        }
        case panda_file::Type::TypeId::I8: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<int8_t>(storeValue));
            return;
        }
        case panda_file::Type::TypeId::I16: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<int16_t>(storeValue));
            return;
        }
        case panda_file::Type::TypeId::U16: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint16_t>(storeValue));
            return;
        }
        case panda_file::Type::TypeId::I32: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<int32_t>(storeValue));
            return;
        }
        case panda_file::Type::TypeId::U32: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I32);
            handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint32_t>(storeValue));
            return;
        }
        case panda_file::Type::TypeId::I64: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
            handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<int64_t>(storeValue));
            return;
        }
        case panda_file::Type::TypeId::U64: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::I64);
            handleObj.GetPtr()->SetFieldPrimitive(*field, static_cast<uint64_t>(storeValue));
            return;
        }
        case panda_file::Type::TypeId::F32: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F32);
            handleObj.GetPtr()->SetFieldPrimitive(*field, storeValue);
            return;
        }
        case panda_file::Type::TypeId::F64: {
            ASSERT(FIELD_TYPE == panda_file::Type::TypeId::F64);
            handleObj.GetPtr()->SetFieldPrimitive(*field, storeValue);
            return;
        }
        default: {
            UNREACHABLE();
        }
    }
}

template <panda_file::Type::TypeId FIELD_TYPE, class T>
void CompilerEtsStObjByName(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj, T storeValue)
{
    ASSERT(method != nullptr);
    ark::Class *klass;
    ark::Field *rawField;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handleObj(thread, obj);
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
        rawField = classLinker->GetField(*method, panda_file::File::EntityId(id), false);

        Field *field = TryGetField<FIELD_TYPE, false>(method, rawField, pc, klass);
        if (field != nullptr) {
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                UNREACHABLE();
            } else {
                SetTypedFieldPrimitive<FIELD_TYPE, T>(field, handleObj, storeValue);
                return;
            }
        }

        auto callee = TryGetCallee<FIELD_TYPE, false>(method, rawField, pc, klass);
        if (callee != nullptr) {
            PandaVector<Value> args {Value(handleObj.GetPtr()), Value(storeValue)};
            callee->Invoke(Coroutine::GetCurrent(), args.data());
            return;
        }
    }
    LookUpException<false>(klass, rawField);
    HandlePendingException();
    UNREACHABLE();
}

void CompilerEtsStObjByNameRef(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj,
                               ark::ObjectHeader *storeValue)
{
    ASSERT(method != nullptr);
    ark::Class *klass;
    ark::Field *rawField;
    {
        auto *thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handleObj(thread, obj);
        VMHandle<ObjectHeader> handleStore(thread, storeValue);
        auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
        klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
        rawField = classLinker->GetField(*method, panda_file::File::EntityId(id), false);

        Field *field = TryGetField<panda_file::Type::TypeId::REFERENCE, false>(method, rawField, pc, klass);
        if (field != nullptr) {
            ASSERT(handleObj.GetPtr() != nullptr);
            return handleObj.GetPtr()->SetFieldObject(*field, handleStore.GetPtr());
        }

        auto callee = TryGetCallee<panda_file::Type::TypeId::REFERENCE, false>(method, rawField, pc, klass);
        if (callee != nullptr) {
            PandaVector<Value> args {Value(handleObj.GetPtr()), Value(handleStore.GetPtr())};
            callee->Invoke(Coroutine::GetCurrent(), args.data());
            return;
        }
    }
    LookUpException<false>(klass, rawField);
    HandlePendingException();
    UNREACHABLE();
}

extern "C" int32_t CompilerEtsLdObjByNameI32(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::I32, int32_t>(method, id, pc, obj);
}

extern "C" int64_t CompilerEtsLdObjByNameI64(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::I64, int64_t>(method, id, pc, obj);
}

extern "C" float CompilerEtsLdObjByNameF32(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::F32, float>(method, id, pc, obj);
}

extern "C" double CompilerEtsLdObjByNameF64(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::F64, double>(method, id, pc, obj);
}

extern "C" ark::ObjectHeader *CompilerEtsLdObjByNameObj(ark::Method *method, int32_t id, uint32_t pc,
                                                        ark::ObjectHeader *obj)
{
    return CompilerEtsLdObjByName<panda_file::Type::TypeId::REFERENCE, ark::ObjectHeader *>(method, id, pc, obj);
}

extern "C" void CompilerEtsStObjByNameI8(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj,
                                         int8_t storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::I32, int32_t>(method, id, pc, obj,
                                                                   static_cast<int32_t>(storeValue));
}

extern "C" void CompilerEtsStObjByNameI16(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj,
                                          int16_t storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::I32, int32_t>(method, id, pc, obj,
                                                                   static_cast<int32_t>(storeValue));
}

extern "C" void CompilerEtsStObjByNameI32(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj,
                                          int32_t storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::I32, int32_t>(method, id, pc, obj, storeValue);
}

extern "C" void CompilerEtsStObjByNameI64(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj,
                                          int64_t storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::I64, int64_t>(method, id, pc, obj, storeValue);
}

extern "C" void CompilerEtsStObjByNameF32(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj,
                                          float storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::F32, float>(method, id, pc, obj, storeValue);
}

extern "C" void CompilerEtsStObjByNameF64(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj,
                                          double storeValue)
{
    CompilerEtsStObjByName<panda_file::Type::TypeId::F64, double>(method, id, pc, obj, storeValue);
}

extern "C" void CompilerEtsStObjByNameObj(ark::Method *method, int32_t id, uint32_t pc, ark::ObjectHeader *obj,
                                          ark::ObjectHeader *storeValue)
{
    CompilerEtsStObjByNameRef(method, id, pc, obj, storeValue);
}

extern "C" uint8_t CompilerEtsEquals(ObjectHeader *obj1, ObjectHeader *obj2)
{
    auto coro = EtsCoroutine::GetCurrent();
    return static_cast<uint8_t>(EtsReferenceEquals(coro, EtsObject::FromCoreType(obj1), EtsObject::FromCoreType(obj2)));
}

extern "C" uint8_t CompilerEtsStrictEquals(ObjectHeader *obj1, ObjectHeader *obj2)
{
    auto coro = EtsCoroutine::GetCurrent();
    return static_cast<uint8_t>(
        EtsReferenceEquals<true>(coro, EtsObject::FromCoreType(obj1), EtsObject::FromCoreType(obj2)));
}

extern "C" EtsString *CompilerEtsTypeof(ObjectHeader *obj)
{
    auto coro = EtsCoroutine::GetCurrent();
    return EtsReferenceTypeof(coro, EtsObject::FromCoreType(obj));
}

extern "C" uint8_t CompilerEtsIstrue(ObjectHeader *obj)
{
    auto coro = EtsCoroutine::GetCurrent();
    return static_cast<uint8_t>(EtsIstrue(coro, EtsObject::FromCoreType(obj)));
}

extern "C" void CompilerEtsNullcheck(ObjectHeader *obj)
{
    if (UNLIKELY(obj == nullptr)) {
        ThrowNullPointerException();
    }
}

extern "C" EtsString *CompilerDoubleToStringDecimal(ObjectHeader *cache, uint64_t number,
                                                    [[maybe_unused]] uint64_t unused)
{
    if (UNLIKELY(cache == nullptr)) {
        return DoubleToStringCache::GetNoCache(bit_cast<double>(number));
    }
    return DoubleToStringCache::FromCoreType(cache)->GetOrCache(EtsCoroutine::GetCurrent(), bit_cast<double>(number));
}

}  // namespace ark::ets::intrinsics
