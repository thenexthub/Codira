//===-- DAPError.h --------------------------------------------------------===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_DAPERROR_H
#define LLDB_TOOLS_LLDB_DAP_DAPERROR_H

#include "llvm/Support/Error.h"
#include <optional>
#include <string>
#include <system_error>

namespace lldb_dap {

/// An error that is reported as a DAP Error Message, which may be presented to
/// the user.
class DAPError : public llvm::ErrorInfo<DAPError> {
public:
  static char ID;

  DAPError(std::string message,
           std::error_code EC = llvm::inconvertibleErrorCode(),
           bool show_user = true, std::optional<std::string> url = std::nullopt,
           std::optional<std::string> url_label = std::nullopt);

  void log(llvm::raw_ostream &OS) const override;
  std::error_code convertToErrorCode() const override;

  const std::string &getMessage() const { return m_message; }
  bool getShowUser() const { return m_show_user; }
  const std::optional<std::string> &getURL() const { return m_url; }
  const std::optional<std::string> &getURLLabel() const { return m_url_label; }

private:
  std::string m_message;
  std::error_code m_ec;
  bool m_show_user;
  std::optional<std::string> m_url;
  std::optional<std::string> m_url_label;
};

/// An error that indicates the current request handler cannot execute because
/// the process is not stopped.
class NotStoppedError : public llvm::ErrorInfo<NotStoppedError> {
public:
  static char ID;
  void log(llvm::raw_ostream &OS) const override;
  std::error_code convertToErrorCode() const override;
};

} // namespace lldb_dap

#endif // LLDB_TOOLS_LLDB_DAP_DAPERROR_H
