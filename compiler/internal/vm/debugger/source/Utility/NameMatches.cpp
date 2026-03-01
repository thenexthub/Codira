//===-- NameMatches.cpp ---------------------------------------------------===//
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
#include "lldb/Utility/NameMatches.h"
#include "lldb/Utility/RegularExpression.h"

#include "llvm/ADT/StringRef.h"

using namespace lldb_private;

bool lldb_private::NameMatches(llvm::StringRef name, NameMatch match_type,
                               llvm::StringRef match) {
  switch (match_type) {
  case NameMatch::Ignore:
    return true;
  case NameMatch::Equals:
    return name == match;
  case NameMatch::Contains:
    return name.contains(match);
  case NameMatch::StartsWith:
    return name.starts_with(match);
  case NameMatch::EndsWith:
    return name.ends_with(match);
  case NameMatch::RegularExpression: {
    RegularExpression regex(match);
    return regex.Execute(name);
  }
  }
  return false;
}
