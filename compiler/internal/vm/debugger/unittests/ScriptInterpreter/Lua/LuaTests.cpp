//===-- LuaTests.cpp ------------------------------------------------------===//
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

#include "Plugins/ScriptInterpreter/Lua/Lua.h"
#include "Plugins/ScriptInterpreter/Lua/SWIGLuaBridge.h"
#include "gtest/gtest.h"

using namespace lldb_private;

extern "C" int luaopen_lldb(lua_State *L) { return 0; }

llvm::Expected<bool>
lldb_private::lua::SWIGBridge::LLDBSwigLuaBreakpointCallbackFunction(
    lua_State *L, lldb::StackFrameSP stop_frame_sp,
    lldb::BreakpointLocationSP bp_loc_sp,
    const StructuredDataImpl &extra_args_impl) {
  return false;
}

llvm::Expected<bool>
lldb_private::lua::SWIGBridge::LLDBSwigLuaWatchpointCallbackFunction(
    lua_State *L, lldb::StackFrameSP stop_frame_sp, lldb::WatchpointSP wp_sp) {
  return false;
}

TEST(LuaTest, RunValid) {
  Lua lua;
  llvm::Error error = lua.Run("foo = 1");
  EXPECT_FALSE(static_cast<bool>(error));
}

TEST(LuaTest, RunInvalid) {
  Lua lua;
  llvm::Error error = lua.Run("nil = foo");
  EXPECT_TRUE(static_cast<bool>(error));
  EXPECT_EQ(llvm::toString(std::move(error)),
            "[string \"buffer\"]:1: unexpected symbol near 'nil'\n");
}
