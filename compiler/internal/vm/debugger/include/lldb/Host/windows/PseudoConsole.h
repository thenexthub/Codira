//===----------------------------------------------------------------------===//
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

#ifndef LIBLLDB_HOST_WINDOWS_PSEUDOCONSOLE_H_
#define LIBLLDB_HOST_WINDOWS_PSEUDOCONSOLE_H_

#include "llvm/Support/Error.h"
#include <string>

#define PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE 0x20016
typedef void *HANDLE;
typedef void *HPCON;

namespace lldb_private {

class PseudoConsole {

public:
  llvm::Error OpenPseudoConsole();

  /// Close the ConPTY, its read/write handles and invalidate them.
  void Close();

  /// The ConPTY HPCON handle accessor.
  ///
  /// This object retains ownership of the HPCON when this accessor is used.
  ///
  /// \return
  ///     The ConPTY HPCON handle, or INVALID_HANDLE_VALUE if it is currently
  ///     invalid.
  HPCON GetPseudoTerminalHandle() { return m_conpty_handle; };

  /// The STDOUT read HANDLE accessor.
  ///
  /// This object retains ownership of the HANDLE when this accessor is used.
  ///
  /// \return
  ///     The STDOUT read HANDLE, or INVALID_HANDLE_VALUE if it is currently
  ///     invalid.
  HANDLE GetSTDOUTHandle() const { return m_conpty_output; };

  /// The STDIN write HANDLE accessor.
  ///
  /// This object retains ownership of the HANDLE when this accessor is used.
  ///
  /// \return
  ///     The STDIN write HANDLE, or INVALID_HANDLE_VALUE if it is currently
  ///     invalid.
  HANDLE GetSTDINHandle() const { return m_conpty_input; };

protected:
  HANDLE m_conpty_handle = ((HANDLE)(long long)-1);
  HANDLE m_conpty_output = ((HANDLE)(long long)-1);
  HANDLE m_conpty_input = ((HANDLE)(long long)-1);
};
}; // namespace lldb_private

#endif // LIBLLDB_HOST_WINDOWS_PSEUDOCONSOLE_H_
