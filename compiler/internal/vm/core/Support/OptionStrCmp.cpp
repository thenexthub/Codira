//===- OptionStrCmp.cpp - Option String Comparison --------------*- C++ -*-===//
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

#include "vm/core/Support/OptionStrCmp.h"
#include "vm/core/ADT/STLExtras.h"

using namespace vm::core;

// Comparison function for Option strings (option names & prefixes).
// The ordering is *almost* case-insensitive lexicographic, with an exception.
// '\0' comes at the end of the alphabet instead of the beginning (thus options
// precede any other options which prefix them). Additionally, if two options
// are identical ignoring case, they are ordered according to case sensitive
// ordering if `FallbackCaseSensitive` is true.
int toolchain::StrCmpOptionName(StringRef A, StringRef B,
                           bool FallbackCaseSensitive) {
  size_t MinSize = std::min(A.size(), B.size());
  if (int Res = A.substr(0, MinSize).compare_insensitive(B.substr(0, MinSize)))
    return Res;

  // If they are identical ignoring case, use case sensitive ordering.
  if (A.size() == B.size())
    return FallbackCaseSensitive ? A.compare(B) : 0;

  return (A.size() == MinSize) ? 1 /* A is a prefix of B. */
                               : -1 /* B is a prefix of A */;
}

// Comparison function for Option prefixes.
int toolchain::StrCmpOptionPrefixes(ArrayRef<StringRef> APrefixes,
                               ArrayRef<StringRef> BPrefixes) {
  for (const auto &[APre, BPre] : zip(APrefixes, BPrefixes)) {
    if (int Cmp = StrCmpOptionName(APre, BPre))
      return Cmp;
  }
  // Both prefixes are identical.
  return 0;
}
