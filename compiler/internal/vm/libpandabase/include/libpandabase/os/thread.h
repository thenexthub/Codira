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

#ifndef LIBPANDABASE_OS_THREAD_H
#define LIBPANDABASE_OS_THREAD_H

#include "os/error.h"
#include "utils/expected.h"

#include <cstdint>
#include <memory>
#include <thread>
#include <pthread.h>
#ifdef PANDA_TARGET_UNIX
#include "unix/libpandabase/thread.h"
#elif defined PANDA_TARGET_WINDOWS
#include "windows/libpandabase/thread.h"
#else
#error "Unsupported platform"
#endif

namespace panda::os::thread {

using ThreadId = uint32_t;
using native_handle_type = std::thread::native_handle_type;

WEAK_FOR_LTO_START

ThreadId GetCurrentThreadId();
int GetPid();
int SetThreadName(native_handle_type pthread_handle, const char *name);
native_handle_type GetNativeHandle();
void NativeSleep(unsigned int ms);
void ThreadDetach(native_handle_type pthread_handle);
void ThreadExit(void *ret);
void ThreadJoin(native_handle_type pthread_handle, void **ret);
void ThreadSendSignal(native_handle_type pthread_handle, int sig);
void ThreadYield();

WEAK_FOR_LTO_END

// Templated functions need to be defined here to be accessible everywhere

namespace internal {

template <typename T>
struct SharedPtrStruct;

template <typename T>
using SharedPtrToSharedPtrStruct = std::shared_ptr<SharedPtrStruct<T>>;

template <typename T>
struct SharedPtrStruct {
    SharedPtrToSharedPtrStruct<T> this_ptr;  // NOLINT(misc-non-private-member-variables-in-classes)
    T data;                                  // NOLINT(misc-non-private-member-variables-in-classes)
    SharedPtrStruct(SharedPtrToSharedPtrStruct<T> ptr_in, T data_in)
        : this_ptr(std::move(ptr_in)), data(std::move(data_in))
    {
    }
};

template <size_t... Is>
struct Seq {
};

template <size_t N, size_t... Is>
struct GenArgSeq : GenArgSeq<N - 1, N - 1, Is...> {
};

template <size_t... Is>
struct GenArgSeq<1, Is...> : Seq<Is...> {
};

template <class Func, typename Tuple, size_t... I>
static void CallFunc(Func &func, Tuple &args, Seq<I...> /* unused */)
{
    func(std::get<I>(args)...);
}

template <class Func, typename Tuple, size_t N>
static void CallFunc(Func &func, Tuple &args)
{
    CallFunc(func, args, GenArgSeq<N>());
}

template <typename Func, typename Tuple, size_t N>
static void *ProxyFunc(void *args)
{
    // Parse pointer and move args to local tuple.
    // We need this pointer to be destroyed by the time function starts to avoid memleak on thread  termination
    Tuple args_tuple;
    {
        auto args_ptr = static_cast<SharedPtrStruct<Tuple> *>(args);
        SharedPtrToSharedPtrStruct<Tuple> local;
        // This breaks shared pointer loop
        local.swap(args_ptr->this_ptr);
        // This moves tuple data to local variable
        args_tuple = args_ptr->data;
    }
    Func *func = std::get<0>(args_tuple);
    CallFunc<Func, Tuple, N>(*func, args_tuple);
    return nullptr;
}

}  // namespace internal

template <typename Func, typename... Args>
native_handle_type ThreadStart(Func *func, Args... args)
{
#ifdef PANDA_TARGET_UNIX
    native_handle_type tid;
#else
    pthread_t tid;
#endif
    auto args_tuple = std::make_tuple(func, std::move(args)...);
    internal::SharedPtrStruct<decltype(args_tuple)> *ptr = nullptr;
    {
        auto shared_ptr = std::make_shared<internal::SharedPtrStruct<decltype(args_tuple)>>(nullptr, args_tuple);
        ptr = shared_ptr.get();
        // Make recursive ref to prevent from shared pointer being destroyed until child thread acquires it.
        ptr->this_ptr = shared_ptr;
        // Leave scope to make sure that local shared_ptr was destroyed before thread creation
    }
    pthread_attr_t attr;
    pthread_attr_init(&attr);
#ifdef PANDA_TARGET_MACOS
    // In MacOS, the stack size of child thread is 512KB by default. Adjust it to 8MB to be consistent with Linux.
    // Increases stack size to accommodate stack overflow protection gaps while maintaining backward
    // compatibility with the original stack layout. Stack size must be multiple of segment alignment (16KB).
    // Therefore, increase the stack space by 64K.
    size_t stack_size = 8256 * 1024;
    pthread_attr_setstacksize(&attr, stack_size);
#endif
    pthread_create(&tid, &attr,
                   &internal::ProxyFunc<Func, decltype(args_tuple), std::tuple_size<decltype(args_tuple)>::value>,
                   static_cast<void *>(ptr));
#ifdef PANDA_TARGET_UNIX
    return tid;
#else
    return reinterpret_cast<native_handle_type>(tid);
#endif
}

WEAK_FOR_LTO_START
int ThreadGetStackInfo(native_handle_type thread, void **stack_addr, size_t *stack_size, size_t *guard_size);
WEAK_FOR_LTO_END

inline bool IsSetPriorityError(int res)
{
#ifdef PANDA_TARGET_UNIX
    return res != 0;
#elif defined(PANDA_TARGET_WINDOWS)
    return res == 0;
#endif
}
}  // namespace panda::os::thread

#endif  // LIBPANDABASE_OS_THREAD_H
