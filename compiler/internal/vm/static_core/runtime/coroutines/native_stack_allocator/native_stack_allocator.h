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

#ifndef RUNTIME_COROUTINES_NATIVE_STACK_ALLOCATOR_NATIVE_STACK_ALLOCATOR_H
#define RUNTIME_COROUTINES_NATIVE_STACK_ALLOCATOR_NATIVE_STACK_ALLOCATOR_H

#include "libarkbase/mem/mem_pool.h"
#include "libarkbase/os/mutex.h"

#include <bitset>

namespace ark {

/**
 * NativeStackAllocator is a class that allows you to allocate
 * and free a large chunk of memory that should normally be used for the stack.
 * NativeStackAllocator allows you to use fewer mmap calls to work with the same number of stacks.
 */
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class NativeStackAllocator {
public:
    static constexpr uint8_t STACK_COUNT_IN_POOL = 8U;
    // Check if STACK_COUNT_IN_POOL is divisible by 8
    // NOLINTNEXTLINE(hicpp-signed-bitwise, readability-magic-numbers)
    static_assert((STACK_COUNT_IN_POOL & 0xF) == STACK_COUNT_IN_POOL);

private:
    class StacksHolder {
    public:
        explicit StacksHolder(size_t stackSize);
        ~StacksHolder();
        NO_COPY_SEMANTIC(StacksHolder);
        NO_MOVE_SEMANTIC(StacksHolder);

        [[nodiscard]] uint8_t *AllocStack();
        bool TryFreeStack(uint8_t *stack);
        bool CheckIsFree() const;

        StacksHolder *next = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
        const size_t size;  // NOLINT(misc-non-private-member-variables-in-classes, readability-identifier-naming)
        std::bitset<STACK_COUNT_IN_POOL> bitset;  // NOLINT(misc-non-private-member-variables-in-classes)
        uint8_t *mem;                             // NOLINT(misc-non-private-member-variables-in-classes)
    };

public:
    NativeStackAllocator() = default;  // NOLINT(cppcoreguidelines-pro-type-member-init)
    ~NativeStackAllocator() = default;

    NO_COPY_SEMANTIC(NativeStackAllocator);
    NO_MOVE_SEMANTIC(NativeStackAllocator);

    /**
     * @brief Method inits usage of stack manager instance. To finish usage on stack manager use Finalize method.
     * @param stackSize: size of stack in future allocations.
     * @see Finalize()
     */
    void Initialize(size_t stackSize);
    /**
     * @brief Method finishs usage of stack manager instance.
     * All stacks should be freed before calling of the method.
     * @see ReleaseStack(...)
     */
    void Finalize();

    /**
     * @brief Method returns stack with size that was specified in Initialize(...) method.
     * @returns pointer to stack.
     * @see Initialize(...)
     */
    [[nodiscard]] uint8_t *AcquireStack();
    /**
     * @brief Method frees stack from this stack manager.
     * @oaram stack: pointer from this stack manager.
     * @see AcquireStack()
     */
    void ReleaseStack(uint8_t *stack);

private:
    [[nodiscard]] static StacksHolder *AllocHolder(size_t poolSize);
    static void FreeHolder(StacksHolder *holder, size_t poolSize);

    os::memory::Mutex mutex_;
    size_t poolSize_ GUARDED_BY(mutex_);
    StacksHolder *first_ GUARDED_BY(mutex_) = nullptr;
};

}  // namespace ark

#endif  // RUNTIME_COROUTINES_NATIVE_STACK_ALLOCTOR_NATIVE_STACK_ALLOCATOR
