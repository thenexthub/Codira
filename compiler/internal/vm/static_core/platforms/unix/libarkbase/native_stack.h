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

#ifndef PANDA_PLATFORMS_UNIX_LIBPANDABASE_NATIVE_STACK_H
#define PANDA_PLATFORMS_UNIX_LIBPANDABASE_NATIVE_STACK_H

#include "libarkbase/os/thread.h"
#include <string>
#include <set>
#include "libarkbase/macros.h"

namespace ark::os::unix::native_stack {
using FuncUnwindstack = bool (*)(pid_t, std::ostream &, int);

PANDA_PUBLIC_API void DumpKernelStack(std::ostream &os, pid_t tid, const char *tag, bool count);

PANDA_PUBLIC_API std::string GetNativeThreadNameForFile(pid_t tid);

class DumpUnattachedThread {
public:
    PANDA_PUBLIC_API void AddTid(pid_t tidThread);
    PANDA_PUBLIC_API bool InitKernelTidLists();
    PANDA_PUBLIC_API void Dump(std::ostream &os, bool dumpNativeCrash, FuncUnwindstack callUnwindstack);

private:
    std::set<pid_t> kernelTid_;
    std::set<pid_t> threadManagerTids_;
};

PANDA_PUBLIC_API bool ReadOsFile(const std::string &fileName, std::string *result);
PANDA_PUBLIC_API bool WriterOsFile(const void *buffer, size_t count, int fd);
PANDA_PUBLIC_API std::string ChangeJaveStackFormat(const char *descriptor);

}  // namespace ark::os::unix::native_stack

#endif  // PANDA_PLATFORMS_UNIX_LIBPANDABASE_NATIVE_STACK_H
