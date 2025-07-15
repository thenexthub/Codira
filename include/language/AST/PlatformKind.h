//===--- AST/PlatformKind.h - Codira Language Platform Kinds -----*- C++ -*-===//
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
/// This file defines the platform kinds for API availability.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_PLATFORM_KIND_H
#define LANGUAGE_AST_PLATFORM_KIND_H

/// This header is included in a bridging header. Be *very* careful with what
/// you include here! See include caveats in `ASTBridging.h`.
#include "language/Basic/LanguageBridging.h"
#include <stdint.h>

namespace language {

/// Available platforms for the availability attribute.
enum class ENUM_EXTENSIBILITY_ATTR(closed) PlatformKind : uint8_t {
  none,
#define AVAILABILITY_PLATFORM(X, PrettyName) X,
#include "language/AST/PlatformKinds.def"
};

} // end namespace language

#endif // LANGUAGE_AST_PLATFORM_KIND_H
