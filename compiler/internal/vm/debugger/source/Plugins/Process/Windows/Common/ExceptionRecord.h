//===-- ExceptionRecord.h ---------------------------------------*- C++ -*-===//
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

#ifndef liblldb_Plugins_Process_Windows_ExceptionRecord_H_
#define liblldb_Plugins_Process_Windows_ExceptionRecord_H_

#include "lldb/Host/windows/windows.h"
#include "lldb/lldb-forward.h"
#include <dbghelp.h>

#include <memory>
#include <vector>

namespace lldb_private {

// ExceptionRecord
//
// ExceptionRecord defines an interface which allows implementors to receive
// notification of events that happen in a debugged process.
class ExceptionRecord {
public:
  ExceptionRecord(const EXCEPTION_RECORD &record, lldb::tid_t thread_id) {
    // Notes about the `record.ExceptionRecord` field:
    // In the past, some code tried to parse the nested exception with it, but
    // in practice, that code just causes Access Violation. I suspect
    // `ExceptionRecord` here actually points to the address space of the
    // debuggee process. However, I did not manage to find any official or
    // unofficial reference that clarifies this point. If anyone would like to
    // reimplement this, please also keep in mind to check how this behaves when
    // debugging a WOW64 process. I suspect you may have to use the explicit
    // `EXCEPTION_RECORD32` and `EXCEPTION_RECORD64` structs.
    m_code = record.ExceptionCode;
    m_continuable = (record.ExceptionFlags == 0);
    m_exception_addr = reinterpret_cast<lldb::addr_t>(record.ExceptionAddress);
    m_thread_id = thread_id;
    m_arguments.assign(record.ExceptionInformation,
                       record.ExceptionInformation + record.NumberParameters);
  }

  // MINIDUMP_EXCEPTIONs are almost identical to EXCEPTION_RECORDs.
  ExceptionRecord(const MINIDUMP_EXCEPTION &record, lldb::tid_t thread_id)
      : m_code(record.ExceptionCode), m_continuable(record.ExceptionFlags == 0),
        m_exception_addr(static_cast<lldb::addr_t>(record.ExceptionAddress)),
        m_thread_id(thread_id),
        m_arguments(record.ExceptionInformation,
                    record.ExceptionInformation + record.NumberParameters) {}

  virtual ~ExceptionRecord() {}

  DWORD
  GetExceptionCode() const { return m_code; }
  bool IsContinuable() const { return m_continuable; }
  lldb::addr_t GetExceptionAddress() const { return m_exception_addr; }

  lldb::tid_t GetThreadID() const { return m_thread_id; }

  const std::vector<ULONG_PTR>& GetExceptionArguments() const { return m_arguments; }

private:
  DWORD m_code;
  bool m_continuable;
  lldb::addr_t m_exception_addr;
  lldb::tid_t m_thread_id;
  std::vector<ULONG_PTR> m_arguments;
};
}

#endif
