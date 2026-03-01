//===-- ResponseHandler.cpp -----------------------------------------------===//
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

#include "ResponseHandler.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

namespace lldb_dap {

void UnknownResponseHandler::operator()(
    llvm::Expected<llvm::json::Value> value) const {
  llvm::errs() << "unexpected response: ";
  if (value) {
    if (std::optional<llvm::StringRef> str = value->getAsString())
      llvm::errs() << *str;
  } else {
    llvm::errs() << "error: " << llvm::toString(value.takeError());
  }
  llvm::errs() << '\n';
}

void LogFailureResponseHandler::operator()(
    llvm::Expected<llvm::json::Value> value) const {
  if (!value)
    llvm::errs() << "reverse request \"" << m_command << "\" (" << m_id
                 << ") failed: " << llvm::toString(value.takeError()) << '\n';
}

} // namespace lldb_dap
