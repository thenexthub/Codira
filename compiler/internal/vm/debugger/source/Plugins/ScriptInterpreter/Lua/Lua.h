//===-- ScriptInterpreterLua.h ----------------------------------*- C++ -*-===//
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

#ifndef liblldb_Lua_h_
#define liblldb_Lua_h_

#include "lldb/API/SBBreakpointLocation.h"
#include "lldb/API/SBFrame.h"
#include "lldb/Core/StructuredDataImpl.h"
#include "lldb/lldb-types.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"

#include "lua.hpp"

#include <mutex>

namespace lldb_private {

extern "C" {
int luaopen_lldb(lua_State *L);
}

class Lua {
public:
  Lua();
  ~Lua();

  llvm::Error Run(llvm::StringRef buffer);
  llvm::Error RegisterBreakpointCallback(void *baton, const char *body);
  llvm::Expected<bool>
  CallBreakpointCallback(void *baton, lldb::StackFrameSP stop_frame_sp,
                         lldb::BreakpointLocationSP bp_loc_sp,
                         StructuredData::ObjectSP extra_args_sp);
  llvm::Error RegisterWatchpointCallback(void *baton, const char *body);
  llvm::Expected<bool> CallWatchpointCallback(void *baton,
                                              lldb::StackFrameSP stop_frame_sp,
                                              lldb::WatchpointSP wp_sp);
  llvm::Error LoadModule(llvm::StringRef filename);
  llvm::Error CheckSyntax(llvm::StringRef buffer);
  llvm::Error ChangeIO(FILE *out, FILE *err);

private:
  lua_State *m_lua_state;
};

} // namespace lldb_private

#endif // liblldb_Lua_h_
