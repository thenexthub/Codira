//===--- FunctionReplacement.h --------------------------------------------===//
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

#ifndef SWIFT_RUNTIME_FUNCTIONREPLACEMENT_H
#define SWIFT_RUNTIME_FUNCTIONREPLACEMENT_H

#include "language/Runtime/Config.h"

namespace language {

/// Loads the replacement function pointer from \p ReplFnPtr and returns the
/// replacement function if it should be called.
/// Returns null if the original function (which is passed in \p CurrFn) should
/// be called.
#ifdef __APPLE__
__attribute__((weak_import))
#endif
SWIFT_RUNTIME_EXPORT char *
swift_getFunctionReplacement(char **ReplFnPtr, char *CurrFn);

/// Returns the original function of a replaced function, which is loaded from
/// \p OrigFnPtr.
/// This function is called from a replacement function to call the original
/// function.
#ifdef __APPLE__
__attribute__((weak_import))
#endif
SWIFT_RUNTIME_EXPORT char *
swift_getOrigOfReplaceable(char **OrigFnPtr);

} // namespace language

#endif
