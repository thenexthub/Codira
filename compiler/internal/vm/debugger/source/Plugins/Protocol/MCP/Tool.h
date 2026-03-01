//===- Tool.h -------------------------------------------------------------===//
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

#ifndef LLDB_PLUGINS_PROTOCOL_MCP_TOOL_H
#define LLDB_PLUGINS_PROTOCOL_MCP_TOOL_H

#include "lldb/Protocol/MCP/Protocol.h"
#include "lldb/Protocol/MCP/Tool.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include <optional>

namespace lldb_private::mcp {

class CommandTool : public lldb_protocol::mcp::Tool {
public:
  using lldb_protocol::mcp::Tool::Tool;
  ~CommandTool() = default;

  llvm::Expected<lldb_protocol::mcp::CallToolResult>
  Call(const lldb_protocol::mcp::ToolArguments &args) override;

  std::optional<llvm::json::Value> GetSchema() const override;
};

class DebuggerListTool : public lldb_protocol::mcp::Tool {
public:
  using lldb_protocol::mcp::Tool::Tool;
  ~DebuggerListTool() = default;

  llvm::Expected<lldb_protocol::mcp::CallToolResult>
  Call(const lldb_protocol::mcp::ToolArguments &args) override;
};

} // namespace lldb_private::mcp

#endif
