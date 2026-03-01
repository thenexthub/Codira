//===-- AVRCallingConv.td - Calling Conventions for AVR ----*- tablegen -*-===//
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
// This describes the calling conventions for AVR architecture.
// Normal functions use a special calling convention, solved in code.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// AVR Return Value Calling Convention
//===----------------------------------------------------------------------===//

// Special return value calling convention for runtime functions.
def RetCC_AVR_BUILTIN : CallingConv<[
  CCIfType<[i8], CCAssignToReg<[R24, R25]>>,
  CCIfType<[i16], CCAssignToReg<[R23R22, R25R24]>>
]>;

//===----------------------------------------------------------------------===//
// AVR Argument Calling Conventions
//===----------------------------------------------------------------------===//

// The calling conventions are implemented in custom C++ code

// Calling convention for variadic functions.
def ArgCC_AVR_Vararg : CallingConv<[
  // i8 are always passed through the stack with a byte slot and byte alignment.
  CCIfType<[i8], CCAssignToStack<1, 1>>,
  // i16 are always passed through the stack with an alignment of 1.
  CCAssignToStack<2, 1>
]>;

//===----------------------------------------------------------------------===//
// Callee-saved register lists.
//===----------------------------------------------------------------------===//

def CSR_Normal : CalleeSavedRegs<(add R29, R28, (sequence "R%u", 17, 2))>;
def CSR_NormalTiny : CalleeSavedRegs<(add R29, R28, R19, R18)>;
def CSR_Interrupts : CalleeSavedRegs<(add(sequence "R%u", 31, 2))>;
def CSR_InterruptsTiny : CalleeSavedRegs<(add(sequence "R%u", 31, 18))>;
