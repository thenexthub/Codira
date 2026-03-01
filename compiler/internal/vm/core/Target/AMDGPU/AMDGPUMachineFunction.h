//===-- AMDGPUMachineFunctionInfo.h -------------------------------*- C++ -*-=//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUMACHINEFUNCTION_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUMACHINEFUNCTION_H

#include "Utils/AMDGPUBaseInfo.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/GlobalVariable.h"

namespace vm::core {

class AMDGPUSubtarget;

class AMDGPUMachineFunction : public MachineFunctionInfo {
  /// A map to keep track of local memory objects and their offsets within the
  /// local memory space.
  SmallDenseMap<const GlobalValue *, unsigned, 4> LocalMemoryObjects;

protected:
  uint64_t ExplicitKernArgSize = 0; // Cache for this.
  Align MaxKernArgAlign;        // Cache for this.

  /// Number of bytes in the LDS that are being used.
  uint32_t LDSSize = 0;
  uint32_t GDSSize = 0;

  /// Number of bytes in the LDS allocated statically. This field is only used
  /// in the instruction selector and not part of the machine function info.
  uint32_t StaticLDSSize = 0;
  uint32_t StaticGDSSize = 0;

  /// Align for dynamic shared memory if any. Dynamic shared memory is
  /// allocated directly after the static one, i.e., LDSSize. Need to pad
  /// LDSSize to ensure that dynamic one is aligned accordingly.
  /// The maximal alignment is updated during IR translation or lowering
  /// stages.
  Align DynLDSAlign;

  // Flag to check dynamic LDS usage by kernel.
  bool UsesDynamicLDS = false;

  uint32_t NumNamedBarriers = 0;

  // Kernels + shaders. i.e. functions called by the hardware and not called
  // by other functions.
  bool IsEntryFunction = false;

  // Entry points called by other functions instead of directly by the hardware.
  bool IsModuleEntryFunction = false;

  // Functions with the amdgpu_cs_chain or amdgpu_cs_chain_preserve CC.
  bool IsChainFunction = false;

  bool NoSignedZerosFPMath = false;

  // Function may be memory bound.
  bool MemoryBound = false;

  // Kernel may need limited waves per EU for better performance.
  bool WaveLimiter = false;

  bool HasInitWholeWave = false;

public:
  AMDGPUMachineFunction(const Function &F, const AMDGPUSubtarget &ST);

  uint64_t getExplicitKernArgSize() const {
    return ExplicitKernArgSize;
  }

  Align getMaxKernArgAlign() const { return MaxKernArgAlign; }

  uint32_t getLDSSize() const {
    return LDSSize;
  }

  uint32_t getGDSSize() const {
    return GDSSize;
  }

  void recordNumNamedBarriers(uint32_t GVAddr, unsigned BarCnt) {
    NumNamedBarriers =
        std::max(NumNamedBarriers, ((GVAddr & 0x1ff) >> 4) + BarCnt - 1);
  }
  uint32_t getNumNamedBarriers() const { return NumNamedBarriers; }

  bool isEntryFunction() const {
    return IsEntryFunction;
  }

  bool isModuleEntryFunction() const { return IsModuleEntryFunction; }

  bool isChainFunction() const { return IsChainFunction; }

  // The stack is empty upon entry to this function.
  bool isBottomOfStack() const {
    return isEntryFunction() || isChainFunction();
  }

  bool hasNoSignedZerosFPMath() const {
    return NoSignedZerosFPMath;
  }

  bool isMemoryBound() const {
    return MemoryBound;
  }

  bool needsWaveLimiter() const {
    return WaveLimiter;
  }

  bool hasInitWholeWave() const { return HasInitWholeWave; }
  void setInitWholeWave() { HasInitWholeWave = true; }

  unsigned allocateLDSGlobal(const DataLayout &DL, const GlobalVariable &GV) {
    return allocateLDSGlobal(DL, GV, DynLDSAlign);
  }

  unsigned allocateLDSGlobal(const DataLayout &DL, const GlobalVariable &GV,
                             Align Trailing);

  static std::optional<uint32_t> getLDSKernelIdMetadata(const Function &F);
  static std::optional<uint32_t> getLDSAbsoluteAddress(const GlobalValue &GV);

  Align getDynLDSAlign() const { return DynLDSAlign; }

  void setDynLDSAlign(const Function &F, const GlobalVariable &GV);

  void setUsesDynamicLDS(bool DynLDS);

  bool isDynamicLDSUsed() const;
};

}
#endif
