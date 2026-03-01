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

#include "lldb/Protocol/MCP/MCPError.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>

using namespace lldb_protocol::mcp;

char MCPError::ID;
char UnsupportedURI::ID;

MCPError::MCPError(std::string message, int64_t error_code)
    : m_message(message), m_error_code(error_code) {}

void MCPError::log(llvm::raw_ostream &OS) const { OS << m_message; }

std::error_code MCPError::convertToErrorCode() const {
  return std::error_code(m_error_code, std::generic_category());
}

UnsupportedURI::UnsupportedURI(std::string uri) : m_uri(uri) {}

void UnsupportedURI::log(llvm::raw_ostream &OS) const {
  OS << "unsupported uri: " << m_uri;
}

std::error_code UnsupportedURI::convertToErrorCode() const {
  return llvm::inconvertibleErrorCode();
}
