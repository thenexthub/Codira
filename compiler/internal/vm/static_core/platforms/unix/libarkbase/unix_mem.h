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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_UNIX_UNIX_MEM_H_
#define PANDA_LIBPANDABASE_PBASE_OS_UNIX_UNIX_MEM_H_

#include <sys/mman.h>

namespace ark::os::mem {

static constexpr uint32_t MMAP_PROT_NONE = PROT_NONE;
static constexpr uint32_t MMAP_PROT_READ = PROT_READ;
static constexpr uint32_t MMAP_PROT_WRITE = PROT_WRITE;
static constexpr uint32_t MMAP_PROT_EXEC = PROT_EXEC;

static constexpr uint32_t MMAP_FLAG_SHARED = MAP_SHARED;
static constexpr uint32_t MMAP_FLAG_PRIVATE = MAP_PRIVATE;
static constexpr uint32_t MMAP_FLAG_FIXED = MAP_FIXED;
static constexpr uint32_t MMAP_FLAG_ANONYMOUS = MAP_ANONYMOUS;

}  // namespace ark::os::mem
#endif  // PANDA_LIBPANDABASE_PBASE_OS_UNIX_UNIX_MEM_H_
