//===--- SupportedFeatures.h - Supported Features Output --------*- C++ -*-===//
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
// This file provides a high-level API for supported features info
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_SUPPORTEDFEATURES_H
#define SWIFT_SUPPORTEDFEATURES_H

#include "language/Basic/LLVM.h"

namespace language {
namespace features {
void printSupportedFeatures(llvm::raw_ostream &out);
} // namespace features
} // namespace language

#endif
