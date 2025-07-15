//===--- ResilienceExpansion.h ----------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_RESILIENCE_EXPANSION_H
#define LANGUAGE_AST_RESILIENCE_EXPANSION_H

#include "toolchain/Support/raw_ostream.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language {

/// A specification for how much to expand resilient types.
///
/// Right now, this is just a placeholder; a proper expansion
/// specification will probably need to be able to express things like
/// 'expand any type that was fragile in at least such-and-such
/// version'.
enum class ResilienceExpansion : unsigned {
  /// A minimal expansion does not expand types that do not have a
  /// universally fragile representation.  This provides a baseline
  /// for what all components can possibly support.
  ///   - All exported functions must be compiled to at least provide
  ///     a minimally-expanded entrypoint, or else it will be
  ///     impossible for components that do not have that type
  ///     to call the function.
  ///   - Similarly, any sort of abstracted function call must go through
  ///     a minimally-expanded entrypoint.
  ///
  /// Minimal expansion will generally pass all resilient types indirectly.
  Minimal,

  /// A maximal expansion expands all types with fragile
  /// representation, even when they're not universally fragile.  This
  /// is better when internally manipulating values or when working
  /// with specialized entry points for a function.
  Maximal,

  Last_ResilienceExpansion = Maximal
};

inline toolchain::raw_ostream &operator<<(toolchain::raw_ostream &os,
                                     ResilienceExpansion expansion) {
  switch (expansion) {
  case ResilienceExpansion::Minimal:
    return os << "Minimal";
  case ResilienceExpansion::Maximal:
    return os << "Maximal";
  }
  toolchain_unreachable("Unhandled ResilienceExpansion in switch");
}

} // namespace language

#endif // TOOLCHAIN_LANGUAGE_AST_CAPTURE_INFO_H

