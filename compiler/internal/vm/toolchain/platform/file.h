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

#ifndef ARKCOMPILER_TOOLCHAIN_PLATFORM_FILE_H
#define ARKCOMPILER_TOOLCHAIN_PLATFORM_FILE_H

#ifdef WINDOWS_PLATFORM
#include <windows.h>
#else
#include <stdio.h>
#include <unistd.h>
#endif

namespace OHOS::ArkCompiler::Toolchain {
#ifdef WINDOWS_PLATFORM
using fd_t = HANDLE;
#define INVALID_FD INVALID_HANDLE_VALUE

#ifdef ERROR
#undef ERROR
#endif

#ifdef VOID
#undef VOID
#endif

#ifdef CONST
#undef CONST
#endif
#else
using fd_t = int;
#define INVALID_FD (-1)
#endif

void FdsanExchangeOwnerTag(fd_t fd);
void FdsanClose(fd_t fd);
} // namespace OHOS::ArkCompiler::Toolchain
#endif // ARKCOMPILER_TOOLCHAIN_PLATFORM_FILE_H