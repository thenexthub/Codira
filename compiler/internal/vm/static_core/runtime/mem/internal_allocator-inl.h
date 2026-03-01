/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_INTERNAL_ALLOCATOR_INL_H
#define PANDA_RUNTIME_MEM_INTERNAL_ALLOCATOR_INL_H

#include "runtime/mem/malloc-proxy-allocator-inl.h"
#include "runtime/mem/freelist_allocator-inl.h"
#include "runtime/mem/humongous_obj_allocator-inl.h"
#include "runtime/mem/internal_allocator.h"
#include "runtime/mem/runslots_allocator-inl.h"

namespace ark::mem {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_INTERNAL_ALLOCATOR(level) LOG(level, ALLOC) << "InternalAllocator: "

template <InternalAllocatorConfig CONFIG>
template <class T>
T *InternalAllocator<CONFIG>::AllocArray(size_t size)
{
    return static_cast<T *>(this->Alloc(sizeof(T) * size, GetAlignment<T>()));
}

template <InternalAllocatorConfig CONFIG>
template <class T>
T *InternalAllocator<CONFIG>::AllocArrayLocal(size_t size)
{
    return static_cast<T *>(this->AllocLocal(sizeof(T) * size, GetAlignment<T>()));
}

template <InternalAllocatorConfig CONFIG>
template <typename T, typename... Args>
std::enable_if_t<!std::is_array_v<T>, T *> InternalAllocator<CONFIG>::New(Args &&...args)
{
    void *p = Alloc(sizeof(T), GetAlignment<T>());
    if (UNLIKELY(p == nullptr)) {
        return nullptr;
    }
    new (p) T(std::forward<Args>(args)...);
    return reinterpret_cast<T *>(p);
}

template <InternalAllocatorConfig CONFIG>
template <typename T>
std::enable_if_t<is_unbounded_array_v<T>, std::remove_extent_t<T> *> InternalAllocator<CONFIG>::New(size_t size)
{
    static constexpr size_t SIZE_BEFORE_DATA_OFFSET = AlignUp(sizeof(size_t), GetAlignmentInBytes(GetAlignment<T>()));
    using ElementType = std::remove_extent_t<T>;
    void *p = Alloc(SIZE_BEFORE_DATA_OFFSET + sizeof(ElementType) * size, GetAlignment<T>());
    *static_cast<size_t *>(p) = size;
    ElementType *data = ToNativePtr<ElementType>(ToUintPtr(p) + SIZE_BEFORE_DATA_OFFSET);
    ElementType *currentElement = data;
    for (size_t i = 0; i < size; ++i, ++currentElement) {
        new (currentElement) ElementType();
    }
    return data;
}

template <InternalAllocatorConfig CONFIG>
template <class T>
void InternalAllocator<CONFIG>::Delete(T *ptr)
{
    if constexpr (std::is_class_v<T>) {
        ptr->~T();
    }
    Free(ptr);
}

template <InternalAllocatorConfig CONFIG>
template <typename T>
void InternalAllocator<CONFIG>::DeleteArray(T *data)
{
    static constexpr size_t SIZE_BEFORE_DATA_OFFSET = AlignUp(sizeof(size_t), GetAlignmentInBytes(GetAlignment<T>()));
    void *p = ToVoidPtr(ToUintPtr(data) - SIZE_BEFORE_DATA_OFFSET);
    size_t size = *static_cast<size_t *>(p);
    if constexpr (std::is_class_v<T>) {
        for (size_t i = 0; i < size; ++i, ++data) {
            data->~T();
        }
    }
    Free(p);
}

template <InternalAllocatorConfig CONFIG>
template <typename MemVisitor>
void InternalAllocator<CONFIG>::VisitAndRemoveAllPools(MemVisitor memVisitor)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        runslotsAllocator_->VisitAndRemoveAllPools(memVisitor);
        freelistAllocator_->VisitAndRemoveAllPools(memVisitor);
        humongousAllocator_->VisitAndRemoveAllPools(memVisitor);
    }
}

template <InternalAllocatorConfig CONFIG>
template <typename MemVisitor>
void InternalAllocator<CONFIG>::VisitAndRemoveFreePools(MemVisitor memVisitor)
{
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CONFIG == InternalAllocatorConfig::PANDA_ALLOCATORS) {
        runslotsAllocator_->VisitAndRemoveFreePools(memVisitor);
        freelistAllocator_->VisitAndRemoveFreePools(memVisitor);
        humongousAllocator_->VisitAndRemoveFreePools(memVisitor);
    }
}

#undef LOG_INTERNAL_ALLOCATOR

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_INTERNAL_ALLOCATOR_H
