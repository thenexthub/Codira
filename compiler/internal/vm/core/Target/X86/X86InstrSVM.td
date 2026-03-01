//===-- X86InstrSVM.td - SVM Instruction Set Extension -----*- tablegen -*-===//
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
// This file describes the instructions that make up the AMD SVM instruction
// set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// SVM instructions

let SchedRW = [WriteSystem] in {
// 0F 01 D9
def VMMCALL : I<0x01, MRM_D9, (outs), (ins), "vmmcall", []>, TB;

// 0F 01 DC
def STGI : I<0x01, MRM_DC, (outs), (ins), "stgi", []>, TB;

// 0F 01 DD
def CLGI : I<0x01, MRM_DD, (outs), (ins), "clgi", []>, TB;

// 0F 01 DE
let Uses = [EAX] in
def SKINIT : I<0x01, MRM_DE, (outs), (ins), "skinit", []>, TB;

// 0F 01 D8
let Uses = [EAX] in
def VMRUN32 : I<0x01, MRM_D8, (outs), (ins), "vmrun", []>, TB,
                Requires<[Not64BitMode]>;
let Uses = [RAX] in
def VMRUN64 : I<0x01, MRM_D8, (outs), (ins), "vmrun", []>, TB,
                Requires<[In64BitMode]>;

// 0F 01 DA
let Uses = [EAX] in
def VMLOAD32 : I<0x01, MRM_DA, (outs), (ins), "vmload", []>, TB,
                 Requires<[Not64BitMode]>;
let Uses = [RAX] in
def VMLOAD64 : I<0x01, MRM_DA, (outs), (ins), "vmload", []>, TB,
                 Requires<[In64BitMode]>;

// 0F 01 DB
let Uses = [EAX] in
def VMSAVE32 : I<0x01, MRM_DB, (outs), (ins), "vmsave", []>, TB,
                 Requires<[Not64BitMode]>;
let Uses = [RAX] in
def VMSAVE64 : I<0x01, MRM_DB, (outs), (ins), "vmsave", []>, TB,
                 Requires<[In64BitMode]>;

// 0F 01 DF
let Uses = [EAX, ECX] in
def INVLPGA32 : I<0x01, MRM_DF, (outs), (ins),
                "invlpga", []>, TB, Requires<[Not64BitMode]>;
let Uses = [RAX, ECX] in
def INVLPGA64 : I<0x01, MRM_DF, (outs), (ins),
                "invlpga", []>, TB, Requires<[In64BitMode]>;
} // SchedRW
