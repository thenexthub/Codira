//===- XtensaCallingConv.td - Xtensa Calling Conventions -*- tablegen ---*-===//
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
// This describes the calling conventions for the Xtensa ABI.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Xtensa base calling convention
//===----------------------------------------------------------------------===//
// Xtensa return value
def RetCC_Xtensa : CallingConv<[
  // First two return values go in a2, a3, a4, a5
  CCIfType<[i32], CCAssignToReg<[A2, A3, A4, A5]>>,
  CCIfType<[f32], CCAssignToReg<[A2, A3, A4, A5]>>,
  CCIfType<[i64], CCAssignToRegWithShadow<[A2, A4], [A3, A5]>>
]>;

// Callee-saved register lists
def CSR_Xtensa : CalleeSavedRegs<(add A0, A12, A13, A14, A15)>;

//===----------------------------------------------------------------------===//
// Xtensa windowed calling convention. Currently by default implemented
// rotation window by 8 registers.
//===----------------------------------------------------------------------===//
// Xtensa return value for 8 registers window
def RetCCW8_Xtensa : CallingConv<[
  //First two return values go in a10, a11, a12, a13
  CCIfType<[i32], CCAssignToReg<[A10, A11, A12, A13]>>,
  CCIfType<[i64], CCAssignToRegWithShadow<[A10, A12], [A11, A13]>>
]>;

// Callee-saved register lists for rotation window by 8 registers
def CSRW8_Xtensa : CalleeSavedRegs<(add)> {
  let OtherPreserved = (add A0, SP, A2, A3, A4, A5, A6, A7);
}
