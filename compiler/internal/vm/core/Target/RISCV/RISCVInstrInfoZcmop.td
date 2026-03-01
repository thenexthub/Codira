//===-- RISCVInstrInfoZcmop.td -----------------------------*- tablegen -*-===//
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
// This file describes the RISC-V instructions from the standard Compressed
// May-Be-Operations Extension (Zcmop).
//
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class CMOPInst<bits<3> imm3, string opcodestr>
    : RVInst16CI<0b011, 0b01, (outs), (ins), opcodestr, ""> {
  let Inst{6-2} = 0;
  let Inst{7} = 1;
  let Inst{10-8} = imm3;
  let Inst{12-11} = 0;
}

foreach n = [1, 3, 5, 7, 9, 11, 13, 15] in {
  let Predicates = [HasStdExtZcmop] in
  def C_MOP_ # n : CMOPInst<!srl(n, 1), "c.mop." # n>, Sched<[]>;
}
