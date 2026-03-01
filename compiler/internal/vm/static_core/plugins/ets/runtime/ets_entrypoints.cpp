/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/ets_entrypoints.h"

#include "include/coretypes/string.h"
#include "include/object_header.h"
#include "libarkfile/shorty_iterator.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_runtime_interface.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_string_case_conversion.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_string_builder.h"
#include "runtime/arch/helpers.h"
#include "runtime/coroutines/coroutine_worker_group.h"
#include "runtime/interpreter/vregister_iterator.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "runtime/include/class_linker-inl.h"

namespace ark::ets {

using TypeId = panda_file::Type::TypeId;

#if defined(__clang__)
#pragma clang diagnostic push
// CC-OFFNXT(warning_suppression) gcc false positive
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
// CC-OFFNXT(warning_suppression) gcc false positive
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

static inline bool Launch(EtsCoroutine *currentCoro, Method *method, const EtsHandle<EtsPromise> &promiseHandle,
                          PandaVector<Value> &&args)
{
    ASSERT(currentCoro != nullptr);
    PandaEtsVM *etsVm = currentCoro->GetPandaVM();
    auto *coroManager = currentCoro->GetCoroutineManager();
    auto promiseRef = etsVm->GetGlobalObjectStorage()->Add(promiseHandle.GetPtr(), mem::Reference::ObjectType::GLOBAL);
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(promiseRef, coroManager);
    // create the coro and put it to the ready queue
    LaunchResult launchResult = currentCoro->GetCoroutineManager()->Launch(
        evt, method, std::move(args), CoroutineWorkerGroup::AnyId(), EtsCoroutine::LAUNCH, false);
    if (UNLIKELY(launchResult != LaunchResult::OK)) {
        // OOM
        if (launchResult == LaunchResult::COROUTINES_LIMIT_EXCEED) {
            Runtime::GetCurrent()->GetInternalAllocator()->Delete(evt);
        }
        etsVm->GetGlobalObjectStorage()->Remove(promiseRef);
    }
    return launchResult == LaunchResult::OK;
}

void LaunchCoroutine(Method *method, ObjectHeader *obj, uint64_t *args, ObjectHeader *thisObj)
{
    auto *promise = reinterpret_cast<EtsPromise *>(obj);
    ASSERT(promise != nullptr);

    PandaVector<Value> values;
    arch::ArgReaderStack<RUNTIME_ARCH> argReader(reinterpret_cast<uint8_t *>(args));
    arch::ValueWriter writer(&values);
    if (thisObj != nullptr) {
        ASSERT(!method->IsStatic());
        // Add this for virtual call
        values.push_back(Value(thisObj));
    } else {
        if (!method->IsStatic()) {
            auto pThisObj = const_cast<ObjectHeader **>((argReader).template ReadPtr<ObjectHeader *>());
            values.push_back(Value(*pThisObj));
        }
    }
    ARCH_COPY_METHOD_ARGS(method, argReader, writer);

    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    EtsHandle<EtsPromise> promiseHandle(currentCoro, promise);
    // NOTE(panferovi): should be fixed in #19443
    ASSERT(promiseHandle->GetMutex(currentCoro) == nullptr);
    ASSERT(promiseHandle->GetEvent(currentCoro) == nullptr);
    // NOTE(panferovi): issue with raw args and thisObj??
    auto *mutex = EtsMutex::Create(currentCoro);
    promiseHandle->SetMutex(currentCoro, mutex);
    auto *event = EtsEvent::Create(currentCoro);
    promiseHandle->SetEvent(currentCoro, event);
    bool successfulLaunch = Launch(currentCoro, method, promiseHandle, std::move(values));
    if (UNLIKELY(!successfulLaunch)) {
        HandlePendingException();
        UNREACHABLE();
    }
}

extern "C" void CreateLaunchStaticCoroutineEntrypoint(Method *method, ObjectHeader *obj, uint64_t *args)
{
    LaunchCoroutine(method, obj, args, nullptr);
}

extern "C" void CreateLaunchVirtualCoroutineEntrypoint(Method *method, ObjectHeader *obj, uint64_t *args,
                                                       ObjectHeader *thisObj)
{
    LaunchCoroutine(method, obj, args, thisObj);
}

template <BytecodeInstruction::Format FORMAT>
ObjectHeader *LaunchFromInterpreterImpl(Method *method, Frame *frame, const uint8_t *pc)
{
    EtsPromise *promise = EtsPromise::Create();
    if (UNLIKELY(promise == nullptr)) {
        return nullptr;
    }

    auto numArgs = method->GetNumArgs();
    auto args = PandaVector<Value> {numArgs};
    auto frameHandler = StaticFrameHandler(frame);
    auto vregIter = interpreter::VRegisterIterator<FORMAT> {BytecodeInstruction(pc), frame};
    for (decltype(numArgs) i = 0; i < numArgs; ++i) {
        args[i] = Value::FromVReg(frameHandler.GetVReg(vregIter.GetVRegIdx(i)));
    }

    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    EtsHandle<EtsPromise> promiseHandle(currentCoro, promise);
    bool successfulLaunch = Launch(currentCoro, method, promiseHandle, std::move(args));
    if (UNLIKELY(!successfulLaunch)) {
        return nullptr;
    }
    frame->GetAccAsVReg().SetReference(promiseHandle.GetPtr());
    return promiseHandle.GetPtr();
}

extern "C" ObjectHeader *LaunchFromInterpreterShort(Method *method, Frame *frame, const uint8_t *pc)
{
    return LaunchFromInterpreterImpl<BytecodeInstruction::Format::PREF_V4_V4_ID16>(method, frame, pc);
}

extern "C" ObjectHeader *LaunchFromInterpreterLong(Method *method, Frame *frame, const uint8_t *pc)
{
    return LaunchFromInterpreterImpl<BytecodeInstruction::Format::PREF_V4_V4_V4_V4_ID16>(method, frame, pc);
}

extern "C" ObjectHeader *LaunchFromInterpreterRange(Method *method, Frame *frame, const uint8_t *pc)
{
    return LaunchFromInterpreterImpl<BytecodeInstruction::Format::PREF_V8_ID16>(method, frame, pc);
}

extern "C" Field *LookupFieldByNameEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id,
                                              Method *caller, const uint8_t *pc)
{
    auto current = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    Field *metaField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id), false);
    if (UNLIKELY(metaField == nullptr)) {
        HandlePendingException();
        return nullptr;
    }
    return GetFieldByName(entry, caller, metaField, pc, current);
}

template <panda_file::Type::TypeId FORMAT>
Method *LookupGetterByNameEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id, Method *caller,
                                     const uint8_t *pc)
{
    auto current = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    Field *metaField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id), false);
    if (UNLIKELY(metaField == nullptr)) {
        HandlePendingException();
        return nullptr;
    }
    return GetAccessorByName<FORMAT, true>(entry, caller, metaField, pc, current);
}

template <panda_file::Type::TypeId FORMAT>
Method *LookupSetterByNameEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id, Method *caller,
                                     const uint8_t *pc)
{
    auto current = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    Field *metaField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id), false);
    if (UNLIKELY(metaField == nullptr)) {
        HandlePendingException();
        return nullptr;
    }
    return GetAccessorByName<FORMAT, false>(entry, caller, metaField, pc, current);
}

extern "C" Method *LookupGetterByNameShortEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id,
                                                     Method *caller, const uint8_t *pc)
{
    return LookupGetterByNameEntrypoint<panda_file::Type::TypeId::I32>(entry, obj, id, caller, pc);
}

extern "C" Method *LookupGetterByNameLongEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id,
                                                    Method *caller, const uint8_t *pc)
{
    return LookupGetterByNameEntrypoint<panda_file::Type::TypeId::I64>(entry, obj, id, caller, pc);
}

extern "C" Method *LookupGetterByNameObjEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id,
                                                   Method *caller, const uint8_t *pc)
{
    return LookupGetterByNameEntrypoint<panda_file::Type::TypeId::REFERENCE>(entry, obj, id, caller, pc);
}

extern "C" Method *LookupSetterByNameShortEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id,
                                                     Method *caller, const uint8_t *pc)
{
    return LookupSetterByNameEntrypoint<panda_file::Type::TypeId::I32>(entry, obj, id, caller, pc);
}

extern "C" Method *LookupSetterByNameLongEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id,
                                                    Method *caller, const uint8_t *pc)
{
    return LookupSetterByNameEntrypoint<panda_file::Type::TypeId::I64>(entry, obj, id, caller, pc);
}

extern "C" Method *LookupSetterByNameObjEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id,
                                                   Method *caller, const uint8_t *pc)
{
    return LookupSetterByNameEntrypoint<panda_file::Type::TypeId::REFERENCE>(entry, obj, id, caller, pc);
}

extern "C" void ThrowEtsExceptionNoSuchGetterEntrypoint(ObjectHeader *obj, uint32_t id, Method *caller)
{
    auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto rawField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id), false);
    LookUpException<true>(klass, rawField);
}

extern "C" void ThrowEtsExceptionNoSuchSetterEntrypoint(ObjectHeader *obj, uint32_t id, Method *caller)
{
    auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto rawField = classLinker->GetField(*caller, caller->GetClass()->ResolveFieldIndex(id), false);
    LookUpException<false>(klass, rawField);
}

extern "C" Method *LookupMethodByNameEntrypoint(InterpreterCache::Entry *entry, ObjectHeader *obj, uint32_t id,
                                                Method *caller, const uint8_t *pc)
{
    auto current = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    Method *metaMethod = classLinker->GetMethod(*caller, caller->GetClass()->ResolveMethodIndex(id));
    if (UNLIKELY(metaMethod == nullptr)) {
        HandlePendingException();
        return nullptr;
    }
    ETSStubCacheInfo cacheInfo {entry, caller, pc};
    return GetMethodByName(currentCoro, cacheInfo, metaMethod, current);
}

extern "C" void ThrowEtsExceptionNoSuchMethodEntrypoint(ObjectHeader *obj, uint32_t id, Method *caller)
{
    auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto rawMethod = classLinker->GetMethod(*caller, caller->GetClass()->ResolveMethodIndex(id));
    if (UNLIKELY(rawMethod == nullptr)) {
        HandlePendingException();
    }
    LookUpException(klass, rawMethod);
}

extern "C" ObjectHeader *StringBuilderAppendLongEntrypoint(ObjectHeader *sb, int64_t v)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendLong(sb, v);
}

extern "C" ObjectHeader *StringBuilderAppendCharEntrypoint(ObjectHeader *sb, uint16_t ch)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendChar(sb, ch);
}

extern "C" ObjectHeader *StringBuilderAppendBoolEntrypoint(ObjectHeader *sb, uint8_t v)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendBool(sb, ToEtsBoolean(static_cast<bool>(v)));
}

extern "C" ObjectHeader *StringBuilderAppendStringEntrypoint(ObjectHeader *sb, ObjectHeader *str)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendString(sb, reinterpret_cast<EtsString *>(str));
}

extern "C" ObjectHeader *StringBuilderAppendString2Entrypoint(ObjectHeader *sb, ObjectHeader *str0, ObjectHeader *str1)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendStrings(sb, reinterpret_cast<EtsString *>(str0), reinterpret_cast<EtsString *>(str1));
}

extern "C" ObjectHeader *StringBuilderAppendString3Entrypoint(ObjectHeader *sb, ObjectHeader *str0, ObjectHeader *str1,
                                                              ObjectHeader *str2)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendStrings(sb, reinterpret_cast<EtsString *>(str0), reinterpret_cast<EtsString *>(str1),
                                      reinterpret_cast<EtsString *>(str2));
}

extern "C" ObjectHeader *StringBuilderAppendString4Entrypoint(ObjectHeader *sb, ObjectHeader *str0, ObjectHeader *str1,
                                                              ObjectHeader *str2, ObjectHeader *str3)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendStrings(sb, reinterpret_cast<EtsString *>(str0), reinterpret_cast<EtsString *>(str1),
                                      reinterpret_cast<EtsString *>(str2), reinterpret_cast<EtsString *>(str3));
}

extern "C" ObjectHeader *StringBuilderAppendNullStringEntrypoint(ObjectHeader *sb)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendNullString(sb);
}

extern "C" bool IsClassValueTypedEntrypoint(Class *cls)
{
    return EtsClass::FromRuntimeClass(cls)->IsValueTyped();
}

extern "C" bool CompareETSValueTypedEntrypoint(ManagedThread *thread, ObjectHeader *obj1, ObjectHeader *obj2)
{
    auto coro = EtsCoroutine::CastFromThread(thread);
    auto eobj1 = EtsObject::FromCoreType(obj1);
    auto eobj2 = EtsObject::FromCoreType(obj2);
    return EtsValueTypedEquals(coro, eobj1, eobj2);
}

extern "C" EtsString *EtsGetTypeofEntrypoint(ManagedThread *thread, ObjectHeader *obj)
{
    EtsCoroutine *coro = EtsCoroutine::CastFromThread(thread);
    EtsObject *eobj = EtsObject::FromCoreType(obj);
    return EtsGetTypeof(coro, eobj);
}

extern "C" bool EtsIstrueEntrypoint(ManagedThread *thread, ObjectHeader *obj)
{
    EtsCoroutine *coro = EtsCoroutine::CastFromThread(thread);
    EtsObject *eobj = EtsObject::FromCoreType(obj);
    return EtsIstrue(coro, eobj);
}

extern "C" ObjectHeader *StringBuilderToStringEntrypoint(ObjectHeader *sb)
{
    ASSERT(sb != nullptr);
    return StringBuilderToString(sb)->GetCoreType();
}

extern "C" ObjectHeader *DoubleToStringDecimalEntrypoint(ObjectHeader *cache, uint64_t number)
{
    if (UNLIKELY(cache == nullptr)) {
        return DoubleToStringDecimalNoCacheEntrypoint(number);
    }
    return DoubleToStringCache::FromCoreType(cache)
        ->GetOrCache(EtsCoroutine::GetCurrent(), bit_cast<double>(number))
        ->GetCoreType();
}

extern "C" ObjectHeader *DoubleToStringDecimalStoreEntrypoint(ObjectHeader *elem, uint64_t number, uint64_t cached)
{
    auto *cache = PandaEtsVM::GetCurrent()->GetDoubleToStringCache();
    if (UNLIKELY(cache == nullptr)) {
        return DoubleToStringDecimalNoCacheEntrypoint(number);
    }
    return cache->CacheAndGetNoCheck(EtsCoroutine::GetCurrent(), bit_cast<double>(number), elem, cached)->GetCoreType();
}

extern "C" ObjectHeader *DoubleToStringDecimalNoCacheEntrypoint(uint64_t number)
{
    return DoubleToStringCache::GetNoCache(bit_cast<double>(number))->GetCoreType();
}

extern "C" void BeginGeneralNativeMethod()
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *storage = coroutine->GetEtsNapiEnv()->GetEtsReferenceStorage();

    constexpr uint32_t MAX_LOCAL_REF = 4096;
    if (UNLIKELY(!storage->PushLocalEtsFrame(MAX_LOCAL_REF))) {
        LOG(FATAL, RUNTIME) << "eTS NAPI push local frame failed";
    }

    coroutine->NativeCodeBegin();
}

extern "C" void EndGeneralNativeMethodPrim()
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *storage = coroutine->GetEtsNapiEnv()->GetEtsReferenceStorage();

    coroutine->NativeCodeEnd();
    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
}

extern "C" ObjectHeader *EndGeneralNativeMethodObj(ark::ets::EtsReference *etsRef)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *storage = coroutine->GetEtsNapiEnv()->GetEtsReferenceStorage();
    coroutine->NativeCodeEnd();
    ObjectHeader *ret = nullptr;
    if (LIKELY(!coroutine->HasPendingException())) {
        ret = storage->GetEtsObject(etsRef)->GetCoreType();
    }

    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
    return ret;
}

extern "C" void BeginQuickNativeMethod()
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *storage = coroutine->GetEtsNapiEnv()->GetEtsReferenceStorage();

    constexpr uint32_t MAX_LOCAL_REF = 4096;
    if (UNLIKELY(!storage->PushLocalEtsFrame(MAX_LOCAL_REF))) {
        LOG(FATAL, RUNTIME) << "eTS NAPI push local frame failed";
    }
}

extern "C" void EndQuickNativeMethodPrim()
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *storage = coroutine->GetEtsNapiEnv()->GetEtsReferenceStorage();

    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
}

extern "C" ObjectHeader *EndQuickNativeMethodObj(ark::ets::EtsReference *etsRef)
{
    auto *coroutine = EtsCoroutine::GetCurrent();
    ASSERT(coroutine != nullptr);
    auto *storage = coroutine->GetEtsNapiEnv()->GetEtsReferenceStorage();
    ObjectHeader *ret = nullptr;
    if (LIKELY(!coroutine->HasPendingException())) {
        ret = storage->GetEtsObject(etsRef)->GetCoreType();
    }

    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
    return ret;
}

extern "C" uintptr_t NO_ADDRESS_SANITIZE ResolveCallByNameEntrypoint(const Method *caller, ObjectHeader *obj,
                                                                     size_t calleeId)
{
    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    auto *classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto klass = static_cast<ark::Class *>(obj->ClassAddr<ark::BaseClass>());
    auto rawMethod = classLinker->GetMethod(*caller, panda_file::File::EntityId(calleeId));
    if (LIKELY(rawMethod != nullptr)) {
        auto *resolved = ResolveCompatibleVMethod(currentCoro, klass, rawMethod);
        ASSERT(resolved != nullptr);
        return reinterpret_cast<uintptr_t>(resolved);
    }

    HandlePendingException();
    UNREACHABLE();
}

extern "C" coretypes::String *CreateStringFromCharCodeSingleNoCacheEntrypoint(uint64_t charCode)
{
    // We could use `ark::ets::intrinsics::StdCoreStringFromCharCodeSingle` as the entrypoint, it works, but
    // that function lookups the char code in the cache while we are sure the char code is not cached.
    return EtsString::CreateNewStringFromCharCode(bit_cast<double>(charCode))->GetCoreType();
}

extern "C" int32_t WriteStringToMem(int64_t buf, ObjectHeader *s)
{
    auto str = reinterpret_cast<EtsString *>(s);
    auto addr = reinterpret_cast<void *>(buf);
    auto size = static_cast<uint32_t>(str->GetUtf8Length());

    if (str->IsUtf16()) {
        if (size != str->CopyDataRegionUtf8(addr, 0, size, size)) {
            return -1;
        }
    } else {
        PandaVector<uint8_t> tree8Buf;
        if (memcpy_s(addr, size, str->IsTreeString() ? str->GetTreeStringDataMUtf8(tree8Buf) : str->GetDataMUtf8(),
                     size) != 0) {
            return -1;
        }
    }
    return size;
}

extern "C" ObjectHeader *CreateStringFromMem(int64_t buf, int32_t len)
{
    auto str = EtsString::CreateFromUtf8(reinterpret_cast<const char *>(buf), static_cast<uint32_t>(len));
    return reinterpret_cast<ObjectHeader *>(str);
}

extern "C" int32_t GetStringSizeInBytes(ObjectHeader *str)
{
    return reinterpret_cast<EtsString *>(str)->GetUtf8Length();
}

extern "C" bool SameValueZeroEntrypoint(ObjectHeader *o1, ObjectHeader *o2)
{
    return ark::ets::intrinsics::helpers::SameValueZero(EtsCoroutine::GetCurrent(), EtsObject::FromCoreType(o1),
                                                        EtsObject::FromCoreType(o2));
}

extern "C" EtsBoolean EtsStringEqualsEntrypoint(coretypes::String *str1, coretypes::String *str2)
{
    // We could use `ark::ets::intrinsics::StdCoreStringEquals` as the entrypoint, it works, but
    // `ark::ets::intrinsics::StdCoreStringEquals` repeats the checks that the irtoc implementation
    // of the intrinsic has already performed: whether both pointers contain the same memory address
    // as well as for whether the pointees are actually strings.
    return ToEtsBoolean(EtsString::FromCoreType(str1)->Compare(EtsString::FromCoreType(str2)) == 0);
}

extern "C" EtsBoolean EtsDefaultLocaleAllowsFastLatinCaseConversion()
{
    return ToEtsBoolean(ark::ets::intrinsics::caseconversion::DefaultLocaleAllowsFastLatinCaseConversion());
}

}  // namespace ark::ets
