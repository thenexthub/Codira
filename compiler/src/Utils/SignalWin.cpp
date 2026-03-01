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

/**
 * @file
 *
 * This file implements functions related to the Windows crash signal handler.
 */

#include "Codira/Driver/TempFileManager.h"

#if (defined RELEASE)
#include "SignalUtil.h"

namespace {
LONG WINAPI WindowsExceptionHandler(LPEXCEPTION_POINTERS ep)
{
    Codira::Signal::ThreadDelaySynchronizer();
    // Write out the exception code.
    if (!ep || !ep->ExceptionRecord) {
        return EXCEPTION_EXECUTE_HANDLER;
    }
    int64_t exitCode = ep->ExceptionRecord->ExceptionCode;
    Codira::Signal::WriteICEMessage(exitCode);
    Codira::TempFileManager::Instance().DeleteTempFiles();
    return exitCode;
}

void SignalHandler(int signum)
{
    Codira::Signal::ConcurrentSynchronousSignalHandler(signum);
}

} // namespace

namespace Codira {
void RegisterCrashExceptionHandler()
{
    SetUnhandledExceptionFilter(WindowsExceptionHandler); // Windows API
}

void RegisterCrashSignalHandler()
{
    int signals[] = {SIGABRT, SIGFPE, SIGILL, SIGSEGV};
    for (auto& sig : signals) {
        if (signal(sig, SignalHandler) == SIG_ERR) {
            // Even if sigaction failed, we are still able to continue our compiling.
            continue;
        }
    }
}
} // namespace Codira
#else
#include <csignal>
#include <windows.h>
#endif // (defined NDEBUG)

namespace {
BOOL WINAPI LLVMConsoleCtrlHandler(DWORD dwCtrlType)
{
    Codira::TempFileManager::Instance().DeleteTempFiles();
    return FALSE;
}
} // namespace

namespace Codira {
void RegisterCrtlCSignalHandler()
{
    (void)SetConsoleCtrlHandler(LLVMConsoleCtrlHandler, TRUE);
}
} // namespace Codira
