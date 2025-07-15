//===-- toolchain/Support/DataTypes.h - Define fixed size types ------*- C++ -*-===//
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
// Due to layering constraints (Support depends on toolchain-c) this is a thin
// wrapper around the implementation that lives in toolchain-c, though most clients
// can/should think of this as being provided by Support for simplicity (not
// many clients are aware of their dependency on toolchain-c).
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SUPPORT_DATATYPES_H
#define TOOLCHAIN_SUPPORT_DATATYPES_H

#include "toolchain-c/DataTypes.h"

#endif // TOOLCHAIN_SUPPORT_DATATYPES_H
