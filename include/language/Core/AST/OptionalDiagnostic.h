//===- OptionalDiagnostic.h - An optional diagnostic ------------*- C++ -*-===//
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
/// Implements a partial diagnostic which may not be emitted.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_OPTIONALDIAGNOSTIC_H
#define LANGUAGE_CORE_AST_OPTIONALDIAGNOSTIC_H

#include "language/Core/AST/APValue.h"
#include "language/Core/Basic/PartialDiagnostic.h"
#include "toolchain/ADT/APFloat.h"
#include "toolchain/ADT/APSInt.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {

/// A partial diagnostic which we might know in advance that we are not going
/// to emit.
class OptionalDiagnostic {
  PartialDiagnostic *Diag;

public:
  explicit OptionalDiagnostic(PartialDiagnostic *Diag = nullptr) : Diag(Diag) {}

  template <typename T> OptionalDiagnostic &operator<<(const T &v) {
    if (Diag)
      *Diag << v;
    return *this;
  }

  OptionalDiagnostic &operator<<(const toolchain::APSInt &I) {
    if (Diag) {
      SmallVector<char, 32> Buffer;
      I.toString(Buffer);
      *Diag << StringRef(Buffer.data(), Buffer.size());
    }
    return *this;
  }

  OptionalDiagnostic &operator<<(const toolchain::APFloat &F) {
    if (Diag) {
      // FIXME: Force the precision of the source value down so we don't
      // print digits which are usually useless (we don't really care here if
      // we truncate a digit by accident in edge cases).  Ideally,
      // APFloat::toString would automatically print the shortest
      // representation which rounds to the correct value, but it's a bit
      // tricky to implement. Could use std::to_chars.
      unsigned precision = toolchain::APFloat::semanticsPrecision(F.getSemantics());
      precision = (precision * 59 + 195) / 196;
      SmallVector<char, 32> Buffer;
      F.toString(Buffer, precision);
      *Diag << StringRef(Buffer.data(), Buffer.size());
    }
    return *this;
  }

  OptionalDiagnostic &operator<<(const toolchain::APFixedPoint &FX) {
    if (Diag) {
      SmallVector<char, 32> Buffer;
      FX.toString(Buffer);
      *Diag << StringRef(Buffer.data(), Buffer.size());
    }
    return *this;
  }
};

} // namespace language::Core

#endif
