//===--- XRayInstr.cpp ------------------------------------------*- C++ -*-===//
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
// This is part of XRay, a function call instrumentation system.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/XRayInstr.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringSwitch.h"

namespace language::Core {

XRayInstrMask parseXRayInstrValue(StringRef Value) {
  XRayInstrMask ParsedKind =
      toolchain::StringSwitch<XRayInstrMask>(Value)
          .Case("all", XRayInstrKind::All)
          .Case("custom", XRayInstrKind::Custom)
          .Case("function",
                XRayInstrKind::FunctionEntry | XRayInstrKind::FunctionExit)
          .Case("function-entry", XRayInstrKind::FunctionEntry)
          .Case("function-exit", XRayInstrKind::FunctionExit)
          .Case("typed", XRayInstrKind::Typed)
          .Case("none", XRayInstrKind::None)
          .Default(XRayInstrKind::None);
  return ParsedKind;
}

void serializeXRayInstrValue(XRayInstrSet Set,
                             SmallVectorImpl<StringRef> &Values) {
  if (Set.Mask == XRayInstrKind::All) {
    Values.push_back("all");
    return;
  }

  if (Set.Mask == XRayInstrKind::None) {
    Values.push_back("none");
    return;
  }

  if (Set.has(XRayInstrKind::Custom))
    Values.push_back("custom");

  if (Set.has(XRayInstrKind::Typed))
    Values.push_back("typed");

  if (Set.has(XRayInstrKind::FunctionEntry) &&
      Set.has(XRayInstrKind::FunctionExit))
    Values.push_back("function");
  else if (Set.has(XRayInstrKind::FunctionEntry))
    Values.push_back("function-entry");
  else if (Set.has(XRayInstrKind::FunctionExit))
    Values.push_back("function-exit");
}
} // namespace language::Core
