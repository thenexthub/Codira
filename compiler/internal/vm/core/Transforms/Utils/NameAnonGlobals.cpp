//===- NameAnonGlobals.cpp - ThinLTO Support: Name Unnamed Globals --------===//
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
// This file implements naming anonymous globals to make sure they can be
// referred to by ThinLTO.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/NameAnonGlobals.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/MD5.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

using namespace vm::core;

namespace {
// Compute a "unique" hash for the module based on the name of the public
// globals.
class ModuleHasher {
  Module &TheModule;
  std::string TheHash;

public:
  ModuleHasher(Module &M) : TheModule(M) {}

  /// Return the lazily computed hash.
  std::string &get() {
    if (!TheHash.empty())
      // Cache hit :)
      return TheHash;

    MD5 Hasher;
    for (auto &F : TheModule) {
      if (F.isDeclaration() || F.hasLocalLinkage() || !F.hasName())
        continue;
      auto Name = F.getName();
      Hasher.update(Name);
    }
    for (auto &GV : TheModule.globals()) {
      if (GV.isDeclaration() || GV.hasLocalLinkage() || !GV.hasName())
        continue;
      auto Name = GV.getName();
      Hasher.update(Name);
    }

    // Now return the result.
    MD5::MD5Result Hash;
    Hasher.final(Hash);
    SmallString<32> Result;
    MD5::stringifyResult(Hash, Result);
    TheHash = std::string(Result);
    return TheHash;
  }
};
} // end anonymous namespace

// Rename all the anon globals in the module
bool toolchain::nameUnamedGlobals(Module &M) {
  bool Changed = false;
  ModuleHasher ModuleHash(M);
  int count = 0;
  auto RenameIfNeed = [&](GlobalValue &GV) {
    if (GV.hasName())
      return;
    GV.setName(Twine("anon.") + ModuleHash.get() + "." + Twine(count++));
    Changed = true;
  };
  for (auto &GO : M.global_objects())
    RenameIfNeed(GO);
  for (auto &GA : M.aliases())
    RenameIfNeed(GA);

  return Changed;
}

PreservedAnalyses NameAnonGlobalPass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  if (!nameUnamedGlobals(M))
    return PreservedAnalyses::all();

  return PreservedAnalyses::none();
}
