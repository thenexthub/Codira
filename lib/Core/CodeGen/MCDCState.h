//===---- MCDCState.h - Per-Function MC/DC state ----------------*- C++ -*-===//
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
//  Per-Function MC/DC state for PGO
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_CODEGEN_MCDCSTATE_H
#define LANGUAGE_CORE_LIB_CODEGEN_MCDCSTATE_H

#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ProfileData/Coverage/MCDCTypes.h"

namespace language::Core {
class Stmt;
} // namespace language::Core

namespace language::Core::CodeGen::MCDC {

using namespace toolchain::coverage::mcdc;

/// Per-Function MC/DC state
struct State {
  unsigned BitmapBits = 0;

  struct Decision {
    unsigned BitmapIdx;
    toolchain::SmallVector<std::array<int, 2>> Indices;
  };

  toolchain::DenseMap<const Stmt *, Decision> DecisionByStmt;

  struct Branch {
    ConditionID ID;
    const Stmt *DecisionStmt;
  };

  toolchain::DenseMap<const Stmt *, Branch> BranchByStmt;
};

} // namespace language::Core::CodeGen::MCDC

#endif // LANGUAGE_CORE_LIB_CODEGEN_MCDCSTATE_H
