//===-- DAPError.cpp ------------------------------------------------------===//
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

#include "DAPError.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>

namespace lldb_dap {

char DAPError::ID;

DAPError::DAPError(std::string message, std::error_code EC, bool show_user,
                   std::optional<std::string> url,
                   std::optional<std::string> url_label)
    : m_message(std::move(message)), m_ec(EC), m_show_user(show_user),
      m_url(std::move(url)), m_url_label(std::move(url_label)) {}

void DAPError::log(llvm::raw_ostream &OS) const { OS << m_message; }

std::error_code DAPError::convertToErrorCode() const { return m_ec; }

char NotStoppedError::ID;

void NotStoppedError::log(llvm::raw_ostream &OS) const { OS << "not stopped"; }

std::error_code NotStoppedError::convertToErrorCode() const {
  return llvm::inconvertibleErrorCode();
}

} // namespace lldb_dap
