//===-- ResponseHandler.h -------------------------------------------------===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_HANDLER_RESPONSEHANDLER_H
#define LLDB_TOOLS_LLDB_DAP_HANDLER_RESPONSEHANDLER_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/JSON.h"
#include <cstdint>

namespace lldb_dap {
struct DAP;

/// Handler for responses to reverse requests.
class ResponseHandler {
public:
  ResponseHandler(llvm::StringRef command, int64_t id)
      : m_command(command), m_id(id) {}

  /// ResponseHandlers are not copyable.
  /// @{
  ResponseHandler(const ResponseHandler &) = delete;
  ResponseHandler &operator=(const ResponseHandler &) = delete;
  /// @}

  virtual ~ResponseHandler() = default;

  virtual void operator()(llvm::Expected<llvm::json::Value> value) const = 0;

protected:
  llvm::StringRef m_command;
  int64_t m_id;
};

/// Response handler used for unknown responses.
class UnknownResponseHandler : public ResponseHandler {
public:
  using ResponseHandler::ResponseHandler;
  void operator()(llvm::Expected<llvm::json::Value> value) const override;
};

/// Response handler which logs to stderr in case of a failure.
class LogFailureResponseHandler : public ResponseHandler {
public:
  using ResponseHandler::ResponseHandler;
  void operator()(llvm::Expected<llvm::json::Value> value) const override;
};

} // namespace lldb_dap

#endif
