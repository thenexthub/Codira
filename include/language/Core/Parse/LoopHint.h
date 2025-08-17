//===--- LoopHint.h - Types for LoopHint ------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_PARSE_LOOPHINT_H
#define LANGUAGE_CORE_PARSE_LOOPHINT_H

#include "language/Core/Basic/IdentifierTable.h"
#include "language/Core/Basic/SourceLocation.h"

namespace language::Core {

class Expr;

/// Loop optimization hint for loop and unroll pragmas.
struct LoopHint {
  // Source range of the directive.
  SourceRange Range;
  // Identifier corresponding to the name of the pragma.  "loop" for
  // "#pragma clang loop" directives and "unroll" for "#pragma unroll"
  // hints.
  IdentifierLoc *PragmaNameLoc = nullptr;
  // Name of the loop hint.  Examples: "unroll", "vectorize".  In the
  // "#pragma unroll" and "#pragma nounroll" cases, this is identical to
  // PragmaNameLoc.
  IdentifierLoc *OptionLoc = nullptr;
  // Identifier for the hint state argument.  If null, then the state is
  // default value such as for "#pragma unroll".
  IdentifierLoc *StateLoc = nullptr;
  // Expression for the hint argument if it exists, null otherwise.
  Expr *ValueExpr = nullptr;

  LoopHint() = default;
};

} // end namespace language::Core

#endif // LANGUAGE_CORE_PARSE_LOOPHINT_H
