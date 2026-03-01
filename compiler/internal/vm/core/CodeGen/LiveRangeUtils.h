//===-- LiveRangeUtils.h - Live Range modification utilities ----*- C++ -*-===//
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
/// \file
/// This file contains helper functions to modify live ranges.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_LIVERANGEUTILS_H
#define LLVM_LIB_CODEGEN_LIVERANGEUTILS_H

#include "vm/core/CodeGen/LiveInterval.h"

namespace vm::core {

/// Helper function that distributes live range value numbers and the
/// corresponding segments of a primary live range \p LR to a list of newly
/// created live ranges \p SplitLRs. \p VNIClasses maps each value number in \p
/// LR to 0 meaning it should stay or to 1..N meaning it should go to a specific
/// live range in the \p SplitLRs array.
template<typename LiveRangeT, typename EqClassesT>
static void DistributeRange(LiveRangeT &LR, LiveRangeT *SplitLRs[],
                            EqClassesT VNIClasses) {
  // Move segments to new intervals.
  typename LiveRangeT::iterator J = LR.begin(), E = LR.end();
  while (J != E && VNIClasses[J->valno->id] == 0)
    ++J;
  for (typename LiveRangeT::iterator I = J; I != E; ++I) {
    if (unsigned eq = VNIClasses[I->valno->id]) {
      assert((SplitLRs[eq-1]->empty() || SplitLRs[eq-1]->expiredAt(I->start)) &&
             "New intervals should be empty");
      SplitLRs[eq-1]->segments.push_back(*I);
    } else
      *J++ = *I;
  }
  LR.segments.erase(J, E);

  // Transfer VNInfos to their new owners and renumber them.
  unsigned j = 0, e = LR.getNumValNums();
  while (j != e && VNIClasses[j] == 0)
    ++j;
  for (unsigned i = j; i != e; ++i) {
    VNInfo *VNI = LR.getValNumInfo(i);
    if (unsigned eq = VNIClasses[i]) {
      VNI->id = SplitLRs[eq-1]->getNumValNums();
      SplitLRs[eq-1]->valnos.push_back(VNI);
    } else {
      VNI->id = j;
      LR.valnos[j++] = VNI;
    }
  }
  LR.valnos.resize(j);
}

} // End toolchain namespace

#endif
