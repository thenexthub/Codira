//===--- SmallBitVector.h -------------------------------------------------===//
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

#ifndef LANGUAGE_BASIC_SMALLBITVECTOR_H
#define LANGUAGE_BASIC_SMALLBITVECTOR_H

#include "toolchain/ADT/SmallBitVector.h"
#include "toolchain/Support/raw_ostream.h"

namespace language {

void printBitsAsArray(toolchain::raw_ostream &OS, const toolchain::SmallBitVector &bits,
                      bool bracketed);

inline toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS,
                                     const toolchain::SmallBitVector &bits) {
  printBitsAsArray(OS, bits, /*bracketed=*/false);
  return OS;
}

void dumpBits(const toolchain::SmallBitVector &bits);

} // namespace language

#endif
