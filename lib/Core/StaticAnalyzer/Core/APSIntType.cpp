//===--- APSIntType.cpp - Simple record of the type of APSInts ------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/PathSensitive/APSIntType.h"

using namespace language::Core;
using namespace ento;

APSIntType::RangeTestResultKind
APSIntType::testInRange(const toolchain::APSInt &Value,
                        bool AllowSignConversions) const {

  // Negative numbers cannot be losslessly converted to unsigned type.
  if (IsUnsigned && !AllowSignConversions &&
      Value.isSigned() && Value.isNegative())
    return RTR_Below;

  unsigned MinBits;
  if (AllowSignConversions) {
    if (Value.isSigned() && !IsUnsigned)
      MinBits = Value.getSignificantBits();
    else
      MinBits = Value.getActiveBits();

  } else {
    // Signed integers can be converted to signed integers of the same width
    // or (if positive) unsigned integers with one fewer bit.
    // Unsigned integers can be converted to unsigned integers of the same width
    // or signed integers with one more bit.
    if (Value.isSigned())
      MinBits = Value.getSignificantBits() - IsUnsigned;
    else
      MinBits = Value.getActiveBits() + !IsUnsigned;
  }

  if (MinBits <= BitWidth)
    return RTR_Within;

  if (Value.isSigned() && Value.isNegative())
    return RTR_Below;
  else
    return RTR_Above;
}
