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

#ifndef LLDB_PROTOCOL_MCP_MCPERROR_H
#define LLDB_PROTOCOL_MCP_MCPERROR_H

#include "llvm/Support/Error.h"
#include <string>

namespace lldb_protocol::mcp {

class MCPError : public llvm::ErrorInfo<MCPError> {
public:
  static char ID;

  MCPError(std::string message, int64_t error_code = kInternalError);

  void log(llvm::raw_ostream &OS) const override;
  std::error_code convertToErrorCode() const override;

  const std::string &getMessage() const { return m_message; }

  static constexpr int64_t kResourceNotFound = -32002;
  static constexpr int64_t kInternalError = -32603;

private:
  std::string m_message;
  int m_error_code;
};

class UnsupportedURI : public llvm::ErrorInfo<UnsupportedURI> {
public:
  static char ID;

  UnsupportedURI(std::string uri);

  void log(llvm::raw_ostream &OS) const override;
  std::error_code convertToErrorCode() const override;

private:
  std::string m_uri;
};

} // namespace lldb_protocol::mcp

#endif
