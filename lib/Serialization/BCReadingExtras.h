//===--- BCReadingExtras.h - Useful helpers for bitcode reading -*- C++ -*-===//
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

#ifndef SWIFT_SERIALIZATION_BCREADINGEXTRAS_H
#define SWIFT_SERIALIZATION_BCREADINGEXTRAS_H

#include "llvm/Bitstream/BitstreamReader.h"

namespace language {
namespace serialization {

/// Saves and restores a BitstreamCursor's bit offset in its stream.
class BCOffsetRAII {
  llvm::BitstreamCursor *Cursor;
  decltype(Cursor->GetCurrentBitNo()) Offset;

public:
  explicit BCOffsetRAII(llvm::BitstreamCursor &cursor)
    : Cursor(&cursor), Offset(cursor.GetCurrentBitNo()) {}

  void reset() {
    if (Cursor)
      Offset = Cursor->GetCurrentBitNo();
  }

  void cancel() {
    Cursor = nullptr;
  }

  ~BCOffsetRAII() {
    if (Cursor)
      cantFail(Cursor->JumpToBit(Offset),
               "BCOffsetRAII must be able to go back");
  }
};

} // end namespace serialization
} // end namespace language

static constexpr const auto AF_DontPopBlockAtEnd =
  llvm::BitstreamCursor::AF_DontPopBlockAtEnd;

#endif
