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
 * This file implements crash signal handler related functions.
 */

#if (defined RELEASE)
#include "SignalUtil.h"

#include <cstring>
#include "Codira/Basic/Version.h"
#include "Codira/Driver/TempFileManager.h"

namespace {
#ifdef CODIRA_BUILD_TESTS
// The file handle of output to stderror.
// In the case of a signal test, it could be a real file handle.
int g_errorFd = STDERR_FILENO;
// void (*)(int) test callback function pointer
Codira::SignalTest::SignalTestCallbackFuncType g_signalTestCallbackFunction = nullptr;
// The test callback function is not executed by default.
Codira::SignalTest::TriggerPointer g_signalTestCallbackFunctionTriggerPoint =
    Codira::SignalTest::TriggerPointer::NON_POINTER;
#else
// The file handle of output to stderror
constexpr int g_errorFd = STDERR_FILENO;
#endif

std::atomic<bool> g_processingSignalOrException(false);
constexpr size_t LOOP_SIZE = 100000;

void AsyncSigSafeReverse(char str[])
{
    for (size_t i = 0, j = strlen(str) - 1; i < j; i++, j--) {
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}

/* Convert int to base 10 string (from K&R) */
void AsyncSigSafeItoa(int64_t num, char str[])
{
    bool neg = num < 0;
    if (neg) {
        num = -num;
    }
    int64_t c = 0;
    size_t i = 0;
    const int64_t base = 10; // decimal
    do {
        c = num % base;
        str[i++] =
            static_cast<char>((c < base) ? (c + static_cast<int64_t>('0')) : ((c - base) + static_cast<int64_t>('a')));
    } while ((num /= base) > 0);
    if (neg) {
        str[i++] = '-';
    }
    str[i] = '\0';
    AsyncSigSafeReverse(str);
}

ssize_t AsyncSigSafeWriteToError(const char str[])
{
    return write(g_errorFd, str, strlen(str));
}

ssize_t AsyncSigSafePut(int64_t num) /* Put int */
{
    char str[128] = {0};
    AsyncSigSafeItoa(num, str); /* Based on K&R itoa() */
    return AsyncSigSafeWriteToError(str);
}

#ifdef CODIRA_BUILD_TESTS
void CloseTempFileHandle()
{
    if (g_errorFd != STDERR_FILENO) {
        close(g_errorFd);
        g_errorFd = STDERR_FILENO;
    }
}
#endif
} // namespace

using namespace Codira;

void Signal::WriteICEMessage(int64_t errorCode)
{
    (void)AsyncSigSafeWriteToError(Codira::CODIRA_COMPILER_VERSION.c_str());
    (void)AsyncSigSafeWriteToError("\n");
    (void)AsyncSigSafeWriteToError(Codira::ICE::MSG_PART_ONE.c_str());
    (void)AsyncSigSafeWriteToError(Codira::SIGNAL_MSG_PART_ONE.c_str());
    (void)AsyncSigSafePut(errorCode);
    (void)AsyncSigSafeWriteToError(Codira::SIGNAL_MSG_PART_TWO.c_str());
    (void)AsyncSigSafeWriteToError(Codira::ICE::MSG_PART_TWO.c_str());
    (void)AsyncSigSafePut(Codira::ICE::GetTriggerPoint());
    (void)AsyncSigSafeWriteToError("\n");
#ifdef CODIRA_BUILD_TESTS
    CloseTempFileHandle();
#endif
}

void Signal::ThreadDelaySynchronizer()
{
    // When multiple threads call this function at the same time, only the first thread can exit immediately,
    // and other threads can exit at a later time.
    if (g_processingSignalOrException.exchange(true)) {
        for (size_t i = LOOP_SIZE; i > 0; --i) {
            asm volatile(""); // This assembly prevents the loop from being optimized.
        }
    }
}

void Signal::ConcurrentSynchronousSignalHandler(int signum)
{
    ThreadDelaySynchronizer();
    WriteICEMessage(signum);
    Codira::TempFileManager::Instance().DeleteTempFiles(true);
    int exitCode = 128 + signum; // Add 128 to return the same error code as if the program crashed.
    _exit(exitCode);
}

#ifdef CODIRA_BUILD_TESTS

void SignalTest::SetSignalTestCallbackFunc(SignalTestCallbackFuncType fp, TriggerPointer triggerPoint, int fd)
{
    g_signalTestCallbackFunction = fp;
    g_signalTestCallbackFunctionTriggerPoint = triggerPoint;
    g_errorFd = fd;
}

void SignalTest::ExecuteSignalTestCallbackFunc(TriggerPointer executionPoint)
{
    if (g_signalTestCallbackFunction == nullptr) {
        return;
    }
    if (executionPoint == TriggerPointer::NON_POINTER ||
        g_signalTestCallbackFunctionTriggerPoint == TriggerPointer::NON_POINTER) {
        return;
    }
    if (executionPoint != g_signalTestCallbackFunctionTriggerPoint) {
        return;
    }
    g_signalTestCallbackFunction();
}

#endif // CODIRA_BUILD_TESTS

#endif // (defined NDEBUG)
