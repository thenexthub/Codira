//===--- InterpShared.cpp ---------------------------------------*- C++ -*-===//
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

#include "InterpShared.h"
#include "language/Core/AST/Attr.h"
#include "toolchain/ADT/BitVector.h"

namespace language::Core {
namespace interp {

toolchain::BitVector collectNonNullArgs(const FunctionDecl *F,
                                   ArrayRef<const Expr *> Args) {
  toolchain::BitVector NonNullArgs;

  assert(F);
  assert(F->hasAttr<NonNullAttr>());
  NonNullArgs.resize(Args.size());

  for (const auto *Attr : F->specific_attrs<NonNullAttr>()) {
    if (!Attr->args_size()) {
      NonNullArgs.set();
      break;
    }

    for (auto Idx : Attr->args()) {
      unsigned ASTIdx = Idx.getASTIndex();
      if (ASTIdx >= Args.size())
        continue;
      NonNullArgs[ASTIdx] = true;
    }
  }

  return NonNullArgs;
}

} // namespace interp
} // namespace language::Core
