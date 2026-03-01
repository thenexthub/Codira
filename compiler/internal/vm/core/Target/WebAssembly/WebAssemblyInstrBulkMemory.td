// WebAssemblyInstrBulkMemory.td - bulk memory codegen support --*- tablegen -*-
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
///
/// \file
/// WebAssembly bulk memory codegen constructs.
///
//===----------------------------------------------------------------------===//

// Instruction requiring HasBulkMemoryOpt and the bulk memory prefix byte
multiclass BULK_I<dag oops_r, dag iops_r, dag oops_s, dag iops_s,
                  list<dag> pattern_r, string asmstr_r = "",
                  string asmstr_s = "", bits<32> simdop = -1> {
  defm "" : I<oops_r, iops_r, oops_s, iops_s, pattern_r, asmstr_r, asmstr_s,
              !or(0xfc00, !and(0xff, simdop))>,
            Requires<[HasBulkMemoryOpt]>;
}

// Bespoke types and nodes for bulk memory ops

def wasm_memcpy_t : SDTypeProfile<0, 5,
  [SDTCisInt<0>, SDTCisInt<1>, SDTCisPtrTy<2>, SDTCisPtrTy<3>, SDTCisInt<4>]
>;
def wasm_memset_t : SDTypeProfile<0, 4,
  [SDTCisInt<0>, SDTCisPtrTy<1>, SDTCisInt<2>, SDTCisInt<3>]
>;

// memory.copy with a branch to avoid trapping in the case of out-of-bounds
// pointers with empty ranges.
def wasm_memcpy : SDNode<"WebAssemblyISD::MEMCPY", wasm_memcpy_t,
                         [SDNPHasChain, SDNPMayLoad, SDNPMayStore]>;

// memory.fill with a branch to avoid trapping in the case of out-of-bounds
// pointers with empty ranges.
def wasm_memset : SDNode<"WebAssemblyISD::MEMSET", wasm_memset_t,
                         [SDNPHasChain, SDNPMayStore]>;

// A multiclass for defining Wasm's raw bulk-memory `memory.*` instructions.
// `memory.copy` and `memory.fill` have Wasm's behavior rather than
// `memcpy`/`memset` behavior.
multiclass BulkMemoryOps<WebAssemblyRegClass rc, string B> {

let mayStore = 1, hasSideEffects = 1 in
defm INIT_A#B :
  BULK_I<(outs),
         (ins i32imm_op:$seg, i32imm_op:$idx, rc:$dest,
              I32:$offset, I32:$size),
         (outs), (ins i32imm_op:$seg, i32imm_op:$idx),
         [],
         "memory.init\t$seg, $idx, $dest, $offset, $size",
         "memory.init\t$seg, $idx", 0x08>;

let mayLoad = 1, mayStore = 1 in
defm COPY_A#B :
  BULK_I<(outs), (ins i32imm_op:$src_idx, i32imm_op:$dst_idx,
                      rc:$dst, rc:$src, rc:$len),
         (outs), (ins i32imm_op:$src_idx, i32imm_op:$dst_idx),
         [],
         "memory.copy\t$src_idx, $dst_idx, $dst, $src, $len",
         "memory.copy\t$src_idx, $dst_idx", 0x0a>;

let mayStore = 1 in
defm FILL_A#B :
  BULK_I<(outs), (ins i32imm_op:$idx, rc:$dst, I32:$value, rc:$size),
         (outs), (ins i32imm_op:$idx),
         [],
         "memory.fill\t$idx, $dst, $value, $size",
         "memory.fill\t$idx", 0x0b>;
}

defm MEMORY_ : BulkMemoryOps<I32, "32">;
defm MEMORY_ : BulkMemoryOps<I64, "64">;

// A multiclass for defining `memcpy`/`memset` pseudo instructions. These have
// the behavior the rest of LLVM CodeGen expects, and we lower them into code
// sequences that include the Wasm `memory.fill` and `memory.copy` instructions
// using custom inserters, because they introduce new control flow.
multiclass BulkMemOps<WebAssemblyRegClass rc, string B> {

let usesCustomInserter = 1, isCodeGenOnly = 1, mayLoad = 1, mayStore = 1 in
defm CPY_A#B : I<(outs), (ins i32imm_op:$src_idx, i32imm_op:$dst_idx,
                              rc:$dst, rc:$src, rc:$len),
                 (outs), (ins i32imm_op:$src_idx, i32imm_op:$dst_idx),
                 [(wasm_memcpy (i32 imm:$src_idx), (i32 imm:$dst_idx),
                   rc:$dst, rc:$src, rc:$len
                 )],
                 "", "", 0>,
                  Requires<[HasBulkMemoryOpt]>;

let usesCustomInserter = 1, isCodeGenOnly = 1, mayStore = 1 in
defm SET_A#B : I<(outs), (ins i32imm_op:$idx, rc:$dst, I32:$value, rc:$size),
                 (outs), (ins i32imm_op:$idx),
                 [(wasm_memset (i32 imm:$idx), rc:$dst, I32:$value, rc:$size)],
                 "", "", 0>,
                 Requires<[HasBulkMemoryOpt]>;

}

defm MEM : BulkMemOps<I32, "32">;
defm MEM : BulkMemOps<I64, "64">;

let hasSideEffects = 1 in
defm DATA_DROP :
  BULK_I<(outs), (ins i32imm_op:$seg), (outs), (ins i32imm_op:$seg),
         [],
         "data.drop\t$seg", "data.drop\t$seg", 0x09>;
