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

#ifndef PANDA_LIBPANDABASE_OS_THREAD_H_
#define PANDA_LIBPANDABASE_OS_THREAD_H_

#include "libarkbase/os/error.h"
#include "libarkbase/utils/expected.h"

#include <cstdint>
#include <memory>
#include <thread>
#include <pthread.h>
#ifdef PANDA_TARGET_UNIX
#include "platforms/unix/libarkbase/thread.h"
#elif defined PANDA_TARGET_WINDOWS
#include "platforms/windows/libarkbase/thread.h"
#else
#error "Unsupported platform"
#endif

namespace ark::os::thread {

using ThreadId = uint32_t;
using NativeHandleType = std::thread::native_handle_type;

WEAK_FOR_LTO_START

PANDA_PUBLIC_API ThreadId GetCurrentThreadId();
PANDA_PUBLIC_API int GetPid();
PANDA_PUBLIC_API int GetPPid();
PANDA_PUBLIC_API int GetUid();
PANDA_PUBLIC_API int GetEuid();
PANDA_PUBLIC_API uint32_t GetGid();
PANDA_PUBLIC_API uint32_t GetEgid();
PANDA_PUBLIC_API std::vector<uint32_t> GetGroups();
PANDA_PUBLIC_API int SetThreadName(NativeHandleType pthreadHandle, const char *name);
PANDA_PUBLIC_API NativeHandleType GetNativeHandle();
PANDA_PUBLIC_API void Yield();
PANDA_PUBLIC_API void NativeSleep(unsigned int ms);
PANDA_PUBLIC_API void NativeSleepUS(std::chrono::microseconds us);
PANDA_PUBLIC_API void ThreadDetach(NativeHandleType pthreadHandle);
PANDA_PUBLIC_API void ThreadJoin(NativeHandleType pthreadHandle, void **ret);
PANDA_PUBLIC_API void ThreadSendSignal(NativeHandleType pthreadHandle, int sig);

WEAK_FOR_LTO_END

// Templated functions need to be defined here to be accessible everywhere

namespace internal {

template <typename T>
struct SharedPtrStruct;

template <typename T>
using SharedPtrToSharedPtrStruct = std::shared_ptr<SharedPtrStruct<T>>;

template <typename T>
struct SharedPtrStruct {
    SharedPtrToSharedPtrStruct<T> thisPtr;  // NOLINT(misc-non-private-member-variables-in-classes)
    T data;                                 // NOLINT(misc-non-private-member-variables-in-classes)
    SharedPtrStruct(SharedPtrToSharedPtrStruct<T> ptrIn, T dataIn) : thisPtr(std::move(ptrIn)), data(std::move(dataIn))
    {
    }
};

template <size_t... IS>
struct Seq {
};

template <size_t N, size_t... IS>
struct GenArgSeq : GenArgSeq<N - 1, N - 1, IS...> {
};

template <size_t... IS>
struct GenArgSeq<1, IS...> : Seq<IS...> {
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
    Tuple argsTuple;
    {
        auto argsPtr = static_cast<SharedPtrStruct<Tuple> *>(args);
        SharedPtrToSharedPtrStruct<Tuple> local;
        // This breaks shared pointer loop
        local.swap(argsPtr->thisPtr);
        // This moves tuple data to local variable
        argsTuple = argsPtr->data;
    }
    Func *func = std::get<0>(argsTuple);
    CallFunc<Func, Tuple, N>(*func, argsTuple);
    return nullptr;
}

}  // namespace internal

template <typename Func, typename... Args>
NativeHandleType ThreadStart(Func *func, Args... args)
{
#ifdef PANDA_TARGET_UNIX
    NativeHandleType tid;
#else
    pthread_t tid;
#endif
    auto argsTuple = std::make_tuple(func, std::move(args)...);
    internal::SharedPtrStruct<decltype(argsTuple)> *ptr = nullptr;
    {
        auto sharedPtr = std::make_shared<internal::SharedPtrStruct<decltype(argsTuple)>>(nullptr, argsTuple);
        ptr = sharedPtr.get();
        // Make recursive ref to prevent from shared pointer being destroyed until child thread acquires it.
        ptr->thisPtr = sharedPtr;
        // Leave scope to make sure that local shared_ptr was destroyed before thread creation
    }
    pthread_create(&tid, nullptr,
                   &internal::ProxyFunc<Func, decltype(argsTuple), std::tuple_size<decltype(argsTuple)>::value>,
                   static_cast<void *>(ptr));
#ifdef PANDA_TARGET_UNIX
    return tid;
#else
    return reinterpret_cast<NativeHandleType>(tid);
#endif
}

WEAK_FOR_LTO_START
PANDA_PUBLIC_API int ThreadGetStackInfo(NativeHandleType thread, void **stackAddr, size_t *stackSize,
                                        size_t *guardSize);
WEAK_FOR_LTO_END

inline bool IsSetPriorityError(int res)
{
#ifdef PANDA_TARGET_UNIX
    return res != 0;
#elif defined(PANDA_TARGET_WINDOWS)
    return res == 0;
#endif
}
}  // namespace ark::os::thread

#endif  // PANDA_LIBPANDABASE_OS_THREAD_H_
