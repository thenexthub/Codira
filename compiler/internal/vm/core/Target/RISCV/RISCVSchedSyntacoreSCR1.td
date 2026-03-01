//==- RISCVSchedSyntacoreSCR1.td - Syntacore SCR1 Scheduling Definitions --------*- tablegen -*-=//
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

//===----------------------------------------------------------------------===//

// SCR1: https://github.com/syntacore/scr1

// This model covers SYNTACORE_SCR1_CFG_RV32IMC_MAX configuration (syntacore-scr1-max).
// SYNTACORE_SCR1_CFG_RV32IC_BASE (syntacore-scr1-base) configuration has essentially
// same scheduling characteristics.

// SCR1 is single-issue in-order processor
def SyntacoreSCR1Model : SchedMachineModel {
  let MicroOpBufferSize = 0;
  let IssueWidth = 1;
  let LoadLatency = 2;
  let MispredictPenalty = 3;
  let CompleteModel = 0;
  let UnsupportedFeatures = [HasStdExtZbkb, HasStdExtZbkc, HasStdExtZbkx,
                             HasStdExtZknd, HasStdExtZkne, HasStdExtZknh,
                             HasStdExtZksed, HasStdExtZksh, HasStdExtZkr,
                             HasVInstructions];
}

let SchedModel = SyntacoreSCR1Model in {

let BufferSize = 0 in {
def SCR1_ALU : ProcResource<1>;
def SCR1_LSU : ProcResource<1>;
def SCR1_MUL : ProcResource<1>;
def SCR1_DIV : ProcResource<1>;
def SCR1_CFU : ProcResource<1>;
}

// Branching
def : WriteRes<WriteJmp, [SCR1_CFU]>;
def : WriteRes<WriteJal, [SCR1_CFU]>;
def : WriteRes<WriteJalr, [SCR1_CFU]>;

// Integer arithmetic and logic
def : WriteRes<WriteIALU32, [SCR1_ALU]>;
def : WriteRes<WriteIALU, [SCR1_ALU]>;
def : WriteRes<WriteShiftImm32, [SCR1_ALU]>;
def : WriteRes<WriteShiftImm, [SCR1_ALU]>;
def : WriteRes<WriteShiftReg32, [SCR1_ALU]>;
def : WriteRes<WriteShiftReg, [SCR1_ALU]>;

// Integer multiplication: single-cycle multiplier in SCR1_CFG_RV32IMC_MAX
def : WriteRes<WriteIMul, [SCR1_MUL]>;
def : WriteRes<WriteIMul32, [SCR1_MUL]>;

// Integer division/remainder: latency 33, inverse throughput 33
let Latency = 33, ReleaseAtCycles = [33] in {
def : WriteRes<WriteIDiv32, [SCR1_DIV]>;
def : WriteRes<WriteIDiv, [SCR1_DIV]>;
def : WriteRes<WriteIRem32, [SCR1_DIV]>;
def : WriteRes<WriteIRem, [SCR1_DIV]>;
}

// Load/store instructions on SCR1 have latency 2 and inverse throughput 2
// (SCR1_CFG_RV32IMC_MAX includes TCM)
let Latency = 2, ReleaseAtCycles=[2] in {
// Memory
def : WriteRes<WriteSTB, [SCR1_LSU]>;
def : WriteRes<WriteSTH, [SCR1_LSU]>;
def : WriteRes<WriteSTW, [SCR1_LSU]>;
def : WriteRes<WriteSTD, [SCR1_LSU]>;
def : WriteRes<WriteLDB, [SCR1_LSU]>;
def : WriteRes<WriteLDH, [SCR1_LSU]>;
def : WriteRes<WriteLDW, [SCR1_LSU]>;
def : WriteRes<WriteLDD, [SCR1_LSU]>;
}

// Others
def : WriteRes<WriteCSR, []>;
def : WriteRes<WriteNop, []>;

def : InstRW<[WriteIALU], (instrs COPY)>;

//===----------------------------------------------------------------------===//
// Bypasses (none)
def : ReadAdvance<ReadJmp, 0>;
def : ReadAdvance<ReadJalr, 0>;
def : ReadAdvance<ReadCSR, 0>;
def : ReadAdvance<ReadStoreData, 0>;
def : ReadAdvance<ReadMemBase, 0>;
def : ReadAdvance<ReadIALU, 0>;
def : ReadAdvance<ReadIALU32, 0>;
def : ReadAdvance<ReadShiftImm, 0>;
def : ReadAdvance<ReadShiftImm32, 0>;
def : ReadAdvance<ReadShiftReg, 0>;
def : ReadAdvance<ReadShiftReg32, 0>;
def : ReadAdvance<ReadIDiv, 0>;
def : ReadAdvance<ReadIDiv32, 0>;
def : ReadAdvance<ReadIRem, 0>;
def : ReadAdvance<ReadIRem32, 0>;
def : ReadAdvance<ReadIMul, 0>;
def : ReadAdvance<ReadIMul32, 0>;

//===----------------------------------------------------------------------===//
// Unsupported extensions
defm : UnsupportedSchedA;
defm : UnsupportedSchedF;
defm : UnsupportedSchedSFB;
defm : UnsupportedSchedV;
defm : UnsupportedSchedZabha;
defm : UnsupportedSchedZba;
defm : UnsupportedSchedZbb;
defm : UnsupportedSchedZbc;
defm : UnsupportedSchedZbs;
defm : UnsupportedSchedZbkb;
defm : UnsupportedSchedZbkx;
defm : UnsupportedSchedZfa;
defm : UnsupportedSchedZvk;
defm : UnsupportedSchedXsf;
}
