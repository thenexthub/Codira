//===-- SPIRVCombinerHelper.h -----------------------------------*- C++ -*-===//
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
///
/// This contains common combine transformations that may be used in a combine
/// pass.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVCOMBINERHELPER_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVCOMBINERHELPER_H

#include "SPIRVSubtarget.h"
#include "vm/core/CodeGen/GlobalISel/CombinerHelper.h"

namespace vm::core {
class SPIRVCombinerHelper : public CombinerHelper {
protected:
  const SPIRVSubtarget &STI;

public:
  using CombinerHelper::CombinerHelper;
  SPIRVCombinerHelper(GISelChangeObserver &Observer, MachineIRBuilder &B,
                      bool IsPreLegalize, GISelValueTracking *VT,
                      MachineDominatorTree *MDT, const LegalizerInfo *LI,
                      const SPIRVSubtarget &STI);

  bool matchLengthToDistance(MachineInstr &MI) const;
  void applySPIRVDistance(MachineInstr &MI) const;
  bool matchSelectToFaceForward(MachineInstr &MI) const;
  void applySPIRVFaceForward(MachineInstr &MI) const;
  bool matchMatrixTranspose(MachineInstr &MI) const;
  void applyMatrixTranspose(MachineInstr &MI) const;
  bool matchMatrixMultiply(MachineInstr &MI) const;
  void applyMatrixMultiply(MachineInstr &MI) const;

private:
  SPIRVType *getDotProductVectorType(Register ResReg, uint32_t K,
                                     SPIRVGlobalRegistry *GR) const;
  SmallVector<Register, 4> extractColumns(Register BReg, uint32_t N,
                                          SPIRVType *SpvVecType,
                                          SPIRVGlobalRegistry *GR) const;
  SmallVector<Register, 4> extractRows(Register AReg, uint32_t NumRows,
                                       uint32_t NumCols, SPIRVType *SpvRowType,
                                       SPIRVGlobalRegistry *GR) const;
  SmallVector<Register, 16>
  computeDotProducts(const SmallVector<Register, 4> &RowsA,
                     const SmallVector<Register, 4> &ColsB,
                     SPIRVType *SpvVecType, SPIRVGlobalRegistry *GR) const;
  Register computeDotProduct(Register RowA, Register ColB,
                             SPIRVType *SpvVecType,
                             SPIRVGlobalRegistry *GR) const;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_SPIRV_SPIRVCOMBINERHELPER_H
