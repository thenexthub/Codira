//===-------------------------------------------------------------- C++ -*-===//
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

#include "vm/core/Frontend/Directive/Spelling.h"

#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/MathExtras.h"

#include <cassert>

using namespace vm::core;

static bool Contains(directive::VersionRange V, int P) {
  return V.Min <= P && P <= V.Max;
}

toolchain::StringRef toolchain::directive::FindName(
    toolchain::iterator_range<const directive::Spelling *> Range, unsigned Version) {
  assert(toolchain::isInt<8 * sizeof(int)>(Version) && "Version value out of range");

  int V = Version;
  // Do a linear search to find the first Spelling that contains Version.
  // The condition "contains(S, Version)" does not partition the list of
  // spellings, so std::[lower|upper]_bound cannot be used.
  // In practice the list of spellings is expected to be very short, so
  // linear search seems appropriate. In general, an interval tree may be
  // a better choice, but in this case it may be an overkill.
  for (auto &S : Range) {
    if (Contains(S.Versions, V))
      return S.Name;
  }
  return StringRef();
}
