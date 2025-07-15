//===--- FunctionRefInfo.cpp - Function reference info --------------------===//
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
// This file implements the FunctionRefInfo class.
//
//===----------------------------------------------------------------------===//

#include "language/AST/FunctionRefInfo.h"
#include "language/AST/DeclNameLoc.h"
#include "language/AST/Identifier.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

FunctionRefInfo FunctionRefInfo::unapplied(DeclNameLoc nameLoc) {
  return FunctionRefInfo(ApplyLevel::Unapplied, nameLoc.isCompound());
}

FunctionRefInfo FunctionRefInfo::unapplied(DeclNameRef nameRef) {
  return FunctionRefInfo(ApplyLevel::Unapplied, nameRef.isCompoundName());
}

FunctionRefInfo FunctionRefInfo::addingApplicationLevel() const {
  auto withApply = [&]() {
    switch (getApplyLevel()) {
    case ApplyLevel::Unapplied:
      return ApplyLevel::SingleApply;
    case ApplyLevel::SingleApply:
    case ApplyLevel::DoubleApply:
      return ApplyLevel::DoubleApply;
    }
  };
  return FunctionRefInfo(withApply(), isCompoundName());
}

void FunctionRefInfo::dump(raw_ostream &os) const {
  switch (getApplyLevel()) {
  case ApplyLevel::Unapplied:
    os << "unapplied";
    break;
  case ApplyLevel::SingleApply:
    os << "single apply";
    break;
  case ApplyLevel::DoubleApply:
    os << "double apply";
    break;
  }
  if (isCompoundName())
    os << " (compound)";
}

void FunctionRefInfo::dump() const {
  dump(toolchain::errs());
}
