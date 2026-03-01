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

#ifndef DPROF_LIBDPROF_DPROF_IPC_IPC_UNIX_SOCKET_H
#define DPROF_LIBDPROF_DPROF_IPC_IPC_UNIX_SOCKET_H

#include "libarkbase/os/unique_fd.h"

namespace ark::dprof::ipc {
os::unique_fd::UniqueFd CreateUnixServerSocket(int backlog);
os::unique_fd::UniqueFd CreateUnixClientSocket();

bool WaitDataTimeout(int fd, int timeoutMs);
bool SendAll(int fd, const void *buf, int len);
int RecvTimeout(int fd, void *buf, int len, int timeoutMs);
}  // namespace ark::dprof::ipc

#endif  // DPROF_LIBDPROF_DPROF_IPC_IPC_UNIX_SOCKET_H
