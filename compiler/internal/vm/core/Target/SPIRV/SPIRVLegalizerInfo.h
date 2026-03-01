//===- SPIRVLegalizerInfo.h --- SPIR-V Legalization Rules --------*- C++ -*-==//
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
// This file declares the targeting of the MachineLegalizer class for SPIR-V.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVMACHINELEGALIZER_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVMACHINELEGALIZER_H

#include "SPIRVGlobalRegistry.h"
#include "vm/core/CodeGen/GlobalISel/LegalizerInfo.h"

namespace vm::core {

class LLVMContext;
class SPIRVSubtarget;

// This class provides the information for legalizing SPIR-V instructions.
class SPIRVLegalizerInfo : public LegalizerInfo {
  const SPIRVSubtarget *ST;
  SPIRVGlobalRegistry *GR;

public:
  bool legalizeCustom(LegalizerHelper &Helper, MachineInstr &MI,
                      LostDebugLocObserver &LocObserver) const override;
  bool legalizeIntrinsic(LegalizerHelper &Helper,
                         MachineInstr &MI) const override;

  SPIRVLegalizerInfo(const SPIRVSubtarget &ST);

private:
  bool legalizeIsFPClass(LegalizerHelper &Helper, MachineInstr &MI,
                         LostDebugLocObserver &LocObserver) const;
  bool legalizeBitcast(LegalizerHelper &Helper, MachineInstr &MI) const;
};
} // namespace vm::core
#endif // LLVM_LIB_TARGET_SPIRV_SPIRVMACHINELEGALIZER_H
