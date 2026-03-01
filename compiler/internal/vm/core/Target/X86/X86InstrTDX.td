//===- X86InstrTDX.td - TDX Instruction Set Extension -*- tablegen -*===//
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
// This file describes the instructions that make up the Intel TDX instruction
// set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// TDX instructions

// 64-bit only instructions
let SchedRW = [WriteSystem], Predicates = [In64BitMode] in {
// SEAMCALL - Call to SEAM VMX-root Operation Module
def SEAMCALL : I<0x01, MRM_CF, (outs), (ins), "seamcall", []>, TB, PD;

// SEAMRET - Return to Legacy VMX-root Operation
def SEAMRET : I<0x01, MRM_CD, (outs), (ins), "seamret", []>, TB, PD;

// SEAMOPS - SEAM Operations
def SEAMOPS : I<0x01, MRM_CE, (outs), (ins), "seamops", []>, TB, PD;
} // SchedRW

// common instructions
let SchedRW = [WriteSystem] in {
// TDCALL - Call SEAM Module Functions
def TDCALL : I<0x01, MRM_CC, (outs), (ins), "tdcall", []>, TB, PD;
} // SchedRW
