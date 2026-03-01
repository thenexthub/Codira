//===-- Uio.h ---------------------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef liblldb_Host_linux_Uio_h_
#define liblldb_Host_linux_Uio_h_

#include "lldb/Host/Config.h"
#include <sys/uio.h>

// We shall provide our own implementation of process_vm_readv if it is not
// present
#if !HAVE_PROCESS_VM_READV
ssize_t process_vm_readv(::pid_t pid, const struct iovec *local_iov,
                         unsigned long liovcnt, const struct iovec *remote_iov,
                         unsigned long riovcnt, unsigned long flags);
#endif

#endif // liblldb_Host_linux_Uio_h_
