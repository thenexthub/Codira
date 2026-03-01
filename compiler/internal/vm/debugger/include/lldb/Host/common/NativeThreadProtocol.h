//===-- NativeThreadProtocol.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_COMMON_NATIVETHREADPROTOCOL_H
#define LLDB_HOST_COMMON_NATIVETHREADPROTOCOL_H

#include <memory>

#include "lldb/Host/Debug.h"
#include "lldb/Utility/UnimplementedError.h"
#include "lldb/lldb-private-forward.h"
#include "lldb/lldb-types.h"

#include "llvm/Support/Error.h"
#include "llvm/Support/MemoryBuffer.h"

namespace lldb_private {
// NativeThreadProtocol
class NativeThreadProtocol {
public:
  NativeThreadProtocol(NativeProcessProtocol &process, lldb::tid_t tid);

  virtual ~NativeThreadProtocol() = default;

  virtual std::string GetName() = 0;

  virtual lldb::StateType GetState() = 0;

  virtual NativeRegisterContext &GetRegisterContext() = 0;

  virtual bool GetStopReason(ThreadStopInfo &stop_info,
                             std::string &description) = 0;

  lldb::tid_t GetID() const { return m_tid; }

  NativeProcessProtocol &GetProcess() { return m_process; }

  // Thread-specific watchpoints
  virtual Status SetWatchpoint(lldb::addr_t addr, size_t size,
                               uint32_t watch_flags, bool hardware) = 0;

  virtual Status RemoveWatchpoint(lldb::addr_t addr) = 0;

  // Thread-specific Hardware Breakpoint routines
  virtual Status SetHardwareBreakpoint(lldb::addr_t addr, size_t size) = 0;

  virtual Status RemoveHardwareBreakpoint(lldb::addr_t addr) = 0;

  virtual llvm::Expected<std::unique_ptr<llvm::MemoryBuffer>>
  GetSiginfo() const {
    return llvm::make_error<UnimplementedError>();
  }

protected:
  NativeProcessProtocol &m_process;
  lldb::tid_t m_tid;
};
}

#endif // LLDB_HOST_COMMON_NATIVETHREADPROTOCOL_H
