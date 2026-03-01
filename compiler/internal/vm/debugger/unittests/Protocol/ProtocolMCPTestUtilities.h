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

#ifndef LLDB_UNITTESTS_PROTOCOL_PROTOCOLMCPTESTUTILITIES_H
#define LLDB_UNITTESTS_PROTOCOL_PROTOCOLMCPTESTUTILITIES_H

#include "lldb/Protocol/MCP/Protocol.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/JSON.h" // IWYU pragma: keep
#include "gtest/gtest.h"       // IWYU pragma: keep
#include <ostream>
#include <variant>

namespace lldb_protocol::mcp {

inline void PrintTo(const Request &req, std::ostream *os) {
  *os << llvm::formatv("{0}", toJSON(req)).str();
}

inline void PrintTo(const Response &resp, std::ostream *os) {
  *os << llvm::formatv("{0}", toJSON(resp)).str();
}

inline void PrintTo(const Notification &note, std::ostream *os) {
  *os << llvm::formatv("{0}", toJSON(note)).str();
}

inline void PrintTo(const Message &message, std::ostream *os) {
  return std::visit([os](auto &&message) { return PrintTo(message, os); },
                    message);
}

} // namespace lldb_protocol::mcp

#endif
