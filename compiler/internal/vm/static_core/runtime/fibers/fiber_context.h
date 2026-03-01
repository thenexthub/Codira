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

#ifndef PANDA_RUNTIME_FIBERS_FIBER_CONTEXT_H_
#define PANDA_RUNTIME_FIBERS_FIBER_CONTEXT_H_

#if defined(PANDA_TARGET_ARM64)
#include "runtime/fibers/arch/aarch64/context_layout.h"
#elif defined(PANDA_TARGET_ARM32)
#include "runtime/fibers/arch/arm/context_layout.h"
#elif defined(PANDA_TARGET_X86)
#error "Unsupported target"
#elif defined(PANDA_TARGET_AMD64)
#include "runtime/fibers/arch/amd64/context_layout.h"
#else
#error "Unsupported target"
#endif

#include "libarkbase/macros.h"

#include <cstddef>
#include <cstdint>

namespace ark::fibers {

/**
 * This set of functions implements user mode context switch functionality.
 * The primary (but not the only one) use case for that is the lightweight user mode threads ("fibers") support.
 *
 * These functions allow the user to:
 * - retrieve current context
 * - switch back to it
 * - update the saved context with a new stack (allocated manually elsewhere) and an entrypoint
 * None of these functions performs any system calls to the OS kernel.
 */

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
using FiberContext = uint8_t[FCTX_LEN_BYTES];
using FiberEntry = void (*)(void *);

/// @brief Saves current usermode context to the provided buffer in memory.
extern "C" int GetCurrentContext(FiberContext *ctx);

/**
 * @brief Saves current context and switches to the target one.
 * @param from the buffer to save the current context in
 * @param to the buffer to load the new context from
 */
extern "C" int SwitchContext(FiberContext *from, const FiberContext *to);

/**
 * @brief Updates the previously saved context with a new stack and a new entry point.
 *
 * @param ctx a buffer with a valid saved context (via GetCurrentContext() or SwitchContext())
 * @param func the new entry point. After a context switch to ctx with SwitchContext(), the control transfers
 * to the beginning of func. Upon completion, func should use SwitchContext() to transfer control further.
 * If func, instead of that, does a simple return, this counts as an abnormal EP termination and the main program
 * is terminated with an error.
 * @param argument an optional argument to be passed to the func. Can be nullptr if needed. Might be a pointer to some
 * structure or class that represents the fiber instance.
 * @param stack a pointer to the stack to be used by the func. Should point to a preallocated buffer capable of holding
 * at least stack_size_bytes bytes. Allocation and deallocation of this buffer is the caller's responsibility.
 * @param stack_size_bytes minimal size of the stack provided
 */
extern "C" int UpdateContext(FiberContext *ctx, FiberEntry func, void *argument, uint8_t *stack, size_t stackSizeBytes);

/**
 * @brief Updates the previously saved context with a new entry point, keeping the original stack.
 *
 * Effectively emulates a function call, so EP's return address is the original PC of the saved context.
 */
extern "C" int UpdateContextKeepStack(FiberContext *ctx, FiberEntry func, void *argument);

/// @brief Copies the context stored in @param src to @param dst
extern "C" PANDA_PUBLIC_API void CopyContext(FiberContext *dst, const FiberContext *src);

}  // namespace ark::fibers

#endif /* PANDA_RUNTIME_FIBERS_FIBER_CONTEXT_H_ */
