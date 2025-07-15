//===--- SILUndef.cpp -----------------------------------------------------===//
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

#include "language/SIL/SILUndef.h"
#include "language/SIL/SILModule.h"

using namespace language;

SILUndef::SILUndef(SILFunction *parent, SILType type)
    : ValueBase(ValueKind::SILUndef, type), parent(parent) {}

SILUndef *SILUndef::get(SILFunction *fn, SILType ty) {
  SILUndef *&entry = fn->undefValues[ty];
  if (entry == nullptr)
    entry = new (fn->getModule()) SILUndef(fn, ty);
  return entry;
}
