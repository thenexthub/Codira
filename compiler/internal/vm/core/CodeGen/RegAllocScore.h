//==- RegAllocScore.h - evaluate regalloc policy quality  ----------*-C++-*-==//
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
/// Calculate a measure of the register allocation policy quality. This is used
/// to construct a reward for the training of the ML-driven allocation policy.
/// Currently, the score is the sum of the machine basic block frequency-weighed
/// number of loads, stores, copies, and remat instructions, each factored with
/// a relative weight.
//===----------------------------------------------------------------------===//

#ifndef LLVM_CODEGEN_REGALLOCSCORE_H_
#define LLVM_CODEGEN_REGALLOCSCORE_H_

#include "vm/core/ADT/STLFunctionalExtras.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {

class MachineBasicBlock;
class MachineBlockFrequencyInfo;
class MachineFunction;
class MachineInstr;

/// Regalloc score.
class RegAllocScore final {
  double CopyCounts = 0.0;
  double LoadCounts = 0.0;
  double StoreCounts = 0.0;
  double CheapRematCounts = 0.0;
  double LoadStoreCounts = 0.0;
  double ExpensiveRematCounts = 0.0;

public:
  RegAllocScore() = default;
  RegAllocScore(const RegAllocScore &) = default;

  double copyCounts() const { return CopyCounts; }
  double loadCounts() const { return LoadCounts; }
  double storeCounts() const { return StoreCounts; }
  double loadStoreCounts() const { return LoadStoreCounts; }
  double expensiveRematCounts() const { return ExpensiveRematCounts; }
  double cheapRematCounts() const { return CheapRematCounts; }

  void onCopy(double Freq) { CopyCounts += Freq; }
  void onLoad(double Freq) { LoadCounts += Freq; }
  void onStore(double Freq) { StoreCounts += Freq; }
  void onLoadStore(double Freq) { LoadStoreCounts += Freq; }
  void onExpensiveRemat(double Freq) { ExpensiveRematCounts += Freq; }
  void onCheapRemat(double Freq) { CheapRematCounts += Freq; }

  RegAllocScore &operator+=(const RegAllocScore &Other);
  LLVM_ABI_FOR_TEST bool operator==(const RegAllocScore &Other) const;
  bool operator!=(const RegAllocScore &Other) const;
  LLVM_ABI_FOR_TEST double getScore() const;
};

/// Calculate a score. When comparing 2 scores for the same function but
/// different policies, the better policy would have a smaller score.
/// The implementation is the overload below (which is also easily unittestable)
RegAllocScore calculateRegAllocScore(const MachineFunction &MF,
                                     const MachineBlockFrequencyInfo &MBFI);

/// Implementation of the above, which is also more easily unittestable.
LLVM_ABI_FOR_TEST RegAllocScore calculateRegAllocScore(
    const MachineFunction &MF,
    toolchain::function_ref<double(const MachineBasicBlock &)> GetBBFreq,
    toolchain::function_ref<bool(const MachineInstr &)> IsTriviallyRematerializable);
} // end namespace vm::core

#endif // LLVM_CODEGEN_REGALLOCSCORE_H_
