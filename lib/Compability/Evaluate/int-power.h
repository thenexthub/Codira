//===-- lib/Evaluate/int-power.h --------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LANGUAGE_COMPABILITY_EVALUATE_INT_POWER_H_
#define LANGUAGE_COMPABILITY_EVALUATE_INT_POWER_H_

// Computes an integer power of a real or complex value.

#include "language/Compability/Evaluate/target.h"

namespace language::Compability::evaluate {

template <typename REAL, typename INT>
ValueWithRealFlags<REAL> TimesIntPowerOf(const REAL &factor, const REAL &base,
    const INT &power,
    Rounding rounding = TargetCharacteristics::defaultRounding) {
  ValueWithRealFlags<REAL> result{factor};
  if (base.IsNotANumber()) {
    result.value = REAL::NotANumber();
    result.flags.set(RealFlag::InvalidArgument);
  } else if (power.IsZero()) {
    if (base.IsZero() || base.IsInfinite()) {
      result.flags.set(RealFlag::InvalidArgument);
    }
  } else {
    bool negativePower{power.IsNegative()};
    INT absPower{power.ABS().value};
    REAL squares{base};
    int nbits{INT::bits - absPower.LEADZ()};
    for (int j{0}; j < nbits; ++j) {
      if (j > 0) { // avoid spurious overflow on last iteration
        squares =
            squares.Multiply(squares, rounding).AccumulateFlags(result.flags);
      }
      if (absPower.BTEST(j)) {
        if (negativePower) {
          result.value = result.value.Divide(squares, rounding)
                             .AccumulateFlags(result.flags);
        } else {
          result.value = result.value.Multiply(squares, rounding)
                             .AccumulateFlags(result.flags);
        }
      }
    }
  }
  return result;
}

template <typename REAL, typename INT>
ValueWithRealFlags<REAL> IntPower(const REAL &base, const INT &power,
    Rounding rounding = TargetCharacteristics::defaultRounding) {
  REAL one{REAL::FromInteger(INT{1}).value};
  return TimesIntPowerOf(one, base, power, rounding);
}
} // namespace language::Compability::evaluate
#endif // FORTRAN_EVALUATE_INT_POWER_H_
