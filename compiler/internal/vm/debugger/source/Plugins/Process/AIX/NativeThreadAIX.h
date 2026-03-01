//===-- NativeThreadAIX.h ----------------------------------- -*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVETHREADAIX_H_
#define LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVETHREADAIX_H_

#include "lldb/Host/common/NativeThreadProtocol.h"

namespace lldb_private::process_aix {

class NativeProcessAIX;

class NativeThreadAIX : public NativeThreadProtocol {
  friend class NativeProcessAIX;

public:
  NativeThreadAIX(NativeProcessAIX &process, lldb::tid_t tid);

  // NativeThreadProtocol Interface
  std::string GetName() override;

  lldb::StateType GetState() override;

  bool GetStopReason(ThreadStopInfo &stop_info,
                     std::string &description) override;

  Status SetWatchpoint(lldb::addr_t addr, size_t size, uint32_t watch_flags,
                       bool hardware) override;

  Status RemoveWatchpoint(lldb::addr_t addr) override;

  Status SetHardwareBreakpoint(lldb::addr_t addr, size_t size) override;

  Status RemoveHardwareBreakpoint(lldb::addr_t addr) override;

  NativeProcessAIX &GetProcess();

  const NativeProcessAIX &GetProcess() const;

  llvm::Expected<std::unique_ptr<llvm::MemoryBuffer>>
  GetSiginfo() const override;

private:
  lldb::StateType m_state;
};
} // namespace lldb_private::process_aix

#endif // #ifndef LLDB_SOURCE_PLUGINS_PROCESS_AIX_NATIVETHREADAIX_H_
