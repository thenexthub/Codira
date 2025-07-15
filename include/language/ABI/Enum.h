//===--- Enum.h - Enum implementation runtime declarations ------*- C++ -*-===//
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
// Enum implementation details declarations relevant to the ABI.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_ENUM_H
#define LANGUAGE_ABI_ENUM_H

#include <stdlib.h>

namespace language {

struct EnumTagCounts {
  unsigned numTags, numTagBytes;
};

inline EnumTagCounts
getEnumTagCounts(size_t size, unsigned emptyCases, unsigned payloadCases) {
  // We can use the payload area with a tag bit set somewhere outside of the
  // payload area to represent cases. See how many bytes we need to cover
  // all the empty cases.
  unsigned numTags = payloadCases;
  if (emptyCases > 0) {
    if (size >= 4)
      // Assume that one tag bit is enough if the precise calculation overflows
      // an int32.
      numTags += 1;
    else {
      unsigned bits = size * 8U;
      unsigned casesPerTagBitValue = 1U << bits;
      numTags += ((emptyCases + (casesPerTagBitValue-1U)) >> bits);
    }
  }
  unsigned numTagBytes = (numTags <=    1 ? 0 :
                          numTags <   256 ? 1 :
                          numTags < 65536 ? 2 : 4);
  return {numTags, numTagBytes};
}

} // namespace language

#endif // LANGUAGE_ABI_ENUM_H
