//===-- SWIGLuaBridge.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_PLUGINS_SCRIPTINTERPRETER_LUA_SWIGLUABRIDGE_H
#define LLDB_PLUGINS_SCRIPTINTERPRETER_LUA_SWIGLUABRIDGE_H

#include "lldb/lldb-forward.h"
#include "lua.hpp"
#include "llvm/Support/Error.h"

namespace lldb_private {

namespace lua {

class SWIGBridge {
public:
  static llvm::Expected<bool> LLDBSwigLuaBreakpointCallbackFunction(
      lua_State *L, lldb::StackFrameSP stop_frame_sp,
      lldb::BreakpointLocationSP bp_loc_sp,
      const StructuredDataImpl &extra_args_impl);

  static llvm::Expected<bool> LLDBSwigLuaWatchpointCallbackFunction(
      lua_State *L, lldb::StackFrameSP stop_frame_sp, lldb::WatchpointSP wp_sp);
};

} // namespace lua

} // namespace lldb_private

#endif // LLDB_PLUGINS_SCRIPTINTERPRETER_LUA_SWIGLUABRIDGE_H
