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

#ifndef COMMON_COMMON_RUNTIME_INCLUDE_BASE_OS_THREAD_H
#define COMMON_COMMON_RUNTIME_INCLUDE_BASE_OS_THREAD_H

#include <cstdint>
#include <thread>

namespace common::os::thread {

using ThreadId = uint32_t;
using NativeHandleType = std::thread::native_handle_type;

/// @returns the caller's thread identifier
ThreadId GetCurrentThreadId();

/// @returns the caller's unique native thread identifer
NativeHandleType GetNativeHandle();

/**
 * @brief Set a name for the a thread, which can be useful for debugging purposes
 * @param threadNativeHandle specifies the thread whose name is to be set
 * @param name null-terminated buffer with the name for the thread
 * @returns 0 on success, nonzero error code otherwise
 */
int SetThreadName(NativeHandleType threadNativeHandle, const char *name);

}  // namespace common::os::thread

#endif  // COMMON_COMMON_RUNTIME_INCLUDE_BASE_OS_THREAD_H

