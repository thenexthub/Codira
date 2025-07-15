//===----------------------------------------------------------------------===//
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

#pragma once

#include <stdbool.h>
#include <sys/wait.h>

static inline
bool wIfStopped(int status) {
  return WIFSTOPPED(status) != 0;
}

static inline
bool wIfExited(int status) {
  return WIFEXITED(status) != 0;
}

static inline
bool wIfSignaled(int status) {
  return WIFSIGNALED(status) != 0;
}

static inline
int wStopSig(int status) {
  return WSTOPSIG(status);
}
