//===-- NativeRegisterContextFreeBSD.h --------------------------*- C++ -*-===//
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

#ifndef lldb_NativeRegisterContextFreeBSD_h
#define lldb_NativeRegisterContextFreeBSD_h

#include "Plugins/Process/Utility/NativeRegisterContextRegisterInfo.h"

namespace lldb_private {
namespace process_freebsd {

class NativeProcessFreeBSD;
class NativeThreadFreeBSD;

class NativeRegisterContextFreeBSD
    : public virtual NativeRegisterContextRegisterInfo {
public:
  // This function is implemented in the NativeRegisterContextFreeBSD_*
  // subclasses to create a new instance of the host specific
  // NativeRegisterContextFreeBSD. The implementations can't collide as only one
  // NativeRegisterContextFreeBSD_* variant should be compiled into the final
  // executable.
  static NativeRegisterContextFreeBSD *
  CreateHostNativeRegisterContextFreeBSD(const ArchSpec &target_arch,
                                         NativeThreadFreeBSD &native_thread);
  virtual llvm::Error
  CopyHardwareWatchpointsFrom(NativeRegisterContextFreeBSD &source) = 0;

protected:
  virtual NativeProcessFreeBSD &GetProcess();
  virtual ::pid_t GetProcessPid();
};

} // namespace process_freebsd
} // namespace lldb_private

#endif // #ifndef lldb_NativeRegisterContextFreeBSD_h
