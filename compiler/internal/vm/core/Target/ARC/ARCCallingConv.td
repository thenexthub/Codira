//===- ARCCallingConv.td - Calling Conventions for ARC -----*- tablegen -*-===//
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
// This describes the calling conventions for ARC architecture.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// ARC Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_ARC : CallingConv<[
  // i32 are returned in registers R0, R1, R2, R3
  CCIfType<[i32, i64], CCAssignToReg<[R0, R1, R2, R3]>>,

  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i64], CCAssignToStack<8, 4>>,
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

//===----------------------------------------------------------------------===//
// ARC Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_ARC : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // The first 8 integer arguments are passed in integer registers.
  CCIfType<[i32, i64], CCAssignToReg<[R0, R1, R2, R3, R4, R5, R6, R7]>>,

  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i64], CCAssignToStack<8, 4>>,
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def CSR_ARC : CalleeSavedRegs<(add (sequence "R%u", 13, 25), GP, FP)>;
