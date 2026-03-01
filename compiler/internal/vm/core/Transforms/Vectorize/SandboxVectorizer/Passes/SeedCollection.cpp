//===- SeedCollection.cpp - Seed collection pass --------------------------===//
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

#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/SeedCollection.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/SandboxIR/Module.h"
#include "vm/core/SandboxIR/Region.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/SandboxVectorizerPassBuilder.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/SeedCollector.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/VecUtils.h"

namespace vm::core {

static cl::opt<unsigned>
    OverrideVecRegBits("sbvec-vec-reg-bits", cl::init(0), cl::Hidden,
                       cl::desc("Override the vector register size in bits, "
                                "which is otherwise found by querying TTI."));
static cl::opt<bool>
    AllowNonPow2("sbvec-allow-non-pow2", cl::init(false), cl::Hidden,
                 cl::desc("Allow non-power-of-2 vectorization."));

#define LoadSeedsDef "loads"
#define StoreSeedsDef "stores"
cl::opt<std::string> CollectSeeds(
    "sbvec-collect-seeds", cl::init(StoreSeedsDef), cl::Hidden,
    cl::desc("Collect these seeds. Use empty for none or a comma-separated "
             "list of '" StoreSeedsDef "' and '" LoadSeedsDef "'."));

namespace sandboxir {
SeedCollection::SeedCollection(StringRef Pipeline)
    : FunctionPass("seed-collection"),
      RPM("rpm", Pipeline, SandboxVectorizerPassBuilder::createRegionPass) {}

bool SeedCollection::runOnFunction(Function &F, const Analyses &A) {
  bool Change = false;
  const auto &DL = F.getParent()->getDataLayout();
  unsigned VecRegBits =
      OverrideVecRegBits != 0
          ? OverrideVecRegBits
          : A.getTTI()
                .getRegisterBitWidth(TargetTransformInfo::RGK_FixedWidthVector)
                .getFixedValue();
  bool CollectStores = CollectSeeds.find(StoreSeedsDef) != std::string::npos;
  bool CollectLoads = CollectSeeds.find(LoadSeedsDef) != std::string::npos;

  // TODO: Start from innermost BBs first
  for (auto &BB : F) {
    SeedCollector SC(&BB, A.getScalarEvolution(), CollectStores, CollectLoads);
    for (SeedBundle &Seeds : SC.getStoreSeeds()) {
      unsigned ElmBits =
          Utils::getNumBits(VecUtils::getElementType(Utils::getExpectedType(
                                Seeds[Seeds.getFirstUnusedElementIdx()])),
                            DL);

      auto DivideBy2 = [](unsigned Num) {
        auto Floor = VecUtils::getFloorPowerOf2(Num);
        if (Floor == Num)
          return Floor / 2;
        return Floor;
      };
      // Try to create the largest vector supported by the target. If it fails
      // reduce the vector size by half.
      for (unsigned SliceElms = std::min(VecRegBits / ElmBits,
                                         Seeds.getNumUnusedBits() / ElmBits);
           SliceElms >= 2u; SliceElms = DivideBy2(SliceElms)) {
        if (Seeds.allUsed())
          break;
        // Keep trying offsets after FirstUnusedElementIdx, until we vectorize
        // the slice. This could be quite expensive, so we enforce a limit.
        for (unsigned Offset = Seeds.getFirstUnusedElementIdx(),
                      OE = Seeds.size();
             Offset + 1 < OE; Offset += 1) {
          // Seeds are getting used as we vectorize, so skip them.
          if (Seeds.isUsed(Offset))
            continue;
          if (Seeds.allUsed())
            break;

          auto SeedSlice =
              Seeds.getSlice(Offset, SliceElms * ElmBits, !AllowNonPow2);
          if (SeedSlice.empty())
            continue;

          assert(SeedSlice.size() >= 2 && "Should have been rejected!");

          // Create a region containing the seed slice.
          auto &Ctx = F.getContext();
          Region Rgn(Ctx, A.getTTI());
          Rgn.setAux(SeedSlice);
          // Run the region pass pipeline.
          Change |= RPM.runOnRegion(Rgn, A);
          Rgn.clearAux();
        }
      }
    }
  }
  return Change;
}
} // namespace sandboxir
} // namespace vm::core
