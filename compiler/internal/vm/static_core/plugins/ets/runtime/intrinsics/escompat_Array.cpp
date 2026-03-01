/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <charconv>
#include <cstddef>
#include <cstdint>

#include <endian.h>
#include <optional>

#include "runtime/include/coretypes/string.h"
#include "intrinsics.h"
#include "interpreter/runtime_interface.h"
#include "libarkbase/utils/utf.h"
#include "libarkbase/utils/utils.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_stubs.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_base_enum.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets::intrinsics {

EtsObject *EtsEscompatArrayGet(EtsEscompatArray *array, int32_t index)
{
    return array->EscompatArrayGet(index);
}

void EtsEscompatArraySet(EtsEscompatArray *array, int32_t index, EtsObject *value)
{
    return array->EscompatArraySet(index, value);
}

template <typename Func>
constexpr auto CreateTypeMap(const EtsPlatformTypes *ptypes)
{
    return std::array {std::pair {ptypes->coreDouble, &Func::template Call<EtsDouble>},
                       std::pair {ptypes->coreInt, &Func::template Call<EtsInt>},
                       std::pair {ptypes->coreByte, &Func::template Call<EtsByte>},
                       std::pair {ptypes->coreShort, &Func::template Call<EtsShort>},
                       std::pair {ptypes->coreLong, &Func::template Call<EtsLong>},
                       std::pair {ptypes->coreFloat, &Func::template Call<EtsFloat>},
                       std::pair {ptypes->coreChar, &Func::template Call<EtsChar>},
                       std::pair {ptypes->coreBoolean, &Func::template Call<EtsBoolean>}};
}

EtsInt NormalizeArrayIndex(EtsInt index, EtsInt actualLength)
{
    if (index < -actualLength) {
        return 0;
    }

    if (index < 0) {
        return actualLength + index;
    }

    if (index > actualLength) {
        return actualLength;
    }

    return index;
}

EtsDouble EtsEscompatArrayIndexOfString(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex, EtsInt actualLength)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    auto valueString = coretypes::String::Cast(value->GetCoreType());
    VMHandle<coretypes::String> strHandle(coroutine, valueString);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->IsStringClass() &&
            strHandle->Compare(coretypes::String::Cast(element->GetCoreType()), ctx) == 0) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfString(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    auto valueString = coretypes::String::Cast(value->GetCoreType());
    VMHandle<coretypes::String> strHandle(coroutine, valueString);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->IsStringClass() &&
            strHandle->Compare(coretypes::String::Cast(element->GetCoreType()), ctx) == 0) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfEnum(EtsObjectArray *buffer, EtsCoroutine *coro, EtsObject *value, EtsInt fromIndex,
                                      EtsInt actualLength)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *valueEnum = EtsBaseEnum::FromEtsObject(value)->GetValue();
    VMHandle<EtsBaseEnum> valueEnumHandle(coro, valueEnum->GetCoreType());
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        auto elementClass = element->GetClass();
        if (elementClass->IsEtsEnum()) {
            auto *elementEnum = EtsBaseEnum::FromEtsObject(element)->GetValue();
            if (EtsReferenceEquals(coro, valueEnumHandle.GetPtr(), elementEnum)) {
                return index;
            }
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfEnum(EtsObjectArray *buffer, EtsCoroutine *coro, EtsObject *value,
                                          EtsInt fromIndex)
{
    [[maybe_unused]] EtsHandleScope scope(coro);
    auto *valueEnum = EtsBaseEnum::FromEtsObject(value)->GetValue();
    VMHandle<EtsBaseEnum> valueEnumHandle(coro, valueEnum->GetCoreType());
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        auto elementClass = element->GetClass();
        if (elementClass->IsEtsEnum()) {
            auto *elementEnum = EtsBaseEnum::FromEtsObject(element)->GetValue();
            if (EtsReferenceEquals(coro, valueEnumHandle.GetPtr(), elementEnum)) {
                return index;
            }
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfNull(EtsObjectArray *buffer, EtsCoroutine *coroutine, EtsInt fromIndex,
                                      EtsInt actualLength)
{
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element == EtsObject::FromCoreType(coroutine->GetNullValue())) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfUndefined(EtsObjectArray *buffer, EtsInt fromIndex, EtsInt actualLength)
{
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element == nullptr) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfUndefined(EtsObjectArray *buffer, EtsInt fromIndex)
{
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element == nullptr) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfNull(EtsObjectArray *buffer, EtsCoroutine *coroutine, EtsInt fromIndex)
{
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element == EtsObject::FromCoreType(coroutine->GetNullValue())) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayIndexOfBigInt(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex, EtsInt actualLength)
{
    auto valueBigInt = EtsBigInt::FromEtsObject(value);
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        auto elementClass = element == nullptr ? nullptr : element->GetClass();
        if (elementClass != nullptr && elementClass->IsBigInt() &&
            EtsBigIntEquality(valueBigInt, EtsBigInt::FromEtsObject(element))) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfBigInt(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex)
{
    auto valueBigInt = EtsBigInt::FromEtsObject(value);
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        auto elementClass = element == nullptr ? nullptr : element->GetClass();
        if (elementClass != nullptr && elementClass->IsBigInt() &&
            EtsBigIntEquality(valueBigInt, EtsBigInt::FromEtsObject(element))) {
            return index;
        }
    }
    return -1;
}

template <typename T>
static auto GetNumericValue(EtsObject *element)
{
    return EtsBoxPrimitive<T>::Unbox(element);
}

static bool IsIntegerClass(EtsClass *objClass, const EtsPlatformTypes *ptypes)
{
    return (objClass == ptypes->coreLong || objClass == ptypes->coreByte || objClass == ptypes->coreShort ||
            objClass == ptypes->coreInt);
}

static bool IsNumericClass(EtsClass *objClass, const EtsPlatformTypes *ptypes)
{
    return (objClass == ptypes->coreDouble || objClass == ptypes->coreFloat);
}

template <typename T>
EtsDouble EtsEscompatArrayIndexOfNumeric(EtsObjectArray *buffer, EtsClass *valueCls, EtsObject *value, EtsInt fromIndex,
                                         EtsInt actualLength)
{
    auto valTyped = GetNumericValue<T>(value);
    if (std::isnan(valTyped)) {
        return -1;
    }

    if (actualLength <= fromIndex) {
        return -1;
    }

    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObjectArray> bufferHandle(coro, buffer);

    auto count = actualLength - fromIndex;
    constexpr std::size_t chunkSize = 1U << 9;
    auto iterCount = count / chunkSize;
    auto remainder = count % chunkSize;

    auto processChunk = [fromIndex, &valueCls, valTyped, &bufferHandle](std::size_t iter,
                                                                        std::size_t size) -> std::optional<EtsDouble> {
        auto startOffset = iter * chunkSize;
        auto endOffset = startOffset + size;
        for (size_t j = startOffset; j < endOffset; ++j) {
            auto currentIndex = fromIndex + j;
            auto element = bufferHandle.GetPtr()->Get(currentIndex);
            if (element != nullptr && element->GetClass() == valueCls && valTyped == GetNumericValue<T>(element)) {
                return currentIndex;
            }
        }
        return std::nullopt;
    };

    for (size_t i = 0; i < iterCount; ++i) {
        if (auto result = processChunk(i, chunkSize); result.has_value()) {
            return result.value();
        }
        ark::interpreter::RuntimeInterface::Safepoint();
    }
    if (remainder > 0) {
        if (auto result = processChunk(iterCount, remainder); result.has_value()) {
            return result.value();
        }
    }

    return -1;
}

template <typename T>
EtsDouble EtsEscompatArrayLastIndexOfNumeric(EtsObjectArray *buffer, EtsClass *valueCls, EtsObject *value,
                                             EtsInt fromIndex)
{
    auto valTyped = GetNumericValue<T>(value);
    if (std::isnan(valTyped)) {
        return -1;
    }

    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element != nullptr && element->GetClass() == valueCls && valTyped == GetNumericValue<T>(element)) {
            return index;
        }
    }
    return -1;
}

template <typename T>
static EtsDouble DispatchIndexOf(EtsObjectArray *buf, EtsClass *cls, EtsObject *val, EtsInt idx, EtsInt len)
{
    return EtsEscompatArrayIndexOfNumeric<T>(buf, cls, val, idx, len);
}

template <typename T>
static EtsDouble DispatchLastIndexOf(EtsObjectArray *buf, EtsClass *cls, EtsObject *val, EtsInt idx)
{
    return EtsEscompatArrayLastIndexOfNumeric<T>(buf, cls, val, idx);
}

struct IndexOfDispatcher {
    template <typename T>
    static EtsDouble Call(EtsObjectArray *buf, EtsClass *cls, EtsObject *val, EtsInt idx, EtsInt len)
    {
        return DispatchIndexOf<T>(buf, cls, val, idx, len);
    }
};

struct LastIndexOfDispatcher {
    template <typename T>
    static EtsDouble Call(EtsObjectArray *buf, EtsClass *cls, EtsObject *val, EtsInt idx)
    {
        return DispatchLastIndexOf<T>(buf, cls, val, idx);
    }
};

EtsDouble EtsEscompatArrayIndexOfCommon(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex, EtsInt actualLength)
{
    for (EtsInt index = fromIndex; index < actualLength; index++) {
        auto element = buffer->Get(index);
        if (element == value) {
            return index;
        }
    }
    return -1;
}

EtsDouble EtsEscompatArrayLastIndexOfCommon(EtsObjectArray *buffer, EtsObject *value, EtsInt fromIndex)
{
    for (EtsInt index = fromIndex; index >= 0; index--) {
        auto element = buffer->Get(index);
        if (element == value) {
            return index;
        }
    }
    return -1;
}

extern "C" EtsInt EtsEscompatArrayInternalIndexOf(ObjectHeader *bufferObject, EtsInt actualLength, EtsObject *value,
                                                  EtsInt fromIndex)
{
    auto *buffer = EtsObjectArray::FromCoreType(bufferObject);
    fromIndex = NormalizeArrayIndex(fromIndex, actualLength);
    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    const EtsPlatformTypes *ptypes = PlatformTypes(coroutine);

    if (value == nullptr) {
        return EtsEscompatArrayIndexOfUndefined(buffer, fromIndex, actualLength);
    }

    auto valueCls = value->GetClass();
    static const auto TYPE_MAP = CreateTypeMap<IndexOfDispatcher>(ptypes);
    for (const auto &[type, handler] : TYPE_MAP) {
        if (valueCls == type) {
            return handler(buffer, valueCls, value, fromIndex, actualLength);
        }
    }

    if (valueCls->IsBigInt()) {
        return EtsEscompatArrayIndexOfBigInt(buffer, value, fromIndex, actualLength);
    }

    if (valueCls->IsStringClass()) {
        return EtsEscompatArrayIndexOfString(buffer, value, fromIndex, actualLength);
    }

    if (valueCls->IsEtsEnum()) {
        return EtsEscompatArrayIndexOfEnum(buffer, coroutine, value, fromIndex, actualLength);
    }

    if (value == EtsObject::FromCoreType(coroutine->GetNullValue())) {
        return EtsEscompatArrayIndexOfNull(buffer, coroutine, fromIndex, actualLength);
    }

    return EtsEscompatArrayIndexOfCommon(buffer, value, fromIndex, actualLength);
}

extern "C" EtsInt EtsEscompatArrayInternalLastIndexOf(ObjectHeader *bufferObject, EtsInt actualLength, EtsObject *value,
                                                      EtsInt fromIndex)
{
    if (actualLength == 0) {
        return -1;
    }
    auto *buffer = EtsObjectArray::FromCoreType(bufferObject);

    EtsCoroutine *coroutine = EtsCoroutine::GetCurrent();
    const EtsPlatformTypes *ptypes = PlatformTypes(coroutine);
    EtsInt startIndex = 0;

    if (fromIndex >= 0) {
        startIndex = std::min(actualLength - 1, fromIndex);
    } else {
        startIndex = actualLength + fromIndex;
    }

    if (value == nullptr) {
        return EtsEscompatArrayLastIndexOfUndefined(buffer, startIndex);
    }

    auto valueCls = value->GetClass();
    static const auto TYPE_MAP = CreateTypeMap<LastIndexOfDispatcher>(ptypes);
    for (const auto &[type, handler] : TYPE_MAP) {
        if (valueCls == type) {
            return handler(buffer, valueCls, value, startIndex);
        }
    }

    if (valueCls->IsBigInt()) {
        return EtsEscompatArrayLastIndexOfBigInt(buffer, value, startIndex);
    }

    if (valueCls->IsStringClass()) {
        return EtsEscompatArrayLastIndexOfString(buffer, value, startIndex);
    }

    if (valueCls->IsEtsEnum()) {
        return EtsEscompatArrayLastIndexOfEnum(buffer, coroutine, value, startIndex);
    }

    if (value == EtsObject::FromCoreType(coroutine->GetNullValue())) {
        return EtsEscompatArrayLastIndexOfNull(buffer, coroutine, startIndex);
    }

    return EtsEscompatArrayLastIndexOfCommon(buffer, value, startIndex);
}

static uint32_t NormalizeIndex(int32_t idx, int64_t len)
{
    if (idx < 0) {
        return idx < -len ? 0 : idx + len;
    }
    return idx > len ? len : idx;
}

extern "C" void EtsEscompatArrayFillImpl(ObjectHeader *bufferHeader, int32_t length, EtsObject *value, int32_t start,
                                         int32_t end)
{
    ASSERT(bufferHeader != nullptr);
    auto *buffer = EtsObjectArray::FromEtsObject(EtsObject::FromCoreType(bufferHeader));
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObjectArray> bufferHandle(coro, buffer);
    EtsHandle<EtsObject> valueHandle(coro, value);
    auto startInd = NormalizeIndex(start, length);
    auto endInd = NormalizeIndex(end, length);
    if (endInd <= startInd) {
        return;
    }

    auto count = endInd - startInd;
    constexpr std::size_t chunkSize = 1U << 12;
    auto iterCount = count / chunkSize;
    auto remainder = count % chunkSize;

    auto processChunk = [&bufferHandle, &valueHandle, startInd](std::size_t iter, std::size_t size) {
        auto startOffset = iter * chunkSize + startInd;
        auto endOffset = startOffset + size;
        bufferHandle->Fill(valueHandle.GetPtr(), startOffset, endOffset);
    };

    for (std::uint32_t i = 0; i < iterCount; ++i) {
        processChunk(i, chunkSize);
        ark::interpreter::RuntimeInterface::Safepoint();
    }
    if (remainder > 0) {
        processChunk(iterCount, remainder);
    }
}

template <typename T>
static void MemAtomicSwap(void *aAddr, void *bAddr)
{
    auto a = reinterpret_cast<std::atomic<T> *>(aAddr);
    auto b = reinterpret_cast<std::atomic<T> *>(bAddr);
    // NOTE: exchange works slower on device than load/store pair
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    auto v1 = a->load(std::memory_order_relaxed);
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    auto v2 = b->load(std::memory_order_relaxed);
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    a->store(v2, std::memory_order_relaxed);
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    b->store(v1, std::memory_order_relaxed);
}

template <typename T>
static void MemAtomicSwapReadBarrier(mem::GCBarrierSet *barrierSet, const void *memStart, void *aAddr, void *bAddr)
{
    auto *a1 = reinterpret_cast<std::atomic<T> *>(aAddr);
    auto *a2 = reinterpret_cast<std::atomic<T> *>(bAddr);
    auto *b1 = reinterpret_cast<const std::atomic<T> *>(barrierSet->PreReadBarrier(
        memStart, reinterpret_cast<uint8_t *>(aAddr) - reinterpret_cast<const uint8_t *>(memStart)));
    auto *b2 = reinterpret_cast<const std::atomic<T> *>(barrierSet->PreReadBarrier(
        memStart, reinterpret_cast<const uint8_t *>(bAddr) - reinterpret_cast<const uint8_t *>(memStart)));
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    auto v1 = b1->load(std::memory_order_relaxed);
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    auto v2 = b2->load(std::memory_order_relaxed);
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    a2->store(v1, std::memory_order_relaxed);
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    a1->store(v2, std::memory_order_relaxed);
}

template <typename T>
static auto GetSwap([[maybe_unused]] void *arrAddr, [[maybe_unused]] mem::GCBarrierSet *readBarrierSet)
{
#ifdef ARK_HYBRID
    bool usePreReadBarrier = readBarrierSet->IsPreReadBarrierEnabled();

    return [usePreReadBarrier, readBarrierSet, arrAddr](T *aPtr, T *bPtr) {
        if (usePreReadBarrier) {
            MemAtomicSwapReadBarrier<T>(readBarrierSet, arrAddr, aPtr, bPtr);
        } else {
            MemAtomicSwap<T>(aPtr, bPtr);
        }
    };
#else
    return [](T *aPtr, T *bPtr) { MemAtomicSwap<T>(aPtr, bPtr); };
#endif
}

// Performance measurement demonstrates that SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD object pointers
// are swapped in 0.20-0.40 ms.
static constexpr size_t SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD = 50000UL;

template <typename T>
static void RefReverse(ManagedThread *thread, EtsHandle<EtsObjectArray> &buffer, int32_t length)
{
    auto *arr = buffer->GetData<T>();
    auto *barrierSet = thread->GetBarrierSet();
    auto swap = GetSwap<T>(arr, barrierSet);
    bool usePreBarrier = barrierSet->IsPreBarrierEnabled();

    auto swapWithBarriers = [&swap, &usePreBarrier, barrierSet](T *aPtr, T *bPtr) {
        if (usePreBarrier) {
            barrierSet->PreBarrier(reinterpret_cast<void *>(*aPtr));
            barrierSet->PreBarrier(reinterpret_cast<void *>(*bPtr));
        }
        swap(aPtr, bPtr);
    };
    auto putSafepoint = [&usePreBarrier, barrierSet, &arr, thread](size_t dstStart, size_t dstEndMirror, size_t count) {
        if (barrierSet->GetPostType() != ark::mem::BarrierType::POST_WRB_NONE) {
            constexpr uint32_t offset = ark::coretypes::Array::GetDataOffset();
            const uint32_t size = count * sizeof(T);
            barrierSet->PostBarrier(arr, offset + dstStart * sizeof(T), size);
            barrierSet->PostBarrier(arr, offset + dstEndMirror * sizeof(T) - size, size);
        }
        ark::interpreter::RuntimeInterface::Safepoint(thread);
        usePreBarrier = barrierSet->IsPreBarrierEnabled();
    };
    auto halfLength = static_cast<size_t>(length) / 2;
    // Reverse array in groups of SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD length
    for (size_t i = 0; i < halfLength / SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD; ++i) {
        size_t groupIdx = i * SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD;
        for (size_t j = groupIdx; j < groupIdx + SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD; ++j) {
            swapWithBarriers(&arr[j], &arr[length - 1 - j]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        putSafepoint(groupIdx, length - 1 - groupIdx, SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD);
        // If GC suspend worker during RefReverse execution then it may move array pointed by arr to a different memory
        // location - should re-read a new array address
        arr = buffer->GetData<T>();
    }
    // Reverse possible remaining part with length less than SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD
    size_t finalIdx = (halfLength / SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD) * SWAPPED_BETWEEN_SAFEPOINT_THRESHOLD;
    for (size_t i = finalIdx; i < halfLength; ++i) {
        swapWithBarriers(&arr[i], &arr[length - 1 - i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    putSafepoint(finalIdx, length - 1 - finalIdx, halfLength - finalIdx);
}

extern "C" void EtsEscompatArrayReverse(ObjectHeader *bufferHeader, int32_t length)
{
    ASSERT(bufferHeader != nullptr);
    auto *buffer = EtsObjectArray::FromEtsObject(EtsObject::FromCoreType(bufferHeader));
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsObjectArray> bufferHandle(coro, buffer);

    RefReverse<ObjectPointerType>(coro, bufferHandle, length);
}

struct ElementComputeResult {
    explicit ElementComputeResult(EtsCoroutine *c) : coro(c), ptypes(PlatformTypes(c)) {};
    NO_COPY_SEMANTIC(ElementComputeResult);
    DEFAULT_MOVE_SEMANTIC(ElementComputeResult);
    ~ElementComputeResult() = default;

    EtsCoroutine *coro {};
    const EtsPlatformTypes *ptypes {};
    size_t intCount = 0;
    size_t doubleCount = 0;
    size_t stringCount = 0;
    size_t utf8Size = 0;
    size_t utf16Size = 0;
    size_t toStringIndex = 0;
    PandaVector<EtsHandle<EtsString>> toStringResults;
};

static constexpr std::string_view TRUE_STRING = "true";
static constexpr std::string_view FALSE_STRING = "false";

// Might trigger GC
static ark::ets::EtsString *GetObjStr(EtsCoroutine *coro, EtsClass *cls, EtsObject *element)
{
    auto *toStrMethod = cls->GetInstanceMethod("toString", ":Lstd/core/String;");
    if (toStrMethod == nullptr) {
        return EtsString::CreateNewEmptyString();
    }

    Value args(element->GetCoreType());
    Value result = toStrMethod->GetPandaMethod()->Invoke(coro, &args);
    if (UNLIKELY(coro->HasPendingException())) {
        return nullptr;
    }
    auto *resultObject = EtsObject::FromCoreType(result.GetAs<ObjectHeader *>());
    ASSERT(resultObject != nullptr);
    return EtsString::FromEtsObject(resultObject);
}

template <typename T>
static constexpr size_t MaxChars()
{
    static_assert(std::numeric_limits<T>::radix == 2U);
    if constexpr (std::is_integral_v<T>) {
        auto bits = std::numeric_limits<T>::digits;
        auto digitPerBit = std::log10(std::numeric_limits<T>::radix);
        auto digits = std::ceil(bits * digitPerBit) + static_cast<int>(std::is_signed_v<T>);
        return digits;
    } else {
        static_assert(std::is_floating_point_v<T>);
        // Forced to treat T as double
        auto bits = std::numeric_limits<double>::digits;
        auto digitPerBit = std::log10(std::numeric_limits<double>::radix);
        auto digits = std::ceil(bits * digitPerBit) + static_cast<int>(std::is_signed_v<double>);
        // MaxChars for double
        // integer part + decimal point + digits + e+308
        //     1        +     1         +   17   +   5
        return 1U + 1U + digits + std::char_traits<char>::length("+e308");
    }
}

// Might trigger GC
static bool ComputeElementCharSize(ElementComputeResult &res, EtsObject *element)
{
    const int booleanUtf8Size = 5;
    if (ets::EtsReferenceNullish(res.coro, element)) {
        return true;
    }

    const auto updateStringSize = [&res](EtsString *str) {
        if (str->IsUtf16()) {
            auto utf16StrSize = str->GetUtf16Length();
            res.utf16Size += utf16StrSize;
        } else {
            auto utf8StrSize = str->GetUtf8Length();
            res.utf8Size += utf8StrSize;
        }
    };

    auto elementCls = element->GetClass();
    if (elementCls->IsStringClass()) {
        auto *strElement = EtsString::FromEtsObject(element);
        updateStringSize(strElement);
        res.stringCount++;
    } else if (IsIntegerClass(elementCls, res.ptypes)) {
        res.intCount++;
    } else if (IsNumericClass(elementCls, res.ptypes)) {
        res.doubleCount++;
    } else if (elementCls == res.ptypes->coreChar) {
        res.utf16Size++;
    } else if (elementCls == res.ptypes->coreBoolean) {
        res.utf8Size += booleanUtf8Size;
    } else {
        auto *objStr = GetObjStr(res.coro, elementCls, element);
        if (UNLIKELY(objStr == nullptr)) {
            ASSERT(res.coro->HasPendingException());
            return false;
        }
        updateStringSize(objStr);
        res.toStringResults.emplace_back(res.coro, objStr);
    }
    return true;
}

template <typename T>
static size_t NumberToChars(PandaVector<EtsChar> &buf, size_t pos, T number)
{
    if constexpr (std::is_integral_v<T>) {
        auto num = number < 0 ? -static_cast<uint64_t>(number) : static_cast<uint64_t>(number);
        auto nDigits = (CountDigits(num) + static_cast<uint32_t>(number < 0));
        ASSERT(pos + nDigits <= buf.size());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        utf::UInt64ToUtf16Array(num, buf.data() + pos, nDigits, number < 0);
        return nDigits;
    } else {
        static_assert(std::is_floating_point_v<T>);
        static_assert(std::is_unsigned_v<EtsChar>);
        // Forced to treat T as double
        auto asDouble = static_cast<double>(number);
        return intrinsics::helpers::FpToStringDecimalRadix(asDouble, [&buf, &pos](std::string_view str) {
            ASSERT(str.length() <= MaxChars<T>());
            for (size_t i = 0; i < str.length(); i++) {
                buf[pos + i] = static_cast<EtsChar>(str[i]);
            }
            return str.length();
        });
    }
}

template <typename T>
static size_t NumberToChars(PandaVector<char> &buf, size_t pos, T number)
{
    if constexpr (std::is_integral_v<T>) {
        auto [strEnd, result] = std::to_chars(&buf[pos], &buf[pos + MaxChars<T>()], number);
        ASSERT(result == std::errc());
        return strEnd - &buf[pos];
    } else {
        static_assert(std::is_floating_point_v<T>);
        // Forced to treat T as double
        auto asDouble = static_cast<double>(number);
        return intrinsics::helpers::FpToStringDecimalRadix(asDouble, [&buf, &pos](std::string_view str) {
            ASSERT(str.length() <= MaxChars<T>());
            MemcpyUnsafe(&buf[pos], str.data(), str.length());
            return str.length();
        });
    }
}

template <typename T, typename BufferType>
static void HandleNumericType(EtsObject *element, BufferType &buf, size_t &pos)
{
    auto value = GetNumericValue<T>(element);
    pos += NumberToChars<T>(buf, pos, value);
}

template <typename BufferType>
using NumericHandler = void (*)(EtsHandle<EtsObject>, BufferType &, size_t &);

template <typename BufferType>
static constexpr auto GetNumericHandlerTable(const EtsPlatformTypes *ptypes)
{
    return std::array {std::pair {ptypes->coreInt, &HandleNumericType<EtsInt, BufferType>},
                       std::pair {ptypes->coreDouble, &HandleNumericType<EtsDouble, BufferType>},
                       std::pair {ptypes->coreByte, &HandleNumericType<EtsByte, BufferType>},
                       std::pair {ptypes->coreShort, &HandleNumericType<EtsShort, BufferType>},
                       std::pair {ptypes->coreLong, &HandleNumericType<EtsLong, BufferType>},
                       std::pair {ptypes->coreFloat, &HandleNumericType<EtsFloat, BufferType>}};
}

// Might trigger GC
static bool ComputeCharSize(ElementComputeResult &res, EtsHandle<EtsObjectArray> &bufferHandle, size_t actualLength,
                            EtsHandle<EtsString> &separatorHandle)
{
    ASSERT(actualLength > 0);
    for (size_t index = 0; index < actualLength; index++) {
        auto *element = bufferHandle->Get(index);
        if (UNLIKELY(!ComputeElementCharSize(res, element))) {
            ASSERT(res.coro->HasPendingException());
            return false;
        }
    }

    res.utf8Size = res.utf8Size + res.intCount * MaxChars<EtsLong>() + res.doubleCount * MaxChars<EtsDouble>();
    if (separatorHandle->IsUtf16()) {
        res.utf16Size += separatorHandle->GetUtf16Length() * (actualLength - 1);
    } else {
        res.utf8Size += separatorHandle->GetUtf8Length() * (actualLength - 1);
    }
    return true;
}

static ark::ets::EtsString *EtsEscompatArrayJoinUtf8String(EtsHandle<EtsObjectArray> &bufferHandle, size_t actualLength,
                                                           EtsInt resultUtf8Size, EtsHandle<EtsString> &separatorHandle)
{
    ASSERT(actualLength > 0);
    EtsString *outputString = EtsString::AllocateNonInitializedString(resultUtf8Size, true);
    uint8_t *dstData = outputString->GetDataMUtf8();
    const size_t sepSize = separatorHandle->GetUtf8Length();

    for (size_t i = 0; i < actualLength - 1; i++) {
        auto *srcString = EtsString::FromEtsObject(bufferHandle->Get(i));
        uint32_t n = srcString->CopyDataRegionMUtf8(dstData, 0, srcString->GetLength(), resultUtf8Size);
        dstData += n;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        resultUtf8Size -= n;
        if (sepSize > 0) {
            uint32_t len =
                separatorHandle->CopyDataRegionMUtf8(dstData, 0, separatorHandle->GetLength(), resultUtf8Size);
            dstData += len;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            resultUtf8Size -= len;
        }
    }

    auto *lastString = EtsString::FromEtsObject(bufferHandle->Get(actualLength - 1));
    lastString->CopyDataRegionMUtf8(dstData, 0, lastString->GetLength(), resultUtf8Size);
    return outputString;
}

static void ProcessUtf8Element(ElementComputeResult &res, EtsObject *element, PandaVector<char> &outputBuffer,
                               size_t &pos)
{
    if (ets::EtsReferenceNullish(res.coro, element)) {
        return;
    }

    auto *elementCls = element->GetClass();
    const auto &numericHandlerTable = GetNumericHandlerTable<PandaVector<char>>(res.ptypes);
    for (const auto &entry : numericHandlerTable) {
        if (entry.first == elementCls) {
            entry.second(element, outputBuffer, pos);
            return;
        }
    }

    const auto writeUtf8String = [&outputBuffer, &pos](EtsString *str) {
        auto elementSize = str->GetUtf8Length();
        if (elementSize > 0) {
            PandaVector<uint8_t> tree8Buf;
            const auto *strData = str->IsTreeString() ? str->GetTreeStringDataUtf8(tree8Buf) : str->GetDataUtf8();
            MemcpyUnsafe(&outputBuffer[pos], strData, elementSize);
            pos += elementSize;
        }
    };

    if (elementCls->IsStringClass()) {
        auto *strElement = EtsString::FromEtsObject(element);
        writeUtf8String(strElement);
    } else if (elementCls == res.ptypes->coreBoolean) {
        auto value = GetNumericValue<EtsBoolean>(element);
        std::string_view text = (value == 1) ? TRUE_STRING : FALSE_STRING;
        MemcpyUnsafe(&outputBuffer[pos], text.data(), text.size());
        pos += text.size();
    } else {
        ASSERT(res.toStringIndex < res.toStringResults.size());
        writeUtf8String(res.toStringResults[res.toStringIndex].GetPtr());
        res.toStringIndex++;
    }
}

static ark::ets::EtsString *EtsEscompatArrayJoinUtf8(ElementComputeResult &res, EtsHandle<EtsObjectArray> &buffer,
                                                     size_t actualLength, EtsHandle<EtsString> &separatorHandle)
{
    if (res.utf8Size == 0) {
        return EtsString::CreateNewEmptyString();
    }

    if (res.stringCount == actualLength) {
        return EtsEscompatArrayJoinUtf8String(buffer, actualLength, res.utf8Size, separatorHandle);
    }

    PandaVector<char> buf(res.utf8Size + 1);
    PandaVector<uint8_t> tree8Buf;
    const size_t sepSize = separatorHandle->GetUtf8Length();
    uint8_t *separator = separatorHandle->IsTreeString() ? separatorHandle->GetTreeStringDataUtf8(tree8Buf)
                                                         : separatorHandle->GetDataUtf8();
    size_t pos = 0;
    for (size_t index = 0; index < actualLength - 1; index++) {
        auto *element = buffer->Get(index);
        ProcessUtf8Element(res, element, buf, pos);

        if (sepSize > 0) {
            MemcpyUnsafe(&buf[pos], separator, sepSize);
            pos += sepSize;
        }
    }

    auto *lastElement = buffer->Get(actualLength - 1);
    ProcessUtf8Element(res, lastElement, buf, pos);

    return EtsString::CreateFromUtf8(buf.data(), pos);
}

static void ProcessUtf16Element(ElementComputeResult &res, EtsObject *element, PandaVector<EtsChar> &outputBuffer,
                                size_t &pos)
{
    if (ets::EtsReferenceNullish(res.coro, element)) {
        return;
    }

    auto elementCls = element->GetClass();
    const auto &numericHandlerTable = GetNumericHandlerTable<PandaVector<EtsChar>>(res.ptypes);
    for (const auto &entry : numericHandlerTable) {
        if (entry.first == elementCls) {
            entry.second(element, outputBuffer, pos);
            return;
        }
    }

    const auto writeString = [&outputBuffer, &pos](EtsString *str) {
        ASSERT(str != nullptr);
        auto strSize = str->IsUtf16() ? str->GetUtf16Length() : str->GetUtf8Length();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        str->CopyDataUtf16(outputBuffer.data() + pos, strSize);
        pos += strSize;
    };

    if (elementCls->IsStringClass()) {
        auto *strElement = EtsString::FromEtsObject(element);
        writeString(strElement);
    } else if (elementCls == res.ptypes->coreChar) {
        auto value = GetNumericValue<EtsChar>(element);
        outputBuffer[pos] = value;
        pos++;
    } else if (elementCls == res.ptypes->coreBoolean) {
        auto value = GetNumericValue<EtsBoolean>(element);
        std::string_view text = (value == 1) ? TRUE_STRING : FALSE_STRING;
        std::copy(text.begin(), text.end(), outputBuffer.begin() + pos);
        pos += text.size();
    } else {
        ASSERT(res.toStringIndex < res.toStringResults.size());
        writeString(res.toStringResults[res.toStringIndex].GetPtr());
        res.toStringIndex++;
    }
}

// Might trigger GC
static ark::ets::EtsString *EtsEscompatArrayJoinUtf16(ElementComputeResult &res,
                                                      EtsHandle<EtsObjectArray> &bufferHandle, EtsInt actualLength,
                                                      EtsHandle<EtsString> &separatorHandle)
{
    size_t resultUtf16Size =
        res.utf16Size + res.utf8Size + res.intCount * MaxChars<EtsLong>() + res.doubleCount * MaxChars<EtsDouble>();
    PandaVector<EtsChar> buf(resultUtf16Size);
    auto sepSize = separatorHandle->IsUtf16() ? separatorHandle->GetUtf16Length() : separatorHandle->GetUtf8Length();
    size_t pos = 0;
    for (EtsInt index = 0; index < actualLength - 1; index++) {
        auto *element = bufferHandle->Get(index);
        ProcessUtf16Element(res, element, buf, pos);

        if (sepSize > 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            separatorHandle->CopyDataUtf16(buf.data() + pos, sepSize);
            pos += sepSize;
        }
    }

    auto *lastElement = bufferHandle->Get(actualLength - 1);
    ProcessUtf16Element(res, lastElement, buf, pos);

    return EtsString::CreateFromUtf16(buf.data(), pos);
}

extern "C" ark::ets::EtsString *EtsEscompatArrayJoinInternal(ObjectHeader *buffer, EtsInt actualLength,
                                                             ark::ets::EtsString *separator)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coro);

    EtsHandle<EtsString> separatorHandle(coro, separator);
    EtsHandle<EtsObjectArray> bufferHandle(coro, EtsObjectArray::FromCoreType(buffer));
    ASSERT(actualLength > 0);
    // Sanity check that actual length is less or equal to capacity
    ASSERT(static_cast<size_t>(actualLength) <= bufferHandle->GetLength());

    ElementComputeResult res(coro);

    if (UNLIKELY(!ComputeCharSize(res, bufferHandle, actualLength, separatorHandle))) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    ark::interpreter::RuntimeInterface::Safepoint();

    if (res.utf16Size > 0) {
        return EtsEscompatArrayJoinUtf16(res, bufferHandle, actualLength, separatorHandle);
    }

    return EtsEscompatArrayJoinUtf8(res, bufferHandle, actualLength, separatorHandle);
}

extern "C" EtsObject *EtsEscompatArrayGetUnsafe(EtsEscompatArray *array, int32_t index)
{
    return array->EscompatArrayGetUnsafe(index);
}

extern "C" void EtsEscompatArraySetUnsafe(EtsEscompatArray *array, int32_t index, EtsObject *value)
{
    array->EscompatArraySetUnsafe(index, value);
}

extern "C" void EtsEscompatArrayUnshiftInternal(ObjectHeader *arrayHeader, EtsInt arrayLen, ObjectHeader *bufferHeader,
                                                EtsEscompatArray *values)
{
    ASSERT(arrayHeader != nullptr);
    ASSERT(bufferHeader != nullptr);
    ASSERT(values != nullptr);
    auto *buffer = EtsArray::FromEtsObject(EtsObject::FromCoreType(bufferHeader));
    auto dstLen = buffer->GetLength() * sizeof(ObjectPointerType);
    auto *dst = buffer->GetData<ObjectPointerType>();
    auto *array = EtsArray::FromEtsObject(EtsObject::FromCoreType(arrayHeader));
    auto *arraySrc = array->GetData<ObjectPointerType>();
    auto arraySrcLen = arrayLen * sizeof(ObjectPointerType);
    auto valuesSrcLen = values->GetActualLengthFromEscompatArray() * sizeof(ObjectPointerType);
    [[maybe_unused]] auto error0 =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        memmove_s(dst + values->GetActualLengthFromEscompatArray(), dstLen - valuesSrcLen, arraySrc, arraySrcLen);
    ASSERT(error0 == 0);
    auto *valuesSrc = values->GetDataFromEscompatArray()->GetData<ObjectPointerType>();
    [[maybe_unused]] auto error1 = memmove_s(dst, dstLen, valuesSrc, valuesSrcLen);
    ASSERT(error1 == 0);

    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    auto *barrierSet = thread->GetBarrierSet();
    if (barrierSet != nullptr) {
        barrierSet->PostBarrier(buffer, EtsArray::GetDataOffset(), arraySrcLen + valuesSrcLen);
    }
}

extern "C" EtsBoolean EtsEscompatArrayIsPlatformArray(EtsObject *obj)
{
    return ToEtsBoolean(EtsEscompatArray::IsExactlyEscompatArray(obj, EtsCoroutine::GetCurrent()));
}

extern "C" ObjectHeader *EtsEscompatArrayGetBuffer(EtsObject *obj)
{
    ASSERT(EtsEscompatArray::IsExactlyEscompatArray(obj, EtsCoroutine::GetCurrent()));
    auto *array = EtsEscompatArray::FromEtsObject(obj);
    return array->GetDataFromEscompatArray()->GetCoreType();
}

}  // namespace ark::ets::intrinsics
