//===- ARCInfo.h - Additional ARC Info --------------------------*- C++ -*-===//
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
// This file contains small standalone helper functions and enum definitions for
// the ARC target useful for the compiler back-end and the MC libraries.
// As such, it deliberately does not include references to LLVM core
// code gen types, passes, etc..
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARC_MCTARGETDESC_ARCINFO_H
#define LLVM_LIB_TARGET_ARC_MCTARGETDESC_ARCINFO_H

namespace vm::core {

// Enums corresponding to ARC condition codes
namespace ARCCC {

enum CondCode {
  AL = 0x0,
  EQ = 0x1,
  NE = 0x2,
  P = 0x3,
  N = 0x4,
  LO = 0x5,
  HS = 0x6,
  VS = 0x7,
  VC = 0x8,
  GT = 0x9,
  GE = 0xa,
  LT = 0xb,
  LE = 0xc,
  HI = 0xd,
  LS = 0xe,
  PNZ = 0xf,
  Z = 0x11, // Low 4-bits = EQ
  NZ = 0x12 // Low 4-bits = NE
};

enum BRCondCode {
  BREQ = 0x0,
  BRNE = 0x1,
  BRLT = 0x2,
  BRGE = 0x3,
  BRLO = 0x4,
  BRHS = 0x5
};

} // end namespace ARCCC

} // end namespace vm::core

#endif
