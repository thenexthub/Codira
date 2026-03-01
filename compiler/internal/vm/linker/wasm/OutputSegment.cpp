//===- OutputSegment.h -----------------------------------------*- C++ -*-===//
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

#include "OutputSegment.h"
#include "InputChunks.h"
#include "lld/Common/Memory.h"

#define DEBUG_TYPE "lld"

using namespace llvm;
using namespace llvm::wasm;

namespace lld::wasm {

void OutputSegment::addInputSegment(InputChunk *inSeg) {
  alignment = std::max(alignment, inSeg->alignment);
  inputSegments.push_back(inSeg);
  size = llvm::alignTo(size, 1ULL << inSeg->alignment);
  LLVM_DEBUG(dbgs() << "addInputSegment: " << inSeg->name << " oname=" << name
                    << " size=" << inSeg->getSize()
                    << " align=" << inSeg->alignment << " at:" << size << "\n");
  inSeg->outputSeg = this;
  inSeg->outputSegmentOffset = size;
  size += inSeg->getSize();
}

// This function scans over the input segments.
//
// It removes MergeInputChunks from the input section array and adds
// new synthetic sections at the location of the first input section
// that it replaces. It then finalizes each synthetic section in order
// to compute an output offset for each piece of each input section.
void OutputSegment::finalizeInputSegments() {
  LLVM_DEBUG(llvm::dbgs() << "finalizeInputSegments: " << name << "\n");
  std::vector<SyntheticMergedChunk *> mergedSegments;
  std::vector<InputChunk *> newSegments;
  for (InputChunk *s : inputSegments) {
    MergeInputChunk *ms = dyn_cast<MergeInputChunk>(s);
    if (!ms) {
      newSegments.push_back(s);
      continue;
    }

    // A segment should not make it here unless its alive
    assert(ms->live);

    auto i = llvm::find_if(mergedSegments, [=](SyntheticMergedChunk *seg) {
      return seg->flags == ms->flags && seg->alignment == ms->alignment;
    });
    if (i == mergedSegments.end()) {
      LLVM_DEBUG(llvm::dbgs() << "new merge segment: " << name
                              << " alignment=" << ms->alignment << "\n");
      auto *syn = make<SyntheticMergedChunk>(name, ms->alignment, ms->flags);
      syn->outputSeg = this;
      mergedSegments.push_back(syn);
      i = std::prev(mergedSegments.end());
      newSegments.push_back(syn);
    } else {
      LLVM_DEBUG(llvm::dbgs() << "adding to merge segment: " << name << "\n");
    }
    (*i)->addMergeChunk(ms);
  }

  for (auto *ms : mergedSegments)
    ms->finalizeContents();

  inputSegments = newSegments;
  size = 0;
  for (InputChunk *seg : inputSegments) {
    size = llvm::alignTo(size, 1ULL << seg->alignment);
    LLVM_DEBUG(llvm::dbgs() << "outputSegmentOffset set: " << seg->name
                            << " -> " << size << "\n");
    seg->outputSegmentOffset = size;
    size += seg->getSize();
  }
}

} // namespace lld::wasm
