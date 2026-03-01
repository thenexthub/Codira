//===----------------------------------------------------------------------===//
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
// Automatically generated file, do not edit!
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONDEPITYPES_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONDEPITYPES_H

namespace vm::core {
namespace HexagonII {
enum Type {
  TypeALU32_2op = 0,
  TypeALU32_3op = 1,
  TypeALU32_ADDI = 2,
  TypeALU64 = 3,
  TypeCODE = 4,
  TypeCR = 5,
  TypeCVI_4SLOT_MPY = 6,
  TypeCVI_GATHER = 7,
  TypeCVI_GATHER_DV = 8,
  TypeCVI_GATHER_RST = 9,
  TypeCVI_HIST = 10,
  TypeCVI_SCATTER = 11,
  TypeCVI_SCATTER_DV = 12,
  TypeCVI_SCATTER_NEW_RST = 13,
  TypeCVI_SCATTER_NEW_ST = 14,
  TypeCVI_SCATTER_RST = 15,
  TypeCVI_VA = 16,
  TypeCVI_VA_DV = 17,
  TypeCVI_VM_LD = 18,
  TypeCVI_VM_NEW_ST = 19,
  TypeCVI_VM_ST = 20,
  TypeCVI_VM_STU = 21,
  TypeCVI_VM_TMP_LD = 22,
  TypeCVI_VM_VP_LDU = 23,
  TypeCVI_VP = 24,
  TypeCVI_VP_VS = 25,
  TypeCVI_VS = 26,
  TypeCVI_VS_VX = 27,
  TypeCVI_VX = 28,
  TypeCVI_VX_DV = 29,
  TypeCVI_VX_LATE = 30,
  TypeCVI_ZW = 31,
  TypeDUPLEX = 32,
  TypeENDLOOP = 33,
  TypeEXTENDER = 34,
  TypeJ = 35,
  TypeLD = 36,
  TypeM = 37,
  TypeMAPPING = 38,
  TypeNCODE = 39,
  TypePSEUDO = 40,
  TypeST = 41,
  TypeSUBINSN = 42,
  TypeS_2op = 43,
  TypeS_3op = 44,
  TypeV2LDST = 47,
  TypeV4LDST = 48,
};
}
}

#endif  // LLVM_LIB_TARGET_HEXAGON_HEXAGONDEPITYPES_H
