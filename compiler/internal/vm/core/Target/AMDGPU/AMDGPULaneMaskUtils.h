//===- AMDGPULaneMaskUtils.h - Exec/lane mask helper functions -*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPULANEMASKUTILS_H
#define LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPULANEMASKUTILS_H

#include "GCNSubtarget.h"
#include "vm/core/CodeGen/Register.h"

namespace vm::core {

class GCNSubtarget;

namespace AMDGPU {

class LaneMaskConstants {
public:
  const Register ExecReg;
  const Register VccReg;
  const unsigned AndOpc;
  const unsigned AndTermOpc;
  const unsigned AndN2Opc;
  const unsigned AndN2SaveExecOpc;
  const unsigned AndN2TermOpc;
  const unsigned AndSaveExecOpc;
  const unsigned AndSaveExecTermOpc;
  const unsigned BfmOpc;
  const unsigned CMovOpc;
  const unsigned CSelectOpc;
  const unsigned MovOpc;
  const unsigned MovTermOpc;
  const unsigned OrOpc;
  const unsigned OrTermOpc;
  const unsigned OrSaveExecOpc;
  const unsigned XorOpc;
  const unsigned XorTermOpc;
  const unsigned WQMOpc;

  constexpr LaneMaskConstants(bool IsWave32)
      : ExecReg(IsWave32 ? AMDGPU::EXEC_LO : AMDGPU::EXEC),
        VccReg(IsWave32 ? AMDGPU::VCC_LO : AMDGPU::VCC),
        AndOpc(IsWave32 ? AMDGPU::S_AND_B32 : AMDGPU::S_AND_B64),
        AndTermOpc(IsWave32 ? AMDGPU::S_AND_B32_term : AMDGPU::S_AND_B64_term),
        AndN2Opc(IsWave32 ? AMDGPU::S_ANDN2_B32 : AMDGPU::S_ANDN2_B64),
        AndN2SaveExecOpc(IsWave32 ? AMDGPU::S_ANDN2_SAVEEXEC_B32
                                  : AMDGPU::S_ANDN2_SAVEEXEC_B64),
        AndN2TermOpc(IsWave32 ? AMDGPU::S_ANDN2_B32_term
                              : AMDGPU::S_ANDN2_B64_term),
        AndSaveExecOpc(IsWave32 ? AMDGPU::S_AND_SAVEEXEC_B32
                                : AMDGPU::S_AND_SAVEEXEC_B64),
        AndSaveExecTermOpc(IsWave32 ? AMDGPU::S_AND_SAVEEXEC_B32_term
                                    : AMDGPU::S_AND_SAVEEXEC_B64_term),
        BfmOpc(IsWave32 ? AMDGPU::S_BFM_B32 : AMDGPU::S_BFM_B64),
        CMovOpc(IsWave32 ? AMDGPU::S_CMOV_B32 : AMDGPU::S_CMOV_B64),
        CSelectOpc(IsWave32 ? AMDGPU::S_CSELECT_B32 : AMDGPU::S_CSELECT_B64),
        MovOpc(IsWave32 ? AMDGPU::S_MOV_B32 : AMDGPU::S_MOV_B64),
        MovTermOpc(IsWave32 ? AMDGPU::S_MOV_B32_term : AMDGPU::S_MOV_B64_term),
        OrOpc(IsWave32 ? AMDGPU::S_OR_B32 : AMDGPU::S_OR_B64),
        OrTermOpc(IsWave32 ? AMDGPU::S_OR_B32_term : AMDGPU::S_OR_B64_term),
        OrSaveExecOpc(IsWave32 ? AMDGPU::S_OR_SAVEEXEC_B32
                               : AMDGPU::S_OR_SAVEEXEC_B64),
        XorOpc(IsWave32 ? AMDGPU::S_XOR_B32 : AMDGPU::S_XOR_B64),
        XorTermOpc(IsWave32 ? AMDGPU::S_XOR_B32_term : AMDGPU::S_XOR_B64_term),
        WQMOpc(IsWave32 ? AMDGPU::S_WQM_B32 : AMDGPU::S_WQM_B64) {}

  static inline const LaneMaskConstants &get(const GCNSubtarget &ST);
};

static constexpr LaneMaskConstants LaneMaskConstants32 =
    LaneMaskConstants(/*IsWave32=*/true);
static constexpr LaneMaskConstants LaneMaskConstants64 =
    LaneMaskConstants(/*IsWave32=*/false);

inline const LaneMaskConstants &LaneMaskConstants::get(const GCNSubtarget &ST) {
  unsigned WavefrontSize = ST.getWavefrontSize();
  assert(WavefrontSize == 32 || WavefrontSize == 64);
  return WavefrontSize == 32 ? LaneMaskConstants32 : LaneMaskConstants64;
}

} // end namespace AMDGPU

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_UTILS_AMDGPULANEMASKUTILS_H
