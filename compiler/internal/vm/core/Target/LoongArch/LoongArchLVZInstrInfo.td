//===- LoongArchLVZInstrInfo.td - LoongArch LVZ instructions -*- tablegen -*-=//
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
// This file describes the LVZ extension instructions.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

let hasSideEffects = 1, mayLoad = 0, mayStore = 0, Predicates = [HasExtLVZ] in {

def GCSRRD : FmtCSR<0x05000000, (outs GPR:$rd), (ins uimm14:$csr_num),
                    "$rd, $csr_num">;

let Constraints = "$rd = $dst" in {
def GCSRWR : FmtCSR<0x05000020, (outs GPR:$dst),
                    (ins GPR:$rd, uimm14:$csr_num), "$rd, $csr_num">;
def GCSRXCHG : FmtCSRXCHG<0x05000000, (outs GPR:$dst),
                          (ins GPR:$rd, GPRNoR0R1:$rj, uimm14:$csr_num),
                          "$rd, $rj, $csr_num">;
} // Constraints = "$rd = $dst"

def GTLBFLUSH : FmtI32<0x06482401>;
def HVCL : MISC_I15<0x002b8000>;

} // hasSideEffects = 1, mayLoad = 0, mayStore = 0, Predicates = [HasExtLVZ]
