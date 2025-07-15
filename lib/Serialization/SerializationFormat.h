//===--- SerializationFormat.h - Serialization helpers ------ ---*- C++ -*-===//
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
///
/// \file
/// Various helper functions to deal with common serialization functionality.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SERIALIZATION_SERIALIZATIONFORMAT_H
#define LANGUAGE_SERIALIZATION_SERIALIZATIONFORMAT_H

#include "toolchain/ADT/bit.h"
#include "toolchain/Support/Endian.h"

namespace language {

template <typename value_type, typename CharT>
[[nodiscard]] inline value_type readNext(const CharT *&memory) {
  return toolchain::support::endian::readNext<value_type, toolchain::endianness::little, toolchain::support::unaligned>(memory);
}

} // end namespace language

#endif
