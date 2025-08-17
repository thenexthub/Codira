//===-- lib/Semantics/attr.cpp --------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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

#include "language/Compability/Semantics/attr.h"
#include "language/Compability/Common/idioms.h"
#include "toolchain/Support/raw_ostream.h"
#include <stddef.h>

namespace language::Compability::semantics {

void Attrs::CheckValid(const Attrs &allowed) const {
  if (!allowed.HasAll(*this)) {
    common::die("invalid attribute");
  }
}

std::string AttrToString(Attr attr) {
  switch (attr) {
  case Attr::BIND_C:
    return "BIND(C)";
  case Attr::INTENT_IN:
    return "INTENT(IN)";
  case Attr::INTENT_INOUT:
    return "INTENT(INOUT)";
  case Attr::INTENT_OUT:
    return "INTENT(OUT)";
  default:
    return std::string{EnumToString(attr)};
  }
}

toolchain::raw_ostream &operator<<(toolchain::raw_ostream &o, Attr attr) {
  return o << AttrToString(attr);
}

toolchain::raw_ostream &operator<<(toolchain::raw_ostream &o, const Attrs &attrs) {
  std::size_t n{attrs.count()};
  std::size_t seen{0};
  for (std::size_t j{0}; seen < n; ++j) {
    Attr attr{static_cast<Attr>(j)};
    if (attrs.test(attr)) {
      if (seen > 0) {
        o << ", ";
      }
      o << attr;
      ++seen;
    }
  }
  return o;
}
} // namespace language::Compability::semantics
