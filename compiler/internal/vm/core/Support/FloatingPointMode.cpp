//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/ADT/FloatingPointMode.h"
#include "vm/core/ADT/StringExtras.h"

using namespace vm::core;

FPClassTest toolchain::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  if (Mask & fcNegInf)
    NewMask |= fcPosInf;
  if (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  if (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  if (Mask & fcNegZero)
    NewMask |= fcPosZero;
  if (Mask & fcPosZero)
    NewMask |= fcNegZero;
  if (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  if (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  if (Mask & fcPosInf)
    NewMask |= fcNegInf;
  return NewMask;
}

FPClassTest toolchain::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  if (Mask & fcPosZero)
    NewMask |= fcZero;
  if (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  if (Mask & fcPosNormal)
    NewMask |= fcNormal;
  if (Mask & fcPosInf)
    NewMask |= fcInf;
  return NewMask;
}

FPClassTest toolchain::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  if (Mask & fcZero)
    NewMask |= fcZero;
  if (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  if (Mask & fcNormal)
    NewMask |= fcNormal;
  if (Mask & fcInf)
    NewMask |= fcInf;
  return NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
static constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
  {fcAllFlags, "all"},
  {fcNan, "nan"},
  {fcSNan, "snan"},
  {fcQNan, "qnan"},
  {fcInf, "inf"},
  {fcNegInf, "ninf"},
  {fcPosInf, "pinf"},
  {fcZero, "zero"},
  {fcNegZero, "nzero"},
  {fcPosZero, "pzero"},
  {fcSubnormal, "sub"},
  {fcNegSubnormal, "nsub"},
  {fcPosSubnormal, "psub"},
  {fcNormal, "norm"},
  {fcNegNormal, "nnorm"},
  {fcPosNormal, "pnorm"}
};

raw_ostream &toolchain::operator<<(raw_ostream &OS, FPClassTest Mask) {
  OS << '(';

  if (Mask == fcNone) {
    OS << "none)";
    return OS;
  }

  ListSeparator LS(" ");
  for (auto [BitTest, Name] : NoFPClassName) {
    if ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  return OS;
}

static bool cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           bool OrEqual, bool OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  if (LHS == fcNone || RHS == fcNone)
    return true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  if (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    if (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    if (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  if (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    return true;
  }

  if (LowestBitRHS < HighestBitLHS)
    return false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  return !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool toolchain::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  return cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool toolchain::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  return cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool toolchain::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  return cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool toolchain::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  return cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
}
