//===-- language/Compability/Support/default-kinds.h -------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_SUPPORT_DEFAULT_KINDS_H_
#define LANGUAGE_COMPABILITY_SUPPORT_DEFAULT_KINDS_H_

#include "Fortran.h"
#include <cstdint>

namespace language::Compability::common {

// All address calculations in generated code are 64-bit safe.
// Compile-time folding of bounds, subscripts, and lengths
// consequently uses 64-bit signed integers.  The name reflects
// this usage as a subscript into a constant array.
using ConstantSubscript = std::int64_t;

// Represent the default values of the kind parameters of the
// various intrinsic types.  Most of these can be configured by
// means of the compiler command line.
class IntrinsicTypeDefaultKinds {
public:
  IntrinsicTypeDefaultKinds();
  int subscriptIntegerKind() const { return subscriptIntegerKind_; }
  int sizeIntegerKind() const { return sizeIntegerKind_; }
  int doublePrecisionKind() const { return doublePrecisionKind_; }
  int quadPrecisionKind() const { return quadPrecisionKind_; }

  IntrinsicTypeDefaultKinds &set_defaultIntegerKind(int);
  IntrinsicTypeDefaultKinds &set_subscriptIntegerKind(int);
  IntrinsicTypeDefaultKinds &set_sizeIntegerKind(int);
  IntrinsicTypeDefaultKinds &set_defaultRealKind(int);
  IntrinsicTypeDefaultKinds &set_doublePrecisionKind(int);
  IntrinsicTypeDefaultKinds &set_quadPrecisionKind(int);
  IntrinsicTypeDefaultKinds &set_defaultCharacterKind(int);
  IntrinsicTypeDefaultKinds &set_defaultLogicalKind(int);

  int GetDefaultKind(TypeCategory) const;

private:
  // Default REAL just simply has to be IEEE-754 single precision today.
  // It occupies one numeric storage unit by definition.  The default INTEGER
  // and default LOGICAL intrinsic types also have to occupy one numeric
  // storage unit, so their kinds are also forced.  Default COMPLEX must always
  // comprise two default REAL components.
  int defaultIntegerKind_{4};
  int subscriptIntegerKind_{8};
  int sizeIntegerKind_{4}; // SIZE(), UBOUND(), &c. default KIND=
  int defaultRealKind_{defaultIntegerKind_};
  int doublePrecisionKind_{2 * defaultRealKind_};
  int quadPrecisionKind_{2 * doublePrecisionKind_};
  int defaultCharacterKind_{1};
  int defaultLogicalKind_{defaultIntegerKind_};
};
} // namespace language::Compability::common
#endif // FORTRAN_SUPPORT_DEFAULT_KINDS_H_
