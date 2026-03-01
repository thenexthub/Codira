//===-- NativeRegisterContextWindows.h --------------------------*- C++ -*-===//
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

#ifndef liblldb_NativeRegisterContextWindows_h_
#define liblldb_NativeRegisterContextWindows_h_

#include "Plugins/Process/Utility/NativeRegisterContextRegisterInfo.h"
#include "lldb/Host/common/NativeThreadProtocol.h"
#include "lldb/Utility/DataBufferHeap.h"

namespace lldb_private {

class NativeThreadWindows;

class NativeRegisterContextWindows
    : public virtual NativeRegisterContextRegisterInfo {
public:
  static std::unique_ptr<NativeRegisterContextWindows>
  CreateHostNativeRegisterContextWindows(const ArchSpec &target_arch,
                                         NativeThreadProtocol &native_thread);

  // MSVC compiler deletes the default constructor due to virtual inheritance.
  // Explicitly defining it ensures the class remains constructible.
  NativeRegisterContextWindows() {}

protected:
  lldb::thread_t GetThreadHandle() const;
};

} // namespace lldb_private

#endif // liblldb_NativeRegisterContextWindows_h_
