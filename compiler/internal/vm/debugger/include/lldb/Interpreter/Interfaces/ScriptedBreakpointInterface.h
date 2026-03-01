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

#ifndef LLDB_INTERPRETER_INTERFACES_SCRIPTEDBREAKPOINTINTERFACE_H
#define LLDB_INTERPRETER_INTERFACES_SCRIPTEDBREAKPOINTINTERFACE_H

#include "ScriptedInterface.h"
#include "lldb/Symbol/SymbolContext.h"
#include "lldb/lldb-private.h"

namespace lldb_private {
class ScriptedBreakpointInterface : public ScriptedInterface {
public:
  virtual llvm::Expected<StructuredData::GenericSP>
  CreatePluginObject(llvm::StringRef class_name, lldb::BreakpointSP break_sp,
                     const StructuredDataImpl &args_sp) = 0;

  /// "ResolverCallback" will get called when a new module is loaded.  The
  /// new module information is passed in sym_ctx.  The Resolver will add
  /// any breakpoint locations it found in that module.
  virtual bool ResolverCallback(SymbolContext sym_ctx) { return true; }
  virtual lldb::SearchDepth GetDepth() { return lldb::eSearchDepthModule; }
  virtual std::optional<std::string> GetShortHelp() { return nullptr; }
  /// WasHit returns the breakpoint location SP for the location that was "hit".
  virtual lldb::BreakpointLocationSP
  WasHit(lldb::StackFrameSP frame_sp, lldb::BreakpointLocationSP bp_loc_sp) {
    return LLDB_INVALID_BREAK_ID;
  }
  virtual std::optional<std::string>
  GetLocationDescription(lldb::BreakpointLocationSP bp_loc_sp,
                         lldb::DescriptionLevel level) {
    return {};
  }
};
} // namespace lldb_private

#endif // LLDB_INTERPRETER_INTERFACES_SCRIPTEDSTOPHOOKINTERFACE_H
