//===-- JSONTransport.cpp -------------------------------------------------===//
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

#include "lldb/Host/JSONTransport.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Status.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

using namespace llvm;
using namespace lldb_private::transport;

char TransportUnhandledContentsError::ID;

TransportUnhandledContentsError::TransportUnhandledContentsError(
    std::string unhandled_contents)
    : m_unhandled_contents(unhandled_contents) {}

void TransportUnhandledContentsError::log(raw_ostream &OS) const {
  OS << "transport EOF with unhandled contents: '" << m_unhandled_contents
     << "'";
}
std::error_code TransportUnhandledContentsError::convertToErrorCode() const {
  return std::make_error_code(std::errc::bad_message);
}

char InvalidParams::ID;

void InvalidParams::log(raw_ostream &OS) const {
  OS << "invalid parameters for method '" << m_method << "': '" << m_context
     << "'";
}
std::error_code InvalidParams::convertToErrorCode() const {
  return std::make_error_code(std::errc::invalid_argument);
}

char MethodNotFound::ID;

void MethodNotFound::log(raw_ostream &OS) const {
  OS << "method not found: '" << m_method << "'";
}

std::error_code MethodNotFound::convertToErrorCode() const {
  // JSON-RPC Method not found
  return std::error_code(MethodNotFound::kErrorCode, std::generic_category());
}
