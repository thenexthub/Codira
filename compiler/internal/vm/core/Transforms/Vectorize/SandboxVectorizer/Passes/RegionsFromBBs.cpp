//===- RegionsFromBBs.cpp - A helper to test RegionPasses -----------------===//
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

#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/RegionsFromBBs.h"
#include "vm/core/SandboxIR/Function.h"
#include "vm/core/SandboxIR/Region.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/SandboxVectorizerPassBuilder.h"

namespace vm::core::sandboxir {

RegionsFromBBs::RegionsFromBBs(StringRef Pipeline)
    : FunctionPass("regions-from-bbs"),
      RPM("rpm", Pipeline, SandboxVectorizerPassBuilder::createRegionPass) {}

bool RegionsFromBBs::runOnFunction(Function &F, const Analyses &A) {
  SmallVector<std::unique_ptr<Region>, 16> Regions;
  // Create a region for each BB.
  for (BasicBlock &BB : F) {
    Regions.push_back(std::make_unique<Region>(F.getContext(), A.getTTI()));
    auto &RgnPtr = Regions.back();
    for (Instruction &I : BB)
      RgnPtr->add(&I);
  }
  // For each region run the region pass pipeline.
  for (auto &RgnPtr : Regions)
    RPM.runOnRegion(*RgnPtr, A);
  return false;
}

} // namespace vm::core::sandboxir
