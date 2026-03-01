//===-- X86InstrSNP.td - SNP Instruction Set Extension -----*- tablegen -*-===//
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
// This file describes the instructions that make up the AMD Secure Nested
// Paging (SNP) instruction set.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// SNP instructions

let SchedRW = [WriteSystem] in {
// F3 0F 01 FF
let Uses = [RAX], Defs = [EAX, EFLAGS] in
def PSMASH: I<0x01, MRM_FF, (outs), (ins), "psmash", []>, TB, XS,
            Requires<[In64BitMode]>;

// F2 0F 01 FF
let Uses = [RAX, RCX, RDX], Defs = [EAX, EFLAGS] in
def PVALIDATE64: I<0x01, MRM_FF, (outs), (ins), "pvalidate",[]>,
                 TB, XD, Requires<[In64BitMode]>;

let Uses = [EAX, ECX, EDX], Defs = [EAX, EFLAGS] in
def PVALIDATE32: I<0x01, MRM_FF, (outs), (ins), "pvalidate",[]>,
                 TB, XD, Requires<[Not64BitMode]>;

// F2 0F 01 FE
let Uses = [RAX, RCX], Defs = [EAX, EFLAGS] in
def RMPUPDATE: I<0x01, MRM_FE, (outs), (ins), "rmpupdate", []>, TB, XD,
               Requires<[In64BitMode]>;

// F3 0F 01 FE
let Uses = [RAX, RCX, RDX], Defs = [EAX, EFLAGS] in
def RMPADJUST: I<0x01, MRM_FE, (outs), (ins), "rmpadjust", []>, TB, XS,
               Requires<[In64BitMode]>;

// F3 0F 01 FD
let Uses = [RAX, RDX], Defs = [RAX, RCX, RDX, EFLAGS] in
def RMPQUERY: I<0x01, MRM_FD, (outs), (ins), "rmpquery", []>, TB, XS,
               Requires<[In64BitMode]>;
} // SchedRW

def : InstAlias<"psmash\t{%rax|rax}", (PSMASH)>, Requires<[In64BitMode]>;
def : InstAlias<"pvalidate\t{%rax, %rcx, %rdx|rdx, rcx, rax|}", (PVALIDATE64)>, Requires<[In64BitMode]>;
def : InstAlias<"pvalidate\t{%eax, %ecx, %edx|edx, ecx, eax|}", (PVALIDATE32)>, Requires<[Not64BitMode]>;
def : InstAlias<"rmpupdate\t{%rax, %rcx|rcx, rax|}", (RMPUPDATE)>, Requires<[In64BitMode]>;
def : InstAlias<"rmpadjust\t{%rax, %rcx, %rdx|rdx, rcx, rax|}", (RMPADJUST)>, Requires<[In64BitMode]>;
def : InstAlias<"rmpquery\t{%rax, %rdx|rdx, rax|}", (RMPQUERY)>, Requires<[In64BitMode]>;
