//===-- Statusline.h -----------------------------------------------------===//
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

#ifndef LLDB_CORE_STATUSLINE_H
#define LLDB_CORE_STATUSLINE_H

#include "lldb/Symbol/SymbolContext.h"
#include "lldb/Target/ExecutionContext.h"
#include "lldb/lldb-forward.h"
#include <cstdint>
#include <string>

namespace lldb_private {
class Statusline {
public:
  Statusline(Debugger &debugger);
  ~Statusline();

  using Context = std::pair<ExecutionContextRef, SymbolContext>;

  /// Reduce the scroll window and draw the statusline.
  void Enable(std::optional<ExecutionContextRef> exe_ctx_ref);

  /// Hide the statusline and extend the scroll window.
  void Disable();

  /// Redraw the statusline.
  void Redraw(std::optional<ExecutionContextRef> exe_ctx_ref);

  /// Inform the statusline that the terminal dimensions have changed.
  void TerminalSizeChanged();

private:
  /// Draw the statusline with the given text.
  void Draw(std::string msg);

  enum ScrollWindowMode {
    EnableStatusline,
    DisableStatusline,
    ResizeStatusline,
  };

  /// Set the scroll window for the given mode.
  void UpdateScrollWindow(ScrollWindowMode mode);

  Debugger &m_debugger;

  /// Cached copy of the execution context that allows us to redraw the
  /// statusline.
  ExecutionContextRef m_exe_ctx_ref;

  uint64_t m_terminal_width = 0;
  uint64_t m_terminal_height = 0;
};
} // namespace lldb_private
#endif // LLDB_CORE_STATUSLINE_H
