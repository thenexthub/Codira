//===-- flang/lib/Semantics/openmp-dsa.cpp ----------------------*- C++ -*-===//
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

#include "language/Compability/Semantics/openmp-dsa.h"

namespace language::Compability::semantics {

Symbol::Flags GetSymbolDSA(const Symbol &symbol) {
  Symbol::Flags dsaFlags{Symbol::Flag::OmpPrivate,
      Symbol::Flag::OmpFirstPrivate, Symbol::Flag::OmpLastPrivate,
      Symbol::Flag::OmpShared, Symbol::Flag::OmpLinear,
      Symbol::Flag::OmpReduction};
  Symbol::Flags dsa{symbol.flags() & dsaFlags};
  if (dsa.any()) {
    return dsa;
  }
  // If no DSA are set use those from the host associated symbol, if any.
  if (const auto *details{symbol.detailsIf<HostAssocDetails>()}) {
    return GetSymbolDSA(details->symbol());
  }
  return {};
}

} // namespace language::Compability::semantics
