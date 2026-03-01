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

#ifndef LLDB_PLUGINS_PROTOCOL_MCP_RESOURCE_H
#define LLDB_PLUGINS_PROTOCOL_MCP_RESOURCE_H

#include "lldb/Protocol/MCP/Protocol.h"
#include "lldb/Protocol/MCP/Resource.h"
#include "lldb/lldb-forward.h"
#include "lldb/lldb-types.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <cstddef>
#include <vector>

namespace lldb_private::mcp {

class DebuggerResourceProvider : public lldb_protocol::mcp::ResourceProvider {
public:
  using ResourceProvider::ResourceProvider;
  virtual ~DebuggerResourceProvider() = default;

  std::vector<lldb_protocol::mcp::Resource> GetResources() const override;
  llvm::Expected<lldb_protocol::mcp::ReadResourceResult>
  ReadResource(llvm::StringRef uri) const override;

private:
  static lldb_protocol::mcp::Resource GetDebuggerResource(Debugger &debugger);
  static lldb_protocol::mcp::Resource GetTargetResource(size_t target_idx,
                                                        Target &target);

  static llvm::Expected<lldb_protocol::mcp::ReadResourceResult>
  ReadDebuggerResource(llvm::StringRef uri, lldb::user_id_t debugger_id);
  static llvm::Expected<lldb_protocol::mcp::ReadResourceResult>
  ReadTargetResource(llvm::StringRef uri, lldb::user_id_t debugger_id,
                     size_t target_idx);
};

} // namespace lldb_private::mcp

#endif
