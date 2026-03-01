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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_NATIVESTACK_H
#define PANDA_LIBPANDABASE_PBASE_OS_NATIVESTACK_H

#include "libarkbase/os/thread.h"
#if defined(PANDA_TARGET_UNIX)
#include "platforms/unix/libarkbase/native_stack.h"
#endif  // PANDA_TARGET_UNIX

#include <csignal>
#include <string>
#include <set>

namespace ark::os::native_stack {

#if defined(PANDA_TARGET_UNIX)
using FuncUnwindstack = os::unix::native_stack::FuncUnwindstack;
const auto g_PandaThreadSigmask = pthread_sigmask;  // NOLINT(readability-identifier-naming)
using DumpUnattachedThread = ark::os::unix::native_stack::DumpUnattachedThread;
const auto DumpKernelStack = ark::os::unix::native_stack::DumpKernelStack;  // NOLINT(readability-identifier-naming)
const auto GetNativeThreadNameForFile =                                     // NOLINT(readability-identifier-naming)
    ark::os::unix::native_stack::GetNativeThreadNameForFile;
const auto ReadOsFile = ark::os::unix::native_stack::ReadOsFile;      // NOLINT(readability-identifier-naming)
const auto WriterOsFile = ark::os::unix::native_stack::WriterOsFile;  // NOLINT(readability-identifier-naming)
const auto ChangeJaveStackFormat =                                    // NOLINT(readability-identifier-naming)
    ark::os::unix::native_stack::ChangeJaveStackFormat;
#else
using FuncUnwindstack = bool (*)(pid_t, std::ostream &, int);
class DumpUnattachedThread {
public:
    void AddTid(pid_t tidThread);
    bool InitKernelTidLists();
    void Dump(std::ostream &os, bool dumpNativeCrash, FuncUnwindstack callUnwindstack);

private:
    std::set<pid_t> kernelTid_;
    std::set<pid_t> threadManagerTids_;
};
void DumpKernelStack(std::ostream &os, pid_t tid, const char *tag, bool count);
std::string GetNativeThreadNameForFile(pid_t tid);
bool ReadOsFile(const std::string &file_name, std::string *result);
bool WriterOsFile(const void *buffer, size_t count, int fd);
std::string ChangeJaveStackFormat(const char *descriptor);
#endif  // PANDA_TARGET_UNIX

}  // namespace ark::os::native_stack
#endif  // PANDA_LIBPANDABASE_PBASE_OS_NATIVESTACK_H
