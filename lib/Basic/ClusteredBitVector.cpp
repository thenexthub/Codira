//===--- ClusteredBitVector.cpp - Out-of-line code for the bit vector -----===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file implements support code for ClusteredBitVector.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/ClusteredBitVector.h"

#include "toolchain/Support/raw_ostream.h"

using namespace language;

void ClusteredBitVector::dump() const {
  print(toolchain::errs());
}

/// Pretty-print the vector.
void ClusteredBitVector::print(toolchain::raw_ostream &out) const {
  // Print in 8 clusters of 8 bits per row.
  if (!Bits) {
    return;
  }
  auto &v = Bits.value();
  for (size_t i = 0, e = size(); ; ) {
    out << (v[i++] ? '1' : '0');
    if (i == e) {
      return;
    } else if ((i & 64) == 0) {
      out << '\n';
    } else if ((i & 8) == 0) {
      out << ' ';
    }
  }
}
