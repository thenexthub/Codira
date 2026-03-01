//===-- CSKYInstrAlias.td - Target Description for CSKY ----*- tablegen -*-===//
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
// This file describes the CSKY instructions alias.
//
//===----------------------------------------------------------------------===//

def : InstAlias<"nop", (MOV16 R0, R0)>;
def : InstAlias<"nop", (MOV32 R0, R0)>, Requires<[iHasE2]>;

def : InstAlias<"bgeni16 $dst, $imm", (BGENI GPR:$dst, uimm5:$imm)>;
def : InstAlias<"bgeni32 $dst, $imm", (BGENI GPR:$dst, uimm5:$imm)>;

def : InstAlias<"bsr $dst", (BSR32 call_symbol:$dst)>;

def : InstAlias<"grs\t$rz, $offset", (GRS32 GPR:$rz, bare_symbol:$offset)>;

def : InstAlias<"jbsr\t$src1", (JBSR32 call_symbol:$src1)>;

def : InstAlias<"jbr $dst", (JBR16 br_symbol_16bit:$dst)>;
def : InstAlias<"jbt $dst", (JBT16 C, br_symbol_16bit:$dst)>;
def : InstAlias<"jbf $dst", (JBF16 C, br_symbol_16bit:$dst)>;

def : InstAlias<"lrw $rz, $src", (PseudoLRW16 mGPR:$rz, bare_symbol:$src)>;
def : InstAlias<"lrw $rz, $src", (LRW16 mGPR:$rz, constpool_symbol_16bit:$src)>;
def : InstAlias<"lrw $rz, $src", (PseudoLRW32 GPR:$rz, bare_symbol:$src)>;
def : InstAlias<"lrw $rz, $src", (LRW32 GPR:$rz, constpool_symbol:$src)>;

def : InstAlias<"jsri $dst", (PseudoJSRI32 call_symbol:$dst)>;
def : InstAlias<"jsri $dst", (JSRI32 constpool_symbol:$dst)>;

def : InstAlias<"jmpi $dst", (PseudoJMPI32 br_symbol:$dst)>;
def : InstAlias<"jmpi $dst", (JMPI32 constpool_symbol:$dst)>;
