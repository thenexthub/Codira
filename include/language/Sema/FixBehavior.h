//===--- FixBehavior.h - Constraint Fix Behavior --------------------------===//
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
// This file provides information about how a constraint fix should behavior.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SEMA_FIXBEHAVIOR_H
#define SWIFT_SEMA_FIXBEHAVIOR_H

namespace language {
namespace constraints {

/// Describes the behavior of the diagnostic corresponding to a given fix.
enum class FixBehavior {
  /// The diagnostic is an error, and should prevent constraint application.
  Error,
  /// The diagnostic is always a warning, which should not prevent constraint
  /// application.
  AlwaysWarning,
  /// The diagnostic should be downgraded to a warning, and not prevent
  /// constraint application.
  DowngradeToWarning,
  /// The diagnostic should be suppressed, and not prevent constraint
  /// application.
  Suppress
};

}
}

#endif // SWIFT_SEMA_FIXBEHAVIOR_H
