//===-- language/Compability/Common/target-rounding.h ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_COMMON_TARGET_ROUNDING_H_
#define LANGUAGE_COMPABILITY_COMMON_TARGET_ROUNDING_H_

#include "Fortran-consts.h"
#include "enum-set.h"

namespace language::Compability::common {

// Floating-point rounding control
struct Rounding {
  common::RoundingMode mode{common::RoundingMode::TiesToEven};
  // When set, emulate status flag behavior peculiar to x86
  // (viz., fail to set the Underflow flag when an inexact product of a
  // multiplication is rounded up to a normal number from a subnormal
  // in some rounding modes)
#if __x86_64__ || _M_X64 || __riscv || __loongarch__
  bool x86CompatibleBehavior{true};
#else
  bool x86CompatibleBehavior{false};
#endif
};

// These are ordered like the bits in a common fenv.h header file.
ENUM_CLASS(RealFlag, InvalidArgument, Denorm, DivideByZero, Overflow, Underflow,
    Inexact)
using RealFlags = common::EnumSet<RealFlag, RealFlag_enumSize>;

} // namespace language::Compability::common
#endif /* FORTRAN_COMMON_TARGET_ROUNDING_H_ */
