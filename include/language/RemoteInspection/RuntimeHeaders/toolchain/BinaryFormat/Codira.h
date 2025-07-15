//===-- toolchain/BinaryFormat/Codira.h ---Codira Constants-------------*- C++ -*-===//
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

#ifndef TOOLCHAIN_BINARYFORMAT_LANGUAGE_H
#define TOOLCHAIN_BINARYFORMAT_LANGUAGE_H

namespace toolchain {
namespace binaryformat {

enum Codira5ReflectionSectionKind {
#define HANDLE_LANGUAGE_SECTION(KIND, MACHO, ELF, COFF) KIND,
#include "toolchain/BinaryFormat/Codira.def"
#undef HANDLE_LANGUAGE_SECTION
  unknown,
  last = unknown
};
} // end of namespace binaryformat
} // end of namespace toolchain

#endif
