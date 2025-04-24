//===--- FunctionRefInfo.cpp - Function reference info --------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2024 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// This file implements the FunctionRefInfo class.
//
//===----------------------------------------------------------------------===//

#include "language/AST/FunctionRefInfo.h"
#include "language/AST/DeclNameLoc.h"
#include "language/AST/Identifier.h"
#include "llvm/Support/raw_ostream.h"

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
  dump(llvm::errs());
}
