//===--- Exclusivity.cpp --------------------------------------------------===//
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
// This implements the runtime support for dynamically tracking exclusivity.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Exclusivity.h"
#include "language/Basic/Lazy.h"

#include <dlfcn.h>

void language::language_task_enterThreadLocalContextBackdeploy56(char *state) {
  const auto enterThreadLocalContext =
      reinterpret_cast<void(*)(char *state)>(LANGUAGE_LAZY_CONSTANT(
          dlsym(RTLD_DEFAULT, "language_task_enterThreadLocalContext")));
  if (enterThreadLocalContext)
    enterThreadLocalContext(state);
}

void language::language_task_exitThreadLocalContextBackdeploy56(char *state) {
  const auto exitThreadLocalContext =
      reinterpret_cast<void(*)(char *state)>(LANGUAGE_LAZY_CONSTANT(
          dlsym(RTLD_DEFAULT, "language_task_exitThreadLocalContext")));
  if (exitThreadLocalContext)
    exitThreadLocalContext(state);
}
