//===-- IndirectCallPromotionAnalysis.cpp - Find promotion candidates ===//
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
// Helper methods for identifying profitable indirect call promotion
// candidates for an instruction when the indirect-call value profile metadata
// is available.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/IndirectCallPromotionAnalysis.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/ProfileData/InstrProf.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"

using namespace vm::core;

#define DEBUG_TYPE "pgo-icall-prom-analysis"

namespace vm::core {

// The percent threshold for the direct-call target (this call site vs the
// remaining call count) for it to be considered as the promotion target.
static cl::opt<unsigned> ICPRemainingPercentThreshold(
    "icp-remaining-percent-threshold", cl::init(30), cl::Hidden,
    cl::desc("The percentage threshold against remaining unpromoted indirect "
             "call count for the promotion"));

// The percent threshold for the direct-call target (this call site vs the
// total call count) for it to be considered as the promotion target.
static cl::opt<uint64_t>
    ICPTotalPercentThreshold("icp-total-percent-threshold", cl::init(5),
                             cl::Hidden,
                             cl::desc("The percentage threshold against total "
                                      "count for the promotion"));

// Set the minimum absolute count threshold for indirect call promotion.
// Candidates with counts below this threshold will not be promoted.
static cl::opt<unsigned> ICPMinimumCountThreshold(
    "icp-minimum-count-threshold", cl::init(0), cl::Hidden,
    cl::desc("Minimum absolute count for promotion candidate"));

// Set the maximum number of targets to promote for a single indirect-call
// callsite.
static cl::opt<unsigned>
    MaxNumPromotions("icp-max-prom", cl::init(3), cl::Hidden,
                     cl::desc("Max number of promotions for a single indirect "
                              "call callsite"));

cl::opt<unsigned> MaxNumVTableAnnotations(
    "icp-max-num-vtables", cl::init(6), cl::Hidden,
    cl::desc("Max number of vtables annotated for a vtable load instruction."));

} // end namespace vm::core

bool ICallPromotionAnalysis::isPromotionProfitable(uint64_t Count,
                                                   uint64_t TotalCount,
                                                   uint64_t RemainingCount) {
  return Count >= ICPMinimumCountThreshold &&
         Count * 100 >= ICPRemainingPercentThreshold * RemainingCount &&
         Count * 100 >= ICPTotalPercentThreshold * TotalCount;
}

// Indirect-call promotion heuristic. The direct targets are sorted based on
// the count. Stop at the first target that is not promoted. Returns the
// number of candidates deemed profitable.
uint32_t ICallPromotionAnalysis::getProfitablePromotionCandidates(
    const Instruction *Inst, uint64_t TotalCount) {
  LLVM_DEBUG(dbgs() << " \nWork on callsite " << *Inst
                    << " Num_targets: " << ValueDataArray.size() << "\n");

  uint32_t I = 0;
  uint64_t RemainingCount = TotalCount;
  for (; I < MaxNumPromotions && I < ValueDataArray.size(); I++) {
    uint64_t Count = ValueDataArray[I].Count;
    assert(Count <= RemainingCount);
    LLVM_DEBUG(dbgs() << " Candidate " << I << " Count=" << Count
                      << "  Target_func: " << ValueDataArray[I].Value << "\n");

    if (!isPromotionProfitable(Count, TotalCount, RemainingCount)) {
      LLVM_DEBUG(dbgs() << " Not promote: Cold target.\n");
      return I;
    }
    RemainingCount -= Count;
  }
  return I;
}

MutableArrayRef<InstrProfValueData>
ICallPromotionAnalysis::getPromotionCandidatesForInstruction(
    const Instruction *I, uint64_t &TotalCount, uint32_t &NumCandidates,
    unsigned MaxNumValueData) {
  // Use the max of the values specified by -icp-max-prom and the provided
  // MaxNumValueData parameter.
  if (MaxNumPromotions > MaxNumValueData)
    MaxNumValueData = MaxNumPromotions;
  ValueDataArray = getValueProfDataFromInst(*I, IPVK_IndirectCallTarget,
                                            MaxNumValueData, TotalCount);
  if (ValueDataArray.empty()) {
    NumCandidates = 0;
    return MutableArrayRef<InstrProfValueData>();
  }
  NumCandidates = getProfitablePromotionCandidates(I, TotalCount);
  return ValueDataArray;
}
