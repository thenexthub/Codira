//===-- Ptrace.h ------------------------------------------------*- C++ -*-===//
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

// This file defines ptrace functions & structures

#ifndef liblldb_Host_linux_Ptrace_h_
#define liblldb_Host_linux_Ptrace_h_

#include <sys/ptrace.h>

#ifndef __GLIBC__
typedef int __ptrace_request;
#endif

#define DEBUG_PTRACE_MAXBYTES 20

// Support ptrace extensions even when compiled without required kernel support
#ifndef PTRACE_GETREGS
#define PTRACE_GETREGS 12
#endif
#ifndef PTRACE_SETREGS
#define PTRACE_SETREGS 13
#endif
#ifndef PTRACE_GETFPREGS
#define PTRACE_GETFPREGS 14
#endif
#ifndef PTRACE_SETFPREGS
#define PTRACE_SETFPREGS 15
#endif
#ifndef PTRACE_GETREGSET
#define PTRACE_GETREGSET 0x4204
#endif
#ifndef PTRACE_SETREGSET
#define PTRACE_SETREGSET 0x4205
#endif
#ifndef PTRACE_GET_THREAD_AREA
#define PTRACE_GET_THREAD_AREA 25
#endif
#ifndef PTRACE_ARCH_PRCTL
#define PTRACE_ARCH_PRCTL 30
#endif
#ifndef ARCH_GET_FS
#define ARCH_SET_GS 0x1001
#define ARCH_SET_FS 0x1002
#define ARCH_GET_FS 0x1003
#define ARCH_GET_GS 0x1004
#endif
#ifndef PTRACE_PEEKMTETAGS
#define PTRACE_PEEKMTETAGS 33
#endif
#ifndef PTRACE_POKEMTETAGS
#define PTRACE_POKEMTETAGS 34
#endif

#endif // liblldb_Host_linux_Ptrace_h_
