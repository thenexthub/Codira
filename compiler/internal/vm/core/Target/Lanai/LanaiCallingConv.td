//===- LanaiCallingConv.td - Calling Conventions Lanai -------*- tablegen -*-===//
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
// This describes the calling conventions for the Lanai architectures.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Return Value Calling Conventions
//===----------------------------------------------------------------------===//

// Lanai 32-bit C Calling convention.
def CC_Lanai32 : CallingConv<[
  // Promote i8/i16 args to i32
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // Put argument in registers if marked 'inreg' and not a vararg call.
  CCIfNotVarArg<CCIfInReg<CCIfType<[i32],
                                   CCAssignToReg<[R6, R7, R18, R19]>>>>,

  // Otherwise they are assigned to the stack in 4-byte aligned units.
  CCAssignToStack<4, 4>
]>;

// Lanai 32-bit Fast Calling convention.
def CC_Lanai32_Fast : CallingConv<[
  // Promote i8/i16 args to i32
  CCIfType<[ i8, i16 ], CCPromoteToType<i32>>,

  // Put arguments in registers.
  CCIfNotVarArg<CCIfType<[i32], CCAssignToReg<[ R6, R7, R18, R19 ]>>>,

  // Otherwise they are assigned to the stack in 4-byte aligned units.
  CCAssignToStack<4, 4>
]>;

// Lanai 32-bit C return-value convention.
def RetCC_Lanai32 : CallingConv<[
  // Specify two registers to allow returning 64-bit results that have already
  // been lowered to 2 32-bit values.
  CCIfType<[i32], CCAssignToReg<[RV, R9]>>
]>;

def CSR: CalleeSavedRegs<(add)>;
