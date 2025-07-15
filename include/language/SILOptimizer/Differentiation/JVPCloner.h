//===--- JVPCloner.h - JVP function generation ----------------*- C++ -*---===//
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
// This file defines a helper class for generating JVP functions for automatic
// differentiation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_JVPCLONER_H
#define LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_JVPCLONER_H

#include "language/SILOptimizer/Differentiation/DifferentiationInvoker.h"

namespace language {

class SILFunction;
class SILDifferentiabilityWitness;

namespace autodiff {

class ADContext;

/// A helper class for generating JVP functions.
class JVPCloner final {
  class Implementation;
  Implementation &impl;

public:
  /// Creates a JVP cloner.
  ///
  /// The parent JVP cloner stores the original function and an empty
  /// to-be-generated pullback function.
  explicit JVPCloner(ADContext &context, SILDifferentiabilityWitness *witness,
                     SILFunction *jvp, DifferentiationInvoker invoker);
  ~JVPCloner();

  /// Performs JVP generation on the empty JVP function. Returns true if any
  /// error occurs.
  bool run();

  SILFunction &getJVP() const;
};

} // end namespace autodiff
} // end namespace language

#endif // LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_JVPCLONER_H
