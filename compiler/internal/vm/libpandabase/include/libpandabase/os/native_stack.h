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

#ifndef LIBPANDABASE_OS_NATIVE_STACK_H
#define LIBPANDABASE_OS_NATIVE_STACK_H

#include <set>
#include <csignal>
#include <string>

#if defined(PANDA_TARGET_UNIX)
#include "unix/libpandabase/native_stack.h"
#endif  // PANDA_TARGET_UNIX
#include "os/thread.h"

namespace panda::os::native_stack {

#if defined(PANDA_TARGET_UNIX)
const auto g_PandaThreadSigmask = pthread_sigmask;  // NOLINT(readability-identifier-naming)
const auto ReadOsFile = panda::os::unix::native_stack::ReadOsFile;      // NOLINT(readability-identifier-naming)
const auto WriterOsFile = panda::os::unix::native_stack::WriterOsFile;  // NOLINT(readability-identifier-naming)
#else
using FUNC_UNWINDSTACK = bool (*)(pid_t, std::ostream &, int);
bool ReadOsFile(const std::string &file_name, std::string *result);
bool WriterOsFile(const void *buffer, size_t count, int fd);
#endif  // PANDA_TARGET_UNIX

}  // namespace panda::os::native_stack

#endif  // LIBPANDABASE_OS_NATIVE_STACK_H
