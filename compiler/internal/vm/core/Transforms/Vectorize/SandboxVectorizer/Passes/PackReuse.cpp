//===- PackReuse.cpp - A pack de-duplication pass -------------------------===//
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

#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/PackReuse.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/VecUtils.h"

namespace vm::core::sandboxir {

bool PackReuse::runOnRegion(Region &Rgn, const Analyses &A) {
  if (Rgn.empty())
    return Change;
  // The key to the map is the ordered operands of the pack.
  // The value is a vector of all Pack Instrs with the same operands.
  DenseMap<std::pair<BasicBlock *, SmallVector<Value *>>,
           SmallVector<SmallVector<Instruction *>>>
      PacksMap;
  // Go over the region and look for pack patterns.
  for (auto *I : Rgn) {
    auto PackOpt = VecUtils::matchPack(I);
    if (PackOpt) {
      // TODO: For now limit pack reuse within a BB.
      BasicBlock *BB = (*PackOpt->Instrs.front()).getParent();
      PacksMap[{BB, PackOpt->Operands}].push_back(PackOpt->Instrs);
    }
  }
  for (auto &Pair : PacksMap) {
    auto &Packs = Pair.second;
    if (Packs.size() <= 1)
      continue;
    // Sort packs by program order.
    sort(Packs, [](const auto &PackInstrs1, const auto &PackInstrs2) {
      return PackInstrs1.front()->comesBefore(PackInstrs2.front());
    });
    Instruction *TopMostPack = Packs[0].front();
    // Replace duplicate packs with the first one.
    for (const auto &PackInstrs :
         make_range(std::next(Packs.begin()), Packs.end())) {
      PackInstrs.front()->replaceAllUsesWith(TopMostPack);
      // Delete the pack instrs bottom-up since they are now dead.
      for (auto *PackI : PackInstrs)
        PackI->eraseFromParent();
    }
    Change = true;
  }
  return Change;
}

} // namespace vm::core::sandboxir
