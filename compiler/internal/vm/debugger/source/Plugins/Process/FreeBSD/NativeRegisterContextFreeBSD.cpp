//===-- NativeRegisterContextFreeBSD.cpp ----------------------------------===//
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

#include "NativeRegisterContextFreeBSD.h"

#include "Plugins/Process/FreeBSD/NativeProcessFreeBSD.h"

#include "lldb/Host/common/NativeProcessProtocol.h"

using namespace lldb_private;
using namespace lldb_private::process_freebsd;

// clang-format off
#include <sys/types.h>
#include <sys/ptrace.h>
// clang-format on

NativeProcessFreeBSD &NativeRegisterContextFreeBSD::GetProcess() {
  return static_cast<NativeProcessFreeBSD &>(m_thread.GetProcess());
}

::pid_t NativeRegisterContextFreeBSD::GetProcessPid() {
  return GetProcess().GetID();
}
