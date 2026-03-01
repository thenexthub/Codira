//===-- RegularExpression.cpp ---------------------------------------------===//
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

#include "lldb/Utility/RegularExpression.h"

#include <string>

using namespace lldb_private;

RegularExpression::RegularExpression(llvm::StringRef str,
                                     llvm::Regex::RegexFlags flags)
    : m_regex_text(std::string(str)),
      // m_regex does not reference str anymore after it is constructed.
      m_regex(llvm::Regex(str, flags)) {}

RegularExpression::RegularExpression(const RegularExpression &rhs)
    : RegularExpression(rhs.GetText()) {}

bool RegularExpression::Execute(
    llvm::StringRef str,
    llvm::SmallVectorImpl<llvm::StringRef> *matches) const {
  if (!IsValid())
    return false;
  return m_regex.match(str, matches);
}

bool RegularExpression::IsValid() const { return m_regex.isValid(); }

llvm::StringRef RegularExpression::GetText() const { return m_regex_text; }

llvm::Error RegularExpression::GetError() const {
  std::string error;
  if (!m_regex.isValid(error))
    return llvm::make_error<llvm::StringError>(error,
                                               llvm::inconvertibleErrorCode());
  return llvm::Error::success();
}
