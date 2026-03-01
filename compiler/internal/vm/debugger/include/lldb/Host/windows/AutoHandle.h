//===-- AutoHandle.h --------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_lldb_Host_windows_AutoHandle_h_
#define LLDB_lldb_Host_windows_AutoHandle_h_

#include "lldb/Host/windows/windows.h"

namespace lldb_private {

class AutoHandle {
public:
  AutoHandle(HANDLE handle, HANDLE invalid_value = INVALID_HANDLE_VALUE)
      : m_handle(handle), m_invalid_value(invalid_value) {}

  ~AutoHandle() {
    if (m_handle != m_invalid_value)
      ::CloseHandle(m_handle);
  }

  bool IsValid() const { return m_handle != m_invalid_value; }

  HANDLE get() const { return m_handle; }

private:
  HANDLE m_handle;
  HANDLE m_invalid_value;
};
}

#endif
