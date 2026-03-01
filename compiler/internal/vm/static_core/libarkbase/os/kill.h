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

#ifndef PANDA_LIBPANDABASE_OS_KILL_H
#define PANDA_LIBPANDABASE_OS_KILL_H

#include "libarkbase/macros.h"

namespace ark::os::kill_process {

// Kill a process with a specific  process id using a specific signal
PANDA_PUBLIC_API int Kill(uint64_t pid, int signal);

// Close a process with a specific  process id
PANDA_PUBLIC_API int Close(uint64_t pid);

}  // namespace ark::os::kill_process

#endif  // PANDA_LIBPANDABASE_OS_KILL_H
