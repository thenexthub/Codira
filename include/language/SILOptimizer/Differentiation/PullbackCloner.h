//===--- PullbackCloner.h - Pullback function generation -----*- C++ -*----===//
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
// This file defines a helper class for generating pullback functions for
// automatic differentiation.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_PULLBACKCLONER_H
#define LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_PULLBACKCLONER_H

namespace language {
namespace autodiff {

class VJPCloner;

/// A helper class for generating pullback functions.
class PullbackCloner final {
  class Implementation;
  Implementation &impl;

public:
  /// Creates a pullback cloner from a parent VJP cloner.
  ///
  /// The parent VJP cloner stores the original function and an empty
  /// to-be-generated pullback function.
  explicit PullbackCloner(VJPCloner &vjpCloner);
  ~PullbackCloner();

  /// Performs pullback generation on the empty pullback function. Returns true
  /// if any error occurs.
  bool run();
};

} // end namespace autodiff
} // end namespace language

#endif // LANGUAGE_SILOPTIMIZER_UTILS_DIFFERENTIATION_PULLBACKCLONER_H
