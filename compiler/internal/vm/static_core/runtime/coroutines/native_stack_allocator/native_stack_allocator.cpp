/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "runtime/coroutines/native_stack_allocator/native_stack_allocator.h"
#include "include/object_header.h"

namespace ark {

NativeStackAllocator::StacksHolder *NativeStackAllocator::AllocHolder(size_t poolSize)
{
    Pool stackPool = PoolManager::GetMmapMemPool()->AllocPool<OSPagesAllocPolicy::NO_POLICY>(
        AlignUp(sizeof(StacksHolder) + poolSize, PANDA_POOL_ALIGNMENT_IN_BYTES), SpaceType::SPACE_TYPE_NATIVE_STACKS,
        AllocatorType::NATIVE_STACKS_ALLOCATOR);
    if (stackPool.GetMem() == nullptr) {
        return nullptr;
    }
    return new (stackPool.GetMem()) StacksHolder(poolSize);
}

void NativeStackAllocator::FreeHolder(StacksHolder *holder, size_t poolSize)
{
    PoolManager::GetMmapMemPool()->FreePool(holder,
                                            AlignUp(sizeof(StacksHolder) + poolSize, PANDA_POOL_ALIGNMENT_IN_BYTES));
}

void NativeStackAllocator::Initialize(size_t stackSize)
{
    os::memory::LockHolder lh(mutex_);
    poolSize_ = stackSize * STACK_COUNT_IN_POOL;
    first_ = AllocHolder(poolSize_);
    ASSERT(first_ != nullptr);
    first_->next = nullptr;
}

void NativeStackAllocator::Finalize()
{
    os::memory::LockHolder lh(mutex_);
    if (first_ != nullptr) {
        // Check if NativeStackAllocator have only one holder
        ASSERT(first_->next == nullptr);
        // Check if holder is totally free
        ASSERT(first_->bitset.none());
        FreeHolder(first_, poolSize_);
    }
}

uint8_t *NativeStackAllocator::AcquireStack()
{
    os::memory::LockHolder lh(mutex_);
    StacksHolder *current = first_;
    StacksHolder *prev = nullptr;
    do {
        auto *stack = current->AllocStack();
        if (stack != nullptr) {
            return stack;
        }
        prev = current;
        current = current->next;
    } while (current != nullptr);
    prev->next = AllocHolder(poolSize_);
    if (prev->next == nullptr) {
        return nullptr;
    }
    prev->next->next = nullptr;
    auto *stack = prev->next->AllocStack();
    ASSERT(stack != nullptr);
    return stack;
}

void NativeStackAllocator::ReleaseStack(uint8_t *stack)
{
    os::memory::LockHolder lh(mutex_);
    StacksHolder *current = first_;
    auto *next = current->next;
    if (current->TryFreeStack(stack)) {
        if (current->CheckIsFree() && next != nullptr) {
            first_ = next;
            FreeHolder(current, poolSize_);
        }
        return;
    }
    while (next != nullptr) {
        if (next->TryFreeStack(stack)) {
            if (next->CheckIsFree()) {
                current->next = next->next;
                FreeHolder(next, poolSize_);
            }
            return;
        }
        current = next;
        next = next->next;
    }
    UNREACHABLE();
}

bool NativeStackAllocator::StacksHolder::CheckIsFree() const
{
    return bitset.none();
}

uint8_t *NativeStackAllocator::StacksHolder::AllocStack()
{
    for (int i = 0; i < STACK_COUNT_IN_POOL; i++) {
        if (!bitset[i]) {
            bitset[i] = true;
            return mem + (size / STACK_COUNT_IN_POOL) * i;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
    }
    return nullptr;
}

// NOLINTNEXTLINE(readability-non-const-parameter)
bool NativeStackAllocator::StacksHolder::TryFreeStack(uint8_t *stack)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    if (stack < mem || stack >= (mem + size)) {
        return false;
    }
    auto stackSize = size / NativeStackAllocator::STACK_COUNT_IN_POOL;
    auto iter = (stack - mem) / stackSize;
    ASSERT(bitset[iter]);
    bitset[iter] = false;
    return true;
}

NativeStackAllocator::StacksHolder::StacksHolder(size_t stackSize)
    : size(stackSize),
      mem(reinterpret_cast<uint8_t *>(
          AlignUp(sizeof(StacksHolder) + reinterpret_cast<uint64_t>(this), os::mem::GetPageSize())))
{
}

NativeStackAllocator::StacksHolder::~StacksHolder()
{
    ASSERT(CheckIsFree());
}

}  // namespace ark
