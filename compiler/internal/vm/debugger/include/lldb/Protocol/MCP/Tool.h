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

#ifndef LLDB_PROTOCOL_MCP_TOOL_H
#define LLDB_PROTOCOL_MCP_TOOL_H

#include "lldb/Protocol/MCP/Protocol.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/JSON.h"
#include <string>

namespace lldb_protocol::mcp {

class Tool {
public:
  Tool(std::string name, std::string description);
  virtual ~Tool() = default;

  virtual llvm::Expected<lldb_protocol::mcp::CallToolResult>
  Call(const lldb_protocol::mcp::ToolArguments &args) = 0;

  virtual std::optional<llvm::json::Value> GetSchema() const {
    return llvm::json::Object{{"type", "object"}};
  }

  lldb_protocol::mcp::ToolDefinition GetDefinition() const;

  const std::string &GetName() { return m_name; }

private:
  std::string m_name;
  std::string m_description;
};

} // namespace lldb_protocol::mcp

#endif
