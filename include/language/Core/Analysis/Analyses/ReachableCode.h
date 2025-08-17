//===- ReachableCode.h -----------------------------------------*- C++ --*-===//
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
// A flow-sensitive, path-insensitive analysis of unreachable code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_ANALYSES_REACHABLECODE_H
#define LANGUAGE_CORE_ANALYSIS_ANALYSES_REACHABLECODE_H

#include "language/Core/Basic/SourceLocation.h"

//===----------------------------------------------------------------------===//
// Forward declarations.
//===----------------------------------------------------------------------===//

namespace toolchain {
  class BitVector;
}

namespace language::Core {
  class AnalysisDeclContext;
  class CFGBlock;
  class Preprocessor;
}

//===----------------------------------------------------------------------===//
// API.
//===----------------------------------------------------------------------===//

namespace language::Core {
namespace reachable_code {

/// Classifications of unreachable code.
enum UnreachableKind {
  UK_Return,
  UK_Break,
  UK_Loop_Increment,
  UK_Other
};

class Callback {
  virtual void anchor();
public:
  virtual ~Callback() {}
  virtual void HandleUnreachable(UnreachableKind UK, SourceLocation L,
                                 SourceRange ConditionVal, SourceRange R1,
                                 SourceRange R2, bool HasFallThroughAttr) = 0;
};

/// ScanReachableFromBlock - Mark all blocks reachable from Start.
/// Returns the total number of blocks that were marked reachable.
unsigned ScanReachableFromBlock(const CFGBlock *Start,
                                toolchain::BitVector &Reachable);

void FindUnreachableCode(AnalysisDeclContext &AC, Preprocessor &PP,
                         Callback &CB);

}} // end namespace language::Core::reachable_code

#endif
