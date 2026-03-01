/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <charconv>
#include "ets_class_linker_extension.h"
#include "ets_to_string_cache.h"
#include "ets_intrinsics_helpers.h"
#include "libarkbase/mem/mem.h"

namespace ark::ets::detail {

template <typename T>
class EtsToStringCacheElement : public EtsObject {
public:
    ~EtsToStringCacheElement() = default;
    NO_COPY_SEMANTIC(EtsToStringCacheElement);
    NO_MOVE_SEMANTIC(EtsToStringCacheElement);

    using Flag = ObjectPointerType;
    static constexpr Flag FLAG_MASK = 1U;

    static EtsClass *GetClass(EtsCoroutine *coro);

    static EtsToStringCacheElement<T> *Create(EtsCoroutine *coro, EtsHandle<EtsString> &stringHandle, T number,
                                              EtsClass *klass);

    static EtsToStringCacheElement<T> *FromCoreType(ObjectHeader *obj)
    {
        return reinterpret_cast<EtsToStringCacheElement<T> *>(obj);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    EtsObject *AsObject()
    {
        return this;
    }

    T LoadNumber() const
    {
        // Atomic with acquire order reason: make `flag` writes from other threads visible
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        return AtomicLoad(&number_, std::memory_order_acquire);
    }

    /*
     * `Data` contains pointer to managed `string` representing corresponding `number` and int `flag` used to provide
     * atomicity `flag` is incremented by 1 in the beginning and in the end of cache update, so:
     * - if we read odd `flag` from another thread, we consider the cache element "locked" and don't use it for
     * read/write
     * - if `flag` is different before and after read of `number`, the cache element is being updated, and we ignore it
     * `Data` is aligned by 8 bytes to read `string` and `flag` with a single atomic operation
     */
    struct alignas(alignof(uint64_t)) Data {
        ObjectPointer<EtsString> string {};
        Flag flag {};
    };
    static_assert(sizeof(Data) == sizeof(ObjectPointerType) * 2U);
    // NOTE(ipetrov): hack for 128 bit ObjectHeader
#if !defined(ARK_HYBRID) && defined(PANDA_32_BIT_MANAGED_POINTER) && defined(PANDA_TARGET_64)
    static_assert(std::atomic<Data>::is_always_lock_free);
#endif

    Data LoadData() const
    {
        // Atomic with acquire order reason: make `number` and `flag` writes from other threads visible
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        return AtomicLoad(&data_, std::memory_order_acquire);
    }

    static bool IsLocked(Flag flag)
    {
        return (flag & FLAG_MASK) != 0;
    }

    bool IsFresh(Flag flag) const
    {
        // Atomic with relaxed order reason: used only after acquire-loads
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        Flag newFlag = AtomicLoad(&data_.flag, std::memory_order_relaxed);
        return newFlag == flag;
    }

    /*
     * Reasoning on correctness:
     * If compare-exchange acq_rel w1 succeeds, than it sees result of w3 (from this or other writer thread),
     * because w1 makes `flag` even and `w3` makes it odd.
     * so ... w3 HB (w1 HB w2 HB w3) HB w1 ..., and there is a total order on all writes to `elem`
     */
    ToStringResult TryStore(EtsCoroutine *coro, EtsString *string, T number, Data oldData)
    {
        ASSERT(!IsLocked(oldData.flag));
        Data newData {string, oldData.flag + FLAG_MASK};
        ASSERT(coro != nullptr);
        auto *barrierSet = coro->GetBarrierSet();

        if (UNLIKELY(barrierSet->IsPreBarrierEnabled())) {
            auto *oldValue = ObjectAccessor::GetObject(this, STRING_OFFSET);
            barrierSet->PreBarrier(oldValue);
        }
        auto oldCopy = oldData;
        // Atomic with acquire order reason: sync flag
        while (
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            !AtomicCmpxchgWeak(&data_, oldData, newData, std::memory_order_acquire, std::memory_order_relaxed)) {  // w1
            if (oldCopy.string == oldData.string && oldCopy.flag == oldData.flag) {
                // spurious failure, try compare_exchange again
                continue;
            }
            return ToStringResult::STORE_FAIL;
        }
        // Atomic with release order reason: make flag update visible in other threads
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AtomicStore(&number_, number, std::memory_order_release);  // w2
        oldData = newData;
        newData.flag++;
        // Atomic with release order reason: number write must be visible after flag update
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        while (!AtomicCmpxchgWeak(&data_, oldData, newData, std::memory_order_release, std::memory_order_relaxed)) {
        }  // w3
        ASSERT(string != nullptr);
        if (!mem::IsEmptyBarrier(barrierSet->GetPostType())) {
            barrierSet->PostBarrier(this, STRING_OFFSET, string);
        }
        return ToStringResult::STORE_UPDATE;
    }

    constexpr static size_t GetUnalignedSize()
    {
        return NUMBER_OFFSET + sizeof(T);
    }

private:
    void SetString(EtsCoroutine *coro, EtsString *string);

    void SetNumber(T number)
    {
        // Atomic with relaxed order reason: used only on newly-created object before store-release into array
        ObjectAccessor::SetPrimitive(this, NUMBER_OFFSET, number);
    }

private:
    Data data_;

    T number_;
    constexpr static size_t STRING_OFFSET =
        MEMBER_OFFSET(EtsToStringCacheElement<T>, data_) + MEMBER_OFFSET(Data, string);
    [[maybe_unused]] constexpr static size_t FLAG_OFFSET =
        MEMBER_OFFSET(EtsToStringCacheElement<T>, data_) + MEMBER_OFFSET(Data, flag);
    constexpr static size_t NUMBER_OFFSET = MEMBER_OFFSET(EtsToStringCacheElement<T>, number_);

    friend class ark::ets::test::EtsToStringCacheTest;
};

/* static */
template <typename T>
EtsClass *EtsToStringCacheElement<T>::GetClass(EtsCoroutine *coro)
{
    auto *classLinker = coro->GetPandaVM()->GetClassLinker();
    auto *ext = classLinker->GetEtsClassLinkerExtension();

    std::string_view classDescriptor;
    if constexpr (std::is_same_v<T, EtsDouble>) {
        classDescriptor = panda_file_items::class_descriptors::DOUBLE_TO_STRING_CACHE_ELEMENT;
    } else if constexpr (std::is_same_v<T, EtsFloat>) {
        classDescriptor = panda_file_items::class_descriptors::FLOAT_TO_STRING_CACHE_ELEMENT;
    } else if constexpr (std::is_same_v<T, EtsLong>) {
        classDescriptor = panda_file_items::class_descriptors::LONG_TO_STRING_CACHE_ELEMENT;
    } else {
        UNREACHABLE();
    }
    return classLinker->GetClass(classDescriptor.data(), false, ext->GetBootContext());
}

/* static */
template <typename T>
EtsToStringCacheElement<T> *EtsToStringCacheElement<T>::Create(EtsCoroutine *coro, EtsHandle<EtsString> &stringHandle,
                                                               T number, EtsClass *klass)
{
    static_assert(STRING_OFFSET == sizeof(ObjectPointerType) * 2U);
    static_assert(NUMBER_OFFSET == sizeof(ObjectPointerType) * 2U + sizeof(Data));
    auto *instance = FromCoreType(EtsObject::Create(coro, klass)->GetCoreType());
    instance->SetString(coro, stringHandle.GetPtr());
    instance->SetNumber(number);
    return instance;
}

template <typename T>
void EtsToStringCacheElement<T>::SetString(EtsCoroutine *coro, EtsString *string)
{
    static_assert(STRING_OFFSET == sizeof(ObjectPointerType) * 2U);
    ASSERT(string != nullptr);
    // Atomic with relaxed order reason: used only on newly-created object before store-release into array
    ObjectAccessor::SetObject(coro, this, STRING_OFFSET, string->GetCoreType());
}

template <typename T>
EtsString *ToString(T number)
{
    if constexpr (std::is_same_v<T, EtsLong>) {
        // 1 for first digit and 1 for zero
        constexpr auto MAX_DIGITS = std::numeric_limits<int64_t>::digits10 + 2U;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        std::array<char, MAX_DIGITS + 1> buf;
        auto [stringEnd, result] = std::to_chars(buf.begin(), buf.end(), number);
        ASSERT(result == std::errc());
        return EtsString::CreateFromAscii(buf.begin(), stringEnd - buf.begin());
    } else {
        static_assert(std::is_floating_point_v<T>);
        return intrinsics::helpers::FpToStringDecimalRadix(
            number, [](std::string_view str) { return EtsString::CreateFromAscii(str.data(), str.length()); });
    }
}

template <typename T>
struct SimpleHash {
    static constexpr uint32_t CACHE_SIZE_SHIFT = 8U;
    static constexpr uint32_t T_BITS = sizeof(T) * BITS_PER_BYTE;
    using UnsignedIntType = ark::helpers::TypeHelperT<T_BITS, false>;
    static constexpr uint32_t SHIFT = T_BITS - CACHE_SIZE_SHIFT;

    static constexpr UnsignedIntType GetMul()
    {
        if constexpr (std::is_same_v<UnsignedIntType, uint32_t>) {
            constexpr auto MUL = 0x5bd1e995U;
            return MUL;
        } else {
            constexpr auto MUL = 0xc6a4a7935bd1e995ULL;
            return MUL;
        }
    }

    size_t operator()(T number) const
    {
        auto res = (bit_cast<UnsignedIntType>(number) * GetMul()) >> SHIFT;
        ASSERT(res < (1U << CACHE_SIZE_SHIFT));
        return static_cast<size_t>(res);
    }
};

/* static */
template <typename T, typename Derived, typename Hash>
uint32_t EtsToStringCache<T, Derived, Hash>::GetIndex(T number)
{
    auto index = Hash()(number);
    ASSERT(index < SIZE);
    return index;
}

template <typename T, typename Derived, typename Hash>
std::pair<EtsString *, ToStringResult> EtsToStringCache<T, Derived, Hash>::FinishUpdate(
    EtsCoroutine *coro, T number, EtsToStringCacheElement<T> *elem, uint64_t cachedAsInt)
{
    // NOTE(ipetrov): hack for 128 bit ObjectHeader
#if !defined(PANDA_32_BIT_MANAGED_POINTER)
    UNUSED_VAR(coro);
    UNUSED_VAR(elem);
    UNUSED_VAR(cachedAsInt);
    return {ToString(number), ToStringResult::STORE_UPDATE};
#else
    [[maybe_unused]] EtsHandleScope scope(coro);
    EtsHandle<EtsToStringCacheElement<T>> elemHandle(coro, elem);
    // may trigger GC
    auto *string = ToString(number);
    ASSERT(string != nullptr);
    auto *cached = reinterpret_cast<typename EtsToStringCacheElement<T>::Data *>(&cachedAsInt);
    ASSERT(elemHandle.GetPtr() != nullptr);
    auto storeRes = elemHandle->TryStore(coro, string, number, *cached);
    return {string, storeRes};
#endif
}

// Reasoning on correctness:
// If we return cached `number`, r1 and r3 saw the same `flag` stored by `w1` or `w3` (I) (or initial `0` value as
// corner-case) It cannot be `w1`, because `w1` stores odd flag, and we have read even flag. So, `w2` HB `w3`
// HB(rel-acq) `r1` HB `r2` HB `r3`, and `w2` HB `r2`.
//    Consider the case when `r2` sees some later write to number, `w2a`. Then `w1a` HB `w2a` HB(rel-acq) `r2` HB `r3`,
//    and `r3` sees `w1a` or some later write to `flag`, which leads to contradiction with (I). (we consider equality
//    after overflow impossible, because in this case writer threads do ~2^32 operations while the reader does nothing).
//    So `r2` sees number from `w2`.
// `r1` sees string from `w1` because of (I), so we prove that string and number in successful cache read correspond to
// each other.
//
// In case of cache miss which leads to store, ordering of `r2` and `r3` does not matter because we only use `r1` and
// pass it to compare-exchange.
template <typename T, typename Derived, typename Hash>
std::pair<EtsString *, ToStringResult> EtsToStringCache<T, Derived, Hash>::GetOrCacheImpl(EtsCoroutine *coro, T number)
{
    // NOTE(ipetrov): hack for 128 bit ObjectHeader
#if !defined(PANDA_32_BIT_MANAGED_POINTER)
    UNUSED_VAR(coro);
    return {ToString(number), ToStringResult::STORE_NEW};
#else
    EVENT_ETS_CACHE("Fastpath: create string from number with cache");
    auto index = GetIndex(number);
    // Atomic with acquire order reason: write of `elem` to array is dependency-ordered before reads from loaded `elem`
    auto *elem = Base::Get(index, std::memory_order_acquire);
    if (UNLIKELY(elem == nullptr)) {
        [[maybe_unused]] EtsHandleScope scope(coro);
        EtsHandle<EtsString> string(coro, ToString(number));
        ASSERT(string.GetPtr() != nullptr);
        // may trigger GC
        StoreToCache(coro, string, number, index);
        ASSERT(string.GetPtr() != nullptr);
        return {string.GetPtr(), ToStringResult::STORE_NEW};
    }
    auto cached = elem->LoadData();  // r1
    if (UNLIKELY(Elem::IsLocked(cached.flag))) {
        return {ToString(number), ToStringResult::LOAD_FAIL_LOCKED};
    }
    auto cachedNumber = elem->LoadNumber();  // r2
    if (LIKELY(cachedNumber == number)) {
        if (LIKELY(elem->IsFresh(cached.flag))) {  // r3
            return {cached.string, ToStringResult::LOAD_CACHED};
        }
        return {ToString(number), ToStringResult::LOAD_FAIL_UPDATED};
    }
    return FinishUpdate(coro, number, elem, bit_cast<uint64_t>(cached));
#endif
}

template <typename T, typename Derived, typename Hash>
EtsString *EtsToStringCache<T, Derived, Hash>::CacheAndGetNoCheck(EtsCoroutine *coro, T number, ObjectHeader *elem,
                                                                  uint64_t cached)
{
    ASSERT(elem != nullptr);
    EVENT_ETS_CACHE("Fastpath: create string from number with cache");
    return FinishUpdate(coro, number, EtsToStringCacheElement<T>::FromCoreType(elem), cached).first;
}

/* static */
template <typename T, typename Derived, typename Hash>
EtsString *EtsToStringCache<T, Derived, Hash>::GetNoCache(T number)
{
    EVENT_ETS_CACHE("Slowpath: create string from number without cache");
    return ToString(number);
}

/* static */
template <typename T, typename Derived, typename Hash>
Derived *EtsToStringCache<T, Derived, Hash>::Create(EtsCoroutine *coro)
{
    auto *etsClass = detail::EtsToStringCacheElement<T>::GetClass(coro);
    if (etsClass == nullptr) {
        return nullptr;
    }
    return static_cast<Derived *>(Base::Create(etsClass, SIZE, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT));
}

template <typename T, typename Derived, typename Hash>
void EtsToStringCache<T, Derived, Hash>::StoreToCache(EtsCoroutine *coro, EtsHandle<EtsString> &stringHandle, T number,
                                                      uint32_t index)
{
    auto *arrayClass = Base::GetCoreType()->template ClassAddr<Class>();
    auto *elemClass = arrayClass->GetComponentType();
    ASSERT(elemClass->GetObjectSize() == EtsToStringCacheElement<T>::GetUnalignedSize());
    auto *elem = EtsToStringCacheElement<T>::Create(coro, stringHandle, number, EtsClass::FromRuntimeClass(elemClass));
    // Atomic with release order reason: writes to elem should be visible in other threads
    Base::Set(index, elem, std::memory_order_release);
}

template class EtsToStringCache<EtsDouble, DoubleToStringCache>;
template class EtsToStringCache<EtsFloat, FloatToStringCache>;
template class EtsToStringCache<EtsLong, LongToStringCache>;

}  // namespace ark::ets::detail
