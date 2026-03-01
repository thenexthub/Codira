//===-- UriParser.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_URIPARSER_H
#define LLDB_UTILITY_URIPARSER_H

#include "llvm/ADT/StringRef.h"
#include <optional>

namespace llvm {
class raw_ostream;
} // namespace llvm

namespace lldb_private {

struct URI {
  llvm::StringRef scheme;
  llvm::StringRef hostname;
  std::optional<uint16_t> port;
  llvm::StringRef path;

  bool operator==(const URI &R) const {
    return port == R.port && scheme == R.scheme && hostname == R.hostname &&
           path == R.path;
  }

  static std::optional<URI> Parse(llvm::StringRef uri);
};

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const URI &U);

} // namespace lldb_private

#endif // LLDB_UTILITY_URIPARSER_H
