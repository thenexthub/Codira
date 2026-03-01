//===- CanonicalizeAliases.cpp - ThinLTO Support: Canonicalize Aliases ----===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// Currently this file implements partial alias canonicalization, to
// flatten chains of aliases (also done by GlobalOpt, but not on for
// O0 compiles). E.g.
//  @a = alias i8, i8 *@b
//  @b = alias i8, i8 *@g
//
// will be converted to:
//  @a = alias i8, i8 *@g  <-- @a is now an alias to base object @g
//  @b = alias i8, i8 *@g
//
// Eventually this file will implement full alias canonicalization, so that
// all aliasees are private anonymous values. E.g.
//  @a = alias i8, i8 *@g
//  @g = global i8 0
//
// will be converted to:
//  @0 = private global
//  @a = alias i8, i8* @0
//  @g = alias i8, i8* @0
//
// This simplifies optimization and ThinLTO linking of the original symbols.
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/CanonicalizeAliases.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Module.h"

using namespace vm::core;

namespace {

static Constant *canonicalizeAlias(Constant *C, bool &Changed) {
  if (auto *GA = dyn_cast<GlobalAlias>(C)) {
    auto *NewAliasee = canonicalizeAlias(GA->getAliasee(), Changed);
    if (NewAliasee != GA->getAliasee()) {
      GA->setAliasee(NewAliasee);
      Changed = true;
    }
    return NewAliasee;
  }

  auto *CE = dyn_cast<ConstantExpr>(C);
  if (!CE)
    return C;

  std::vector<Constant *> Ops;
  for (Use &U : CE->operands())
    Ops.push_back(canonicalizeAlias(cast<Constant>(U), Changed));
  return CE->getWithOperands(Ops);
}

/// Convert aliases to canonical form.
static bool canonicalizeAliases(Module &M) {
  bool Changed = false;
  for (auto &GA : M.aliases())
    canonicalizeAlias(&GA, Changed);
  return Changed;
}
} // anonymous namespace

PreservedAnalyses CanonicalizeAliasesPass::run(Module &M,
                                               ModuleAnalysisManager &AM) {
  if (!canonicalizeAliases(M))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}
