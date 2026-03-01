//===-- BPFCallingConv.td - Calling Conventions BPF --------*- tablegen -*-===//
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
// This describes the calling conventions for the BPF architecture.
//
//===----------------------------------------------------------------------===//

// BPF 64-bit C return-value convention.
def RetCC_BPF64 : CallingConv<[CCIfType<[i64], CCAssignToReg<[R0]>>]>;

// BPF 64-bit C Calling convention.
def CC_BPF64 : CallingConv<[
  // Promote i8/i16/i32 args to i64
  CCIfType<[ i8, i16, i32 ], CCPromoteToType<i64>>,

  // All arguments get passed in integer registers if there is space.
  CCIfType<[i64], CCAssignToReg<[ R1, R2, R3, R4, R5 ]>>,

  // Could be assigned to the stack in 8-byte aligned units, but unsupported
  CCAssignToStack<8, 8>
]>;

// Return-value convention when -mattr=+alu32 enabled
def RetCC_BPF32 : CallingConv<[
  CCIfType<[i32], CCAssignToRegWithShadow<[W0], [R0]>>,
  CCIfType<[i64], CCAssignToRegWithShadow<[R0], [W0]>>
]>;

// Calling convention when -mattr=+alu32 enabled
def CC_BPF32 : CallingConv<[
  // Promote i8/i16/i32 args to i64
  CCIfType<[i32], CCAssignToRegWithShadow<[W1, W2, W3, W4, W5],
                                          [R1, R2, R3, R4, R5]>>,

  // All arguments get passed in integer registers if there is space.
  CCIfType<[i64], CCAssignToRegWithShadow<[R1, R2, R3, R4, R5],
                                          [W1, W2, W3, W4, W5]>>,

  // Could be assigned to the stack in 8-byte aligned units, but unsupported
  CCAssignToStack<8, 8>
]>;

def CSR : CalleeSavedRegs<(add R6, R7, R8, R9, R10)>;
def CSR_PreserveAll : CalleeSavedRegs<(add R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10)>;
