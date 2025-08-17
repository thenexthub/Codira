//===-- language/Compability/Testing/fp-testing.h ----------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_TESTING_FP_TESTING_H_
#define LANGUAGE_COMPABILITY_TESTING_FP_TESTING_H_

#include "language/Compability/Common/target-rounding.h"
#include <fenv.h>

using language::Compability::common::RealFlags;
using language::Compability::common::Rounding;
using language::Compability::common::RoundingMode;

class ScopedHostFloatingPointEnvironment {
public:
  ScopedHostFloatingPointEnvironment(bool treatSubnormalOperandsAsZero = false,
      bool flushSubnormalResultsToZero = false);
  ~ScopedHostFloatingPointEnvironment();
  void ClearFlags() const;
  static RealFlags CurrentFlags();
  static void SetRounding(Rounding rounding);

private:
  fenv_t originalFenv_;
#if __x86_64__ || _M_X64
  unsigned int originalMxcsr;
#endif
};

#endif // FORTRAN_TESTING_FP_TESTING_H_
