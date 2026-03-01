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


#ifndef MRT_EXCEPTION_MANAGER_H
#define MRT_EXCEPTION_MANAGER_H

#include <vector>

#include "Codira.h"
#include "Base/CString.h"
#include "Common/TypeDef.h"
#include "Exception/ExceptionCApi.h"

namespace MapleRuntime {
using ExceptionRaiser = void (*)(int, void*);
class ExceptionManager {
public:
    ExceptionManager() {}
    ~ExceptionManager() = default;

    enum ImplicitExceptionType : uint8_t {
        OOM = 0,       // OutOfMemoryError
        SOF = 1,       // StackOverflowError
        OOMR = 2,      // OutOfMemoryRecursionError
        INCOMP = 3,    // IncompatiblePackageExpection
        MAX_COUNT = 4, // implicit exception count
    };

#ifdef __APPLE__
    static void DefaultUncaughtTask(const char* sunmary, const CODEErrorObject errorObj);
#endif

    // runtime required lifecycle interfaces
    void Init()
    {
        uncaughtExceptionHandler.hapPath = nullptr;
#ifdef __APPLE__
        uncaughtExceptionHandler.uncaughtTask = DefaultUncaughtTask;
#endif
    };
    void Fini() const {};

    static void OutOfMemory(); // should it be no-return
    static void StackOverflow(uint32_t adjustedSize, void* ip);
    static void IncompatiblePackageExpection(CString msg); // should it be no-return

    static void DumpException();
    static void ThrowException(const ExceptionRef& exception);
    static void* BeginCatch(ExceptionWrapper* mExceptionWrapper);

    static inline bool HasFatalException();
    static inline void ThrowPendingException();
    static inline bool HasPendingException();
    static inline void CheckAndThrowPendingException(const CString& msg);
    static inline void CheckAndDumpException();
    static inline ExceptionRef GetPendingException();
    static inline void ClearPendingException();
    static inline void* GetExceptionWrapper();
    static inline uint32_t GetExceptionTypeID();
    static inline void EndCatch();

    static void ThrowImplicitException(ImplicitExceptionType type);
    void RegisterExceptionRaiser(void* raiser)
    {
        exceptionRaiser = reinterpret_cast<ExceptionRaiser>(raiser);
    }

    ExceptionRaiser GetExceptionRaiser() const
    {
        return exceptionRaiser;
    }
#if defined(__OHOS__) && (__OHOS__ == 1)
    void RegisterUncaughtExceptionHandler(const CODEUncaughtExceptionInfo& handler);
#endif
    CODEUncaughtExceptionInfo GetUncaughtExceptionHandler() const
    {
        return uncaughtExceptionHandler;
    }

private:
#if defined(_WIN64)
    static constexpr uint32_t COMPENSATE_SIZE = 16;
#elif defined(__x86_64__)
    static constexpr uint32_t COMPENSATE_SIZE = 32;
#endif
    ExceptionRaiser exceptionRaiser = nullptr;
    static std::mutex gUncaughtExceptionHandlerMtx;
    CODEUncaughtExceptionInfo uncaughtExceptionHandler;
};
} // namespace MapleRuntime

#endif // MRT_EXCEPTION_MANAGER_H
