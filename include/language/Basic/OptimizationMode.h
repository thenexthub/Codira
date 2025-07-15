//===-------- OptimizationMode.h - Codira optimization modes -----*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_OPTIMIZATIONMODE_H
#define LANGUAGE_BASIC_OPTIMIZATIONMODE_H

#include "language/Basic/InlineBitfield.h"
#include "toolchain/Support/DataTypes.h"

namespace language {

// The optimization mode specified on the command line or with function
// attributes.
enum class OptimizationMode : uint8_t {
  NotSet = 0,
  NoOptimization = 1,  // -Onone
  ForSpeed = 2,        // -Ospeed == -O
  ForSize = 3,         // -Osize
  LastMode = ForSize
};

enum : unsigned { NumOptimizationModeBits =
  countBitsUsed(static_cast<unsigned>(OptimizationMode::LastMode)) };

} // end namespace language

#endif // LANGUAGE_BASIC_OPTIMIZATIONMODE_H
