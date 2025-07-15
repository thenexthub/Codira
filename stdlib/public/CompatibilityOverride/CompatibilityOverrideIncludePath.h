//===--- CompatibilityOverrideIncludePath.h -------------------------------===//
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
///
/// \file This file should be included instead of including
/// COMPATIBILITY_OVERRIDE_INCLUDE_PATH directly to ensure that syntax
/// highlighting in certain errors is not broken. It is assumed that
/// CompatibilityOverride.h is already included.
///
//===----------------------------------------------------------------------===//

#ifndef COMPATIBILITY_OVERRIDE_H
#error "Must define COMPATIBILITY_OVERRIDE_H before including this file"
#endif

#include COMPATIBILITY_OVERRIDE_INCLUDE_PATH
