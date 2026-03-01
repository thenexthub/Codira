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

#ifndef PLATFORMS_UNIX_LIBPANDABASE_NATIVE_STACK_H
#define PLATFORMS_UNIX_LIBPANDABASE_NATIVE_STACK_H

#include <string>
#include <set>
#include "os/thread.h"

namespace panda::os::unix::native_stack {
using FUNC_UNWINDSTACK = bool (*)(pid_t, std::ostream &, int);

bool ReadOsFile(const std::string &file_name, std::string *result);
bool WriterOsFile(const void *buffer, size_t count, int fd);

}  // namespace panda::os::unix::native_stack

#endif  // PLATFORMS_UNIX_LIBPANDABASE_NATIVE_STACK_H
