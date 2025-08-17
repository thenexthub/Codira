//===--- XRayInstr.h --------------------------------------------*- C++ -*-===//
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
/// \file
/// Defines the language::Core::XRayInstrKind enum.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_XRAYINSTR_H
#define LANGUAGE_CORE_BASIC_XRAYINSTR_H

#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Support/MathExtras.h"
#include <cassert>
#include <cstdint>

namespace language::Core {

using XRayInstrMask = uint32_t;

namespace XRayInstrKind {

// TODO: Auto-generate these as we add more instrumentation kinds.
enum XRayInstrOrdinal : XRayInstrMask {
  XRIO_FunctionEntry,
  XRIO_FunctionExit,
  XRIO_Custom,
  XRIO_Typed,
  XRIO_Count
};

constexpr XRayInstrMask None = 0;
constexpr XRayInstrMask FunctionEntry = 1U << XRIO_FunctionEntry;
constexpr XRayInstrMask FunctionExit = 1U << XRIO_FunctionExit;
constexpr XRayInstrMask Custom = 1U << XRIO_Custom;
constexpr XRayInstrMask Typed = 1U << XRIO_Typed;
constexpr XRayInstrMask All = FunctionEntry | FunctionExit | Custom | Typed;

} // namespace XRayInstrKind

struct XRayInstrSet {
  bool has(XRayInstrMask K) const {
    assert(toolchain::isPowerOf2_32(K));
    return Mask & K;
  }

  bool hasOneOf(XRayInstrMask K) const { return Mask & K; }

  void set(XRayInstrMask K, bool Value) {
    Mask = Value ? (Mask | K) : (Mask & ~K);
  }

  void clear(XRayInstrMask K = XRayInstrKind::All) { Mask &= ~K; }

  bool empty() const { return Mask == 0; }

  bool full() const { return Mask == XRayInstrKind::All; }

  XRayInstrMask Mask = 0;
};

/// Parses a command line argument into a mask.
XRayInstrMask parseXRayInstrValue(StringRef Value);

/// Serializes a set into a list of command line arguments.
void serializeXRayInstrValue(XRayInstrSet Set,
                             SmallVectorImpl<StringRef> &Values);

} // namespace language::Core

#endif // LANGUAGE_CORE_BASIC_XRAYINSTR_H
