//===-- SIDefines.h - SI Helper Functions -----------------------*- C++ -*-===//
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
/// \file - utility functions for the SIDefines and its common uses.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_UTILS_SIDEFINESUTILS_H
#define LLVM_LIB_TARGET_AMDGPU_UTILS_SIDEFINESUTILS_H

#include "vm/core/MC/MCExpr.h"
#include <utility>

namespace vm::core {
class MCContext;
namespace AMDGPU {

/// Deduce the least significant bit aligned shift and mask values for a binary
/// Complement \p Value (as they're defined in SIDefines.h as C_*) as a returned
/// pair<shift, mask>. That is to say \p Value == ~(mask << shift)
///
/// For example, given C_00B848_FWD_PROGRESS (i.e., 0x7FFFFFFF) from
/// SIDefines.h, this will return the pair as (31,1).
constexpr std::pair<unsigned, unsigned> getShiftMask(unsigned Value) {
  unsigned Shift = 0;
  unsigned Mask = 0;

  Mask = ~Value;
  for (; !(Mask & 1); Shift++, Mask >>= 1) {
  }

  return std::make_pair(Shift, Mask);
}

/// Provided with the MCExpr * \p Val, uint32 \p Mask and \p Shift, will return
/// the masked and left shifted, in said order of operations, MCExpr * created
/// within the MCContext \p Ctx.
///
/// For example, given MCExpr *Val, Mask == 0xf, Shift == 6 the returned MCExpr
/// * will be the equivalent of (Val & 0xf) << 6
inline const MCExpr *maskShiftSet(const MCExpr *Val, uint32_t Mask,
                                  uint32_t Shift, MCContext &Ctx) {
  if (Mask) {
    const MCExpr *MaskExpr = MCConstantExpr::create(Mask, Ctx);
    Val = MCBinaryExpr::createAnd(Val, MaskExpr, Ctx);
  }
  if (Shift) {
    const MCExpr *ShiftExpr = MCConstantExpr::create(Shift, Ctx);
    Val = MCBinaryExpr::createShl(Val, ShiftExpr, Ctx);
  }
  return Val;
}

/// Provided with the MCExpr * \p Val, uint32 \p Mask and \p Shift, will return
/// the right shifted and masked, in said order of operations, MCExpr * created
/// within the MCContext \p Ctx.
///
/// For example, given MCExpr *Val, Mask == 0xf, Shift == 6 the returned MCExpr
/// * will be the equivalent of (Val >> 6) & 0xf
inline const MCExpr *maskShiftGet(const MCExpr *Val, uint32_t Mask,
                                  uint32_t Shift, MCContext &Ctx) {
  if (Shift) {
    const MCExpr *ShiftExpr = MCConstantExpr::create(Shift, Ctx);
    Val = MCBinaryExpr::createLShr(Val, ShiftExpr, Ctx);
  }
  if (Mask) {
    const MCExpr *MaskExpr = MCConstantExpr::create(Mask, Ctx);
    Val = MCBinaryExpr::createAnd(Val, MaskExpr, Ctx);
  }
  return Val;
}

} // end namespace AMDGPU
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_UTILS_SIDEFINESUTILS_H
