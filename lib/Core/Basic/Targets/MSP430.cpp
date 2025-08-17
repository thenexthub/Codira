//===--- MSP430.cpp - Implement MSP430 target feature support -------------===//
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
//
// This file implements MSP430 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "MSP430.h"
#include "language/Core/Basic/MacroBuilder.h"

using namespace language::Core;
using namespace language::Core::targets;

const char *const MSP430TargetInfo::GCCRegNames[] = {
    "r0", "r1", "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

ArrayRef<const char *> MSP430TargetInfo::getGCCRegNames() const {
  return toolchain::ArrayRef(GCCRegNames);
}

void MSP430TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  Builder.defineMacro("MSP430");
  Builder.defineMacro("__MSP430__");
  // FIXME: defines for different 'flavours' of MCU
}
