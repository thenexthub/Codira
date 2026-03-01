//===-- ExceptionBreakpoint.h -----------------------------------*- C++ -*-===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_EXCEPTIONBREAKPOINT_H
#define LLDB_TOOLS_LLDB_DAP_EXCEPTIONBREAKPOINT_H

#include "DAPForward.h"
#include "Protocol/ProtocolTypes.h"
#include "lldb/API/SBBreakpoint.h"
#include "lldb/lldb-enumerations.h"
#include "llvm/ADT/StringRef.h"
#include <string>
#include <utility>

namespace lldb_dap {

enum ExceptionKind : unsigned {
  eExceptionKindCatch,
  eExceptionKindThrow,
};

class ExceptionBreakpoint {
public:
  ExceptionBreakpoint(DAP &d, std::string f, std::string l,
                      lldb::LanguageType lang, ExceptionKind kind)
      : m_dap(d), m_filter(std::move(f)), m_label(std::move(l)),
        m_language(lang), m_kind(kind), m_bp() {}

  protocol::Breakpoint SetBreakpoint() { return SetBreakpoint(""); };
  protocol::Breakpoint SetBreakpoint(llvm::StringRef condition);
  void ClearBreakpoint();

  lldb::break_id_t GetID() const { return m_bp.GetID(); }
  llvm::StringRef GetFilter() const { return m_filter; }
  llvm::StringRef GetLabel() const { return m_label; }

  static constexpr bool kDefaultValue = false;

protected:
  DAP &m_dap;
  std::string m_filter;
  std::string m_label;
  lldb::LanguageType m_language;
  ExceptionKind m_kind;
  lldb::SBBreakpoint m_bp;
};

} // namespace lldb_dap

#endif
