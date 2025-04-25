//===--- DriverTool.h - Driver control ----------------------*- C++ -*-===//
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
// This file provides a high-level API for interacting with the basic
// driver operation.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_DRIVERTOOL_H
#define SWIFT_DRIVERTOOL_H

#include "language/Basic/LLVM.h"

namespace language {
  int mainEntry(int argc_, const char **argv_);
} // namespace language

#endif
