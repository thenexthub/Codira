/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "ets_class_root.h"
#include "ets_vm.h"
#include "intrinsics.h"
#include "interpreter/runtime_interface.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_language_context.h"
#include "types/ets_array.h"

namespace ark::ets::intrinsics {

static const char *CheckCopyToPreConditions(int32_t srcStart, int32_t srcLen, int32_t srcEnd, int32_t dstStart,
                                            int32_t dstLen)
{
    if (srcStart < 0 || srcStart > srcEnd || srcEnd > srcLen) {
        return "copyTo: src bounds verification failed";
    }
    if (dstStart < 0 || dstStart > dstLen) {
        return "copyTo: dst bounds verification failed";
    }
    if ((srcEnd - srcStart) > (dstLen - dstStart)) {
        return "copyTo: Destination array doesn't have enough space";
    }
    return nullptr;
}

template <typename T>
static void StdCoreCopyTo(coretypes::Array *src, coretypes::Array *dst, int32_t dstStart, int32_t srcStart,
                          int32_t srcEnd)
{
    auto srcLen = static_cast<int32_t>(src->GetLength());
    auto dstLen = static_cast<int32_t>(dst->GetLength());
    const char *errmsg = CheckCopyToPreConditions(srcStart, srcLen, srcEnd, dstStart, dstLen);

    if (errmsg == nullptr) {
        auto srcAddr = ToVoidPtr(ToUintPtr(src->GetData()) + srcStart * sizeof(T));
        auto dstAddr = ToVoidPtr(ToUintPtr(dst->GetData()) + dstStart * sizeof(T));
        auto size = static_cast<size_t>((srcEnd - srcStart) * sizeof(T));
        if (size != 0) {  // cannot be less than zero at this point
            if (memmove_s(dstAddr, size, srcAddr, size) != EOK) {
                errmsg = "copyTo: copying data failed";
            }
        }
    }

    if (errmsg != nullptr) {
        auto *coroutine = EtsCoroutine::GetCurrent();
        ThrowEtsException(coroutine, panda_file_items::class_descriptors::ARRAY_INDEX_OUT_OF_BOUNDS_ERROR, errmsg);
    }
}

extern "C" ObjectHeader *StdCoreAllocGenericArray(EtsInt len, EtsObject *sample)
{
    EtsClass *klass;
    if (sample != nullptr) {
        klass = sample->GetClass()->GetComponentType();
    } else {
        klass = PandaEtsVM::GetCurrent()->GetClassLinker()->GetClassRoot(EtsClassRoot::OBJECT);
    }
    auto *array = EtsObjectArray::Create(klass, len);
    ASSERT(array != nullptr);
    return array->GetCoreType();
}

extern "C" void StdCoreBoolCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                  int32_t srcEnd)
{
    StdCoreCopyTo<uint8_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreCharCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                  int32_t srcEnd)
{
    StdCoreCopyTo<uint16_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreShortCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                   int32_t srcEnd)
{
    StdCoreCopyTo<uint16_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreByteCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                  int32_t srcEnd)
{
    StdCoreCopyTo<uint8_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreIntCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                 int32_t srcEnd)
{
    StdCoreCopyTo<uint32_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreLongCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                  int32_t srcEnd)
{
    StdCoreCopyTo<uint64_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreFloatCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                   int32_t srcEnd)
{
    StdCoreCopyTo<uint32_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

extern "C" void StdCoreDoubleCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                    int32_t srcEnd)
{
    StdCoreCopyTo<uint64_t>(src->GetCoreType(), dst->GetCoreType(), dstStart, srcStart, srcEnd);
}

template <typename T>
static void MemAtomicCopy(const void *srcAddr, void *dstAddr)
{
    auto src = reinterpret_cast<const std::atomic<T> *>(srcAddr);
    auto dst = reinterpret_cast<std::atomic<T> *>(dstAddr);
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    dst->store(src->load(std::memory_order_relaxed), std::memory_order_relaxed);
}

template <typename T>
static void MemAtomicCopyReadBarrier(mem::GCBarrierSet *barrierSet, const void *srcStart, const void *srcAddr,
                                     void *dstAddr)
{
    auto *dst = reinterpret_cast<std::atomic<T> *>(dstAddr);
    auto *src = reinterpret_cast<const std::atomic<T> *>(barrierSet->PreReadBarrier(
        srcStart, reinterpret_cast<const uint8_t *>(srcAddr) - reinterpret_cast<const uint8_t *>(srcStart)));
    // Atomic with relaxed order reason: use the relaxed memory order in hope the GC takes care
    // of the memory ordering at a higher logical level
    dst->store(src->load(std::memory_order_relaxed), std::memory_order_relaxed);
}

template <typename T>
static auto GetCopy([[maybe_unused]] const void *srcAddr)
{
#ifdef ARK_HYBRID
    auto *readBarrierSet = Thread::GetCurrent()->GetBarrierSet();
    bool usePreReadBarrier = readBarrierSet->IsPreReadBarrierEnabled();

    return [usePreReadBarrier, readBarrierSet, srcAddr](T *srcPtr, T *dstPtr) {
        if (usePreReadBarrier) {
            MemAtomicCopyReadBarrier<T>(readBarrierSet, srcAddr, srcPtr, dstPtr);
        } else {
            MemAtomicCopy<T>(srcPtr, dstPtr);
        }
    };
#else
    return [](T *srcPtr, T *dstPtr) { MemAtomicCopy<T>(srcPtr, dstPtr); };
#endif
}

namespace {
template <typename T>
struct ObjectArrayHandle final {
    EtsHandle<EtsCharArray> &array;
    int32_t start;

    void *GetBasePtr()
    {
        return array.GetPtr();
    }

    [[nodiscard]] static size_t GetElementOffset(size_t index)
    {
        constexpr uint32_t OFFSET = ark::coretypes::Array::GetDataOffset();
        return OFFSET + index * sizeof(T);
    }

    T *GetStartPtr()
    {
        return static_cast<T *>(ToVoidPtr(ToUintPtr(array->GetCoreType()->GetData()) + start * sizeof(T)));
    }
};
}  // namespace

// Performance measurement on the device demonstrates that COPIED_BETWEEN_SAFEPOINT_THRESHOLD object pointers
// are copied in 0.07-0.1 ms.
static constexpr size_t COPIED_BETWEEN_SAFEPOINT_THRESHOLD = 100000UL;

template <typename T, typename Copy, typename PutSP>
static void CopyForward(T *src, T *dst, size_t length, Copy copy, PutSP putSafepoint)
{
    for (size_t i = 0; i < length / COPIED_BETWEEN_SAFEPOINT_THRESHOLD; ++i) {
        for (size_t j = i * COPIED_BETWEEN_SAFEPOINT_THRESHOLD; j < (i + 1) * COPIED_BETWEEN_SAFEPOINT_THRESHOLD; ++j) {
            copy(&src[j], &dst[j]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        putSafepoint(src, dst, i * COPIED_BETWEEN_SAFEPOINT_THRESHOLD, COPIED_BETWEEN_SAFEPOINT_THRESHOLD);
    }
    size_t batchesFinalIdx = (length / COPIED_BETWEEN_SAFEPOINT_THRESHOLD) * COPIED_BETWEEN_SAFEPOINT_THRESHOLD;
    for (size_t i = batchesFinalIdx; i < length; ++i) {
        copy(&src[i], &dst[i]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    putSafepoint(src, dst, batchesFinalIdx, length - batchesFinalIdx);
}

template <typename T, typename Copy, typename PutSP>
static void CopyBackward(T *src, T *dst, size_t length, Copy copy, PutSP putSafepoint)
{
    size_t batchesStartIdx = (length / COPIED_BETWEEN_SAFEPOINT_THRESHOLD) * COPIED_BETWEEN_SAFEPOINT_THRESHOLD;
    for (size_t i = length; i > batchesStartIdx; --i) {
        copy(&src[i - 1], &dst[i - 1]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    putSafepoint(src, dst, batchesStartIdx, length - batchesStartIdx);
    for (size_t i = length / COPIED_BETWEEN_SAFEPOINT_THRESHOLD; i > 0; --i) {
        for (size_t j = i * COPIED_BETWEEN_SAFEPOINT_THRESHOLD; j > (i - 1) * COPIED_BETWEEN_SAFEPOINT_THRESHOLD; --j) {
            copy(&src[j - 1], &dst[j - 1]);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        putSafepoint(src, dst, (i - 1) * COPIED_BETWEEN_SAFEPOINT_THRESHOLD, COPIED_BETWEEN_SAFEPOINT_THRESHOLD);
    }
}

template <typename T>
static void PostWrite(ManagedThread *thread, ObjectArrayHandle<T> dstArray, size_t start, size_t length)
{
    auto *barrierSet = thread->GetBarrierSet();
    if (barrierSet->GetPostType() != ark::mem::BarrierType::POST_WRB_NONE) {
        const uint32_t size = length * sizeof(T);
        barrierSet->PostBarrier(dstArray.GetBasePtr(), ObjectArrayHandle<T>::GetElementOffset(start), size);
    }
}

template <typename T, bool NEED_PRE_WRITE_BARRIER = true>
static void RefCopy(ManagedThread *thread, ObjectArrayHandle<T> srcArray, ObjectArrayHandle<T> dstArray, int32_t length)
{
    auto *src = srcArray.GetStartPtr();
    auto *dst = dstArray.GetStartPtr();
    bool backwards = reinterpret_cast<uintptr_t>(src) < reinterpret_cast<uintptr_t>(dst);
    auto copy = GetCopy<T>(src);

    if constexpr (NEED_PRE_WRITE_BARRIER) {
        auto *barrierSet = thread->GetBarrierSet();
        bool usePreBarrier = barrierSet->IsPreBarrierEnabled();
        auto copyWithBarriers = [&copy, &usePreBarrier, barrierSet](T *srcPtr, T *dstPtr) {
            if (usePreBarrier) {
                barrierSet->PreBarrier(reinterpret_cast<void *>(*dstPtr));
            }
            copy(srcPtr, dstPtr);
        };
        auto putSafepoint = [&usePreBarrier, barrierSet, srcArray, dstArray,
                             thread](T *&srcPtr, T *&dstPtr, size_t start, size_t count) mutable {
            PostWrite<T>(thread, dstArray, start, count);
            ark::interpreter::RuntimeInterface::Safepoint(thread);
            usePreBarrier = barrierSet->IsPreBarrierEnabled();
            // If GC suspends worker during RefCopy execution, it may move the arrays pointed by srcPtr and dstPtr
            // to different memory locations; therefore, the new array addresses should be re-read.
            srcPtr = srcArray.GetStartPtr();
            dstPtr = dstArray.GetStartPtr();
        };
        if (backwards) {
            CopyBackward(src, dst, length, copyWithBarriers, putSafepoint);
        } else {
            CopyForward(src, dst, length, copyWithBarriers, putSafepoint);
        }
    } else {
        auto putSafepoint = [srcArray, dstArray, thread](T *&srcPtr, T *&dstPtr, size_t start, size_t count) mutable {
            PostWrite<T>(thread, dstArray, start, count);
            ark::interpreter::RuntimeInterface::Safepoint(thread);
            // If GC suspends worker during RefCopy execution, it may move the arrays pointed by srcPtr and dstPtr
            // to different memory locations; therefore, the new array addresses should be re-read.
            srcPtr = srcArray.GetStartPtr();
            dstPtr = dstArray.GetStartPtr();
        };
        if (backwards) {
            CopyBackward(src, dst, length, copy, putSafepoint);
        } else {
            CopyForward(src, dst, length, copy, putSafepoint);
        }
    }
}

template <bool NEED_PRE_WRITE_BARRIER = true, bool CHECKED = true>
void StdCoreCopyToForObjects(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart, int32_t srcEnd)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    auto srcArray = EtsHandle(coroutine, src);
    auto dstArray = EtsHandle(coroutine, dst);

    auto srcLen = static_cast<int32_t>(srcArray->GetLength());
    auto dstLen = static_cast<int32_t>(dstArray->GetLength());
    auto length = srcEnd - srcStart;
    if constexpr (CHECKED) {
        const char *errmsg = CheckCopyToPreConditions(srcStart, srcLen, srcEnd, dstStart, dstLen);
        if (errmsg != nullptr) {
            ThrowEtsException(coroutine, panda_file_items::class_descriptors::ARRAY_INDEX_OUT_OF_BOUNDS_ERROR, errmsg);
            return;
        }
    }

    /* the result will be exactly the same as it is */
    if (length == 0 || (srcArray.GetPtr() == dstArray.GetPtr() && srcStart == dstStart)) {
        return;
    }

    RefCopy<ObjectPointerType, NEED_PRE_WRITE_BARRIER>(coroutine, {srcArray, srcStart}, {dstArray, dstStart}, length);
}

extern "C" void StdCoreObjectCopyTo(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart, int32_t srcStart,
                                    int32_t srcEnd)
{
    StdCoreCopyToForObjects<true>(src, dst, dstStart, srcStart, srcEnd);
}

extern "C" void EscompatObjectFastCopyToUnchecked(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart,
                                                  int32_t srcStart, int32_t srcEnd)
{
    StdCoreCopyToForObjects<false, false>(src, dst, dstStart, srcStart, srcEnd);
}

extern "C" void EscompatObjectFastCopyToUncheckedWithBarriers(EtsCharArray *src, EtsCharArray *dst, int32_t dstStart,
                                                              int32_t srcStart, int32_t srcEnd)
{
    StdCoreCopyToForObjects<true, false>(src, dst, dstStart, srcStart, srcEnd);
}

extern "C" void CompilerMemmoveUnchecked(ark::ObjectHeader *src, ark::ObjectHeader *dst, int32_t dstStart,
                                         int32_t srcStart, int32_t srcEnd)
{
    auto *srcArr = EtsCharArray::FromEtsObject(EtsObject::FromCoreType(src));
    auto *dstArr = EtsCharArray::FromEtsObject(EtsObject::FromCoreType(dst));
    StdCoreCopyToForObjects<true, false>(srcArr, dstArr, dstStart, srcStart, srcEnd);
}

}  // namespace ark::ets::intrinsics
