//===--- CompatibilityOverride.h - Back-deploying compatibility fixes --*- C++ -*-===//
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
// Support back-deploying compatibility fixes for newer apps running on older runtimes.
//
//===----------------------------------------------------------------------===//

#ifndef COMPATIBILITY_OVERRIDE_H
#define COMPATIBILITY_OVERRIDE_H

#include "../../public/runtime/Private.h"
#include "language/Runtime/Metadata.h"
#include "language/Runtime/Once.h"
#include <type_traits>

namespace language {

#define COMPATIBILITY_UNPAREN(...) __VA_ARGS__

#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  ccAttrs typedef ret (*Original_ ## name) typedArgs;
#include "CompatibilityOverride.def"

#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  ccAttrs typedef ret (*Override_ ## name)(COMPATIBILITY_UNPAREN typedArgs, \
                                           Original_ ## name originalImpl);
#include "CompatibilityOverride.def"

#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  Override_ ## name getOverride_ ## name();
#include "CompatibilityOverride.def"


/// Used to define an override point. The override point #defines the appropriate
/// OVERRIDE macro from CompatibilityOverride.def to this macro, then includes
/// the file to generate the override points. The original implementation of the
/// functionality must be available as language_funcNameHereImpl.
#define COMPATIBILITY_OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs) \
  attrs ccAttrs ret namespace language_ ## name typedArgs {                          \
    static Override_ ## name Override;                                            \
    static language_once_t Predicate;                                                \
    language_once(&Predicate, [](void *) {                                           \
      Override = getOverride_ ## name();                                          \
    }, nullptr);                                                                  \
    if (Override != nullptr)                                                      \
      return Override(COMPATIBILITY_UNPAREN namedArgs, language_ ## name ## Impl);   \
    return language_ ## name ## Impl namedArgs; \
  }

} /* end namespace language */

#endif /* COMPATIBILITY_OVERRIDE_H */
