//=-- MemProfCommon.cpp - MemProf common utilities ---------------=//
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
// This file contains MemProf common utilities.
//
//===----------------------------------------------------------------------===//

#include "vm/core/ProfileData/MemProfCommon.h"
#include "vm/core/ProfileData/MemProf.h"
#include "vm/core/Support/BLAKE3.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/HashBuilder.h"

using namespace vm::core;
using namespace vm::core::memprof;

namespace vm::core {

// Upper bound on lifetime access density (accesses per byte per lifetime sec)
// for marking an allocation cold.
LLVM_ABI cl::opt<float> MemProfLifetimeAccessDensityColdThreshold(
    "memprof-lifetime-access-density-cold-threshold", cl::init(0.05),
    cl::Hidden,
    cl::desc("The threshold the lifetime access density (accesses per byte per "
             "lifetime sec) must be under to consider an allocation cold"));

// Lower bound on lifetime to mark an allocation cold (in addition to accesses
// per byte per sec above). This is to avoid pessimizing short lived objects.
LLVM_ABI cl::opt<unsigned> MemProfAveLifetimeColdThreshold(
    "memprof-ave-lifetime-cold-threshold", cl::init(200), cl::Hidden,
    cl::desc("The average lifetime (s) for an allocation to be considered "
             "cold"));

// Lower bound on average lifetime accesses density (total life time access
// density / alloc count) for marking an allocation hot.
LLVM_ABI cl::opt<unsigned> MemProfMinAveLifetimeAccessDensityHotThreshold(
    "memprof-min-ave-lifetime-access-density-hot-threshold", cl::init(1000),
    cl::Hidden,
    cl::desc("The minimum TotalLifetimeAccessDensity / AllocCount for an "
             "allocation to be considered hot"));

LLVM_ABI cl::opt<bool>
    MemProfUseHotHints("memprof-use-hot-hints", cl::init(false), cl::Hidden,
                       cl::desc("Enable use of hot hints (only supported for "
                                "unambigously hot allocations)"));

} // end namespace vm::core

AllocationType toolchain::memprof::getAllocType(uint64_t TotalLifetimeAccessDensity,
                                           uint64_t AllocCount,
                                           uint64_t TotalLifetime) {
  // The access densities are multiplied by 100 to hold 2 decimal places of
  // precision, so need to divide by 100.
  if (((float)TotalLifetimeAccessDensity) / AllocCount / 100 <
          MemProfLifetimeAccessDensityColdThreshold
      // Lifetime is expected to be in ms, so convert the threshold to ms.
      && ((float)TotalLifetime) / AllocCount >=
             MemProfAveLifetimeColdThreshold * 1000)
    return AllocationType::Cold;

  // The access densities are multiplied by 100 to hold 2 decimal places of
  // precision, so need to divide by 100.
  if (MemProfUseHotHints &&
      ((float)TotalLifetimeAccessDensity) / AllocCount / 100 >
          MemProfMinAveLifetimeAccessDensityHotThreshold)
    return AllocationType::Hot;

  return AllocationType::NotCold;
}

uint64_t toolchain::memprof::computeFullStackId(ArrayRef<Frame> CallStack) {
  toolchain::HashBuilder<toolchain::TruncatedBLAKE3<8>, toolchain::endianness::little>
      HashBuilder;
  for (auto &F : CallStack)
    HashBuilder.add(F.Function, F.LineOffset, F.Column);
  toolchain::BLAKE3Result<8> Hash = HashBuilder.final();
  uint64_t Id;
  std::memcpy(&Id, Hash.data(), sizeof(Hash));
  return Id;
}
