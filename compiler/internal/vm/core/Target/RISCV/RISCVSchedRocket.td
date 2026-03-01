//==- RISCVSchedRocket.td - Rocket Scheduling Definitions ----*- tablegen -*-=//
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

// ===---------------------------------------------------------------------===//
// The following definitions describe the simpler per-operand machine model.
// This works with MachineScheduler. See MCSchedule.h for details.

// Rocket machine model for scheduling and other instruction cost heuristics.
def RocketModel : SchedMachineModel {
  let MicroOpBufferSize = 0; // Rocket is in-order.
  let IssueWidth = 1;        // 1 micro-op is dispatched per cycle.
  let LoadLatency = 3;
  let MispredictPenalty = 3;
  let CompleteModel = false;
  let UnsupportedFeatures = [HasStdExtZbkb, HasStdExtZbkc, HasStdExtZbkx,
                             HasStdExtZcmt, HasStdExtZknd, HasStdExtZkne,
                             HasStdExtZknh, HasStdExtZksed, HasStdExtZksh,
                             HasStdExtZkr, HasVInstructions, HasVInstructionsI64];
}

//===----------------------------------------------------------------------===//
// Define each kind of processor resource and number available.

// Modeling each pipeline as a ProcResource using the BufferSize = 0 since
// Rocket is in-order.

let BufferSize = 0 in {
def RocketUnitALU        : ProcResource<1>; // Int ALU
def RocketUnitIMul       : ProcResource<1>; // Int Multiply
def RocketUnitMem        : ProcResource<1>; // Load/Store
def RocketUnitB          : ProcResource<1>; // Branch

def RocketUnitFPALU      : ProcResource<1>; // FP ALU
}

let BufferSize = 1 in {
def RocketUnitIDiv       : ProcResource<1>; // Int Division
def RocketUnitFPDivSqrt  : ProcResource<1>; // FP Divide/Sqrt
}

//===----------------------------------------------------------------------===//

let SchedModel = RocketModel in {

// Branching
def : WriteRes<WriteJmp, [RocketUnitB]>;
def : WriteRes<WriteJal, [RocketUnitB]>;
def : WriteRes<WriteJalr, [RocketUnitB]>;

// Integer arithmetic and logic
def : WriteRes<WriteIALU32, [RocketUnitALU]>;
def : WriteRes<WriteIALU, [RocketUnitALU]>;
def : WriteRes<WriteShiftImm32, [RocketUnitALU]>;
def : WriteRes<WriteShiftImm, [RocketUnitALU]>;
def : WriteRes<WriteShiftReg32, [RocketUnitALU]>;
def : WriteRes<WriteShiftReg, [RocketUnitALU]>;

// Integer multiplication
let Latency = 4 in {
def : WriteRes<WriteIMul, [RocketUnitIMul]>;
def : WriteRes<WriteIMul32, [RocketUnitIMul]>;
}

// Integer division
// Worst case latency is used.
def : WriteRes<WriteIDiv32, [RocketUnitIDiv]> {
  let Latency = 34;
  let ReleaseAtCycles = [34];
}
def : WriteRes<WriteIDiv, [RocketUnitIDiv]> {
  let Latency = 33;
  let ReleaseAtCycles = [33];
}

// Integer remainder
def : WriteRes<WriteIRem32, [RocketUnitIDiv]> {
  let Latency = 34;
  let ReleaseAtCycles = [34];
}
def : WriteRes<WriteIRem, [RocketUnitIDiv]> {
  let Latency = 33;
  let ReleaseAtCycles = [33];
}

// Memory
def : WriteRes<WriteSTB, [RocketUnitMem]>;
def : WriteRes<WriteSTH, [RocketUnitMem]>;
def : WriteRes<WriteSTW, [RocketUnitMem]>;
def : WriteRes<WriteSTD, [RocketUnitMem]>;
def : WriteRes<WriteFST32, [RocketUnitMem]>;
def : WriteRes<WriteFST64, [RocketUnitMem]>;

let Latency = 3 in {
def : WriteRes<WriteLDB, [RocketUnitMem]>;
def : WriteRes<WriteLDH, [RocketUnitMem]>;
}

let Latency = 2 in {
def : WriteRes<WriteLDW, [RocketUnitMem]>;
def : WriteRes<WriteLDD, [RocketUnitMem]>;
def : WriteRes<WriteFLD32, [RocketUnitMem]>;
def : WriteRes<WriteFLD64, [RocketUnitMem]>;

// Atomic memory
def : WriteRes<WriteAtomicW, [RocketUnitMem]>;
def : WriteRes<WriteAtomicD, [RocketUnitMem]>;

def : WriteRes<WriteAtomicLDW, [RocketUnitMem]>;
def : WriteRes<WriteAtomicLDD, [RocketUnitMem]>;
}

def : WriteRes<WriteAtomicSTW, [RocketUnitMem]>;
def : WriteRes<WriteAtomicSTD, [RocketUnitMem]>;

// Single precision.
let Latency = 4 in {
def : WriteRes<WriteFAdd32, [RocketUnitFPALU]>;
def : WriteRes<WriteFSGNJ32, [RocketUnitFPALU]>;
def : WriteRes<WriteFMinMax32, [RocketUnitFPALU]>;
}

// Double precision
let Latency = 6 in {
def : WriteRes<WriteFAdd64, [RocketUnitFPALU]>;
def : WriteRes<WriteFSGNJ64, [RocketUnitFPALU]>;
def : WriteRes<WriteFMinMax64, [RocketUnitFPALU]>;
}

// Conversions
let Latency = 2 in {
def : WriteRes<WriteFCvtI32ToF32, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtI32ToF64, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtI64ToF32, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtI64ToF64, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtF32ToI32, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtF32ToI64, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtF64ToI32, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtF64ToI64, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtF32ToF64, [RocketUnitFPALU]>;
def : WriteRes<WriteFCvtF64ToF32, [RocketUnitFPALU]>;

def : WriteRes<WriteFClass32, [RocketUnitFPALU]>;
def : WriteRes<WriteFClass64, [RocketUnitFPALU]>;
def : WriteRes<WriteFCmp32, [RocketUnitFPALU]>;
def : WriteRes<WriteFCmp64, [RocketUnitFPALU]>;
def : WriteRes<WriteFMovF32ToI32, [RocketUnitFPALU]>;
def : WriteRes<WriteFMovI32ToF32, [RocketUnitFPALU]>;
def : WriteRes<WriteFMovF64ToI64, [RocketUnitFPALU]>;
def : WriteRes<WriteFMovI64ToF64, [RocketUnitFPALU]>;
}

// FP multiplication
let Latency = 5 in {
def : WriteRes<WriteFMul32, [RocketUnitFPALU]>;
def : WriteRes<WriteFMA32, [RocketUnitFPALU]>;
}

let Latency = 7 in {
def : WriteRes<WriteFMul64, [RocketUnitFPALU]>;
def : WriteRes<WriteFMA64, [RocketUnitFPALU]>;
}

// FP division
// FP division unit on Rocket is not pipelined, so set resource cycles to latency.
let Latency = 20, ReleaseAtCycles = [20] in {
def : WriteRes<WriteFDiv32, [RocketUnitFPDivSqrt]>;
def : WriteRes<WriteFDiv64, [RocketUnitFPDivSqrt]>;
}

// FP square root unit on Rocket is not pipelined, so set resource cycles to latency.
def : WriteRes<WriteFSqrt32, [RocketUnitFPDivSqrt]> { let Latency = 20;
                                                      let ReleaseAtCycles = [20]; }
def : WriteRes<WriteFSqrt64, [RocketUnitFPDivSqrt]> { let Latency = 25;
                                                      let ReleaseAtCycles = [25]; }

// Others
def : WriteRes<WriteCSR, []>;
def : WriteRes<WriteNop, []>;

def : InstRW<[WriteIALU], (instrs COPY)>;

//===----------------------------------------------------------------------===//
// Bypass and advance
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
def : ReadAdvance<ReadAtomicWA, 0>;
def : ReadAdvance<ReadAtomicWD, 0>;
def : ReadAdvance<ReadAtomicDA, 0>;
def : ReadAdvance<ReadAtomicDD, 0>;
def : ReadAdvance<ReadAtomicLDW, 0>;
def : ReadAdvance<ReadAtomicLDD, 0>;
def : ReadAdvance<ReadAtomicSTW, 0>;
def : ReadAdvance<ReadAtomicSTD, 0>;
def : ReadAdvance<ReadFStoreData, 0>;
def : ReadAdvance<ReadFMemBase, 0>;
def : ReadAdvance<ReadFAdd32, 0>;
def : ReadAdvance<ReadFAdd64, 0>;
def : ReadAdvance<ReadFMul32, 0>;
def : ReadAdvance<ReadFMul64, 0>;
def : ReadAdvance<ReadFMA32, 0>;
def : ReadAdvance<ReadFMA32Addend, 0>;
def : ReadAdvance<ReadFMA64, 0>;
def : ReadAdvance<ReadFMA64Addend, 0>;
def : ReadAdvance<ReadFDiv32, 0>;
def : ReadAdvance<ReadFDiv64, 0>;
def : ReadAdvance<ReadFSqrt32, 0>;
def : ReadAdvance<ReadFSqrt64, 0>;
def : ReadAdvance<ReadFCmp32, 0>;
def : ReadAdvance<ReadFCmp64, 0>;
def : ReadAdvance<ReadFSGNJ32, 0>;
def : ReadAdvance<ReadFSGNJ64, 0>;
def : ReadAdvance<ReadFMinMax32, 0>;
def : ReadAdvance<ReadFMinMax64, 0>;
def : ReadAdvance<ReadFCvtF32ToI32, 0>;
def : ReadAdvance<ReadFCvtF32ToI64, 0>;
def : ReadAdvance<ReadFCvtF64ToI32, 0>;
def : ReadAdvance<ReadFCvtF64ToI64, 0>;
def : ReadAdvance<ReadFCvtI32ToF32, 0>;
def : ReadAdvance<ReadFCvtI32ToF64, 0>;
def : ReadAdvance<ReadFCvtI64ToF32, 0>;
def : ReadAdvance<ReadFCvtI64ToF64, 0>;
def : ReadAdvance<ReadFCvtF32ToF64, 0>;
def : ReadAdvance<ReadFCvtF64ToF32, 0>;
def : ReadAdvance<ReadFMovF32ToI32, 0>;
def : ReadAdvance<ReadFMovI32ToF32, 0>;
def : ReadAdvance<ReadFMovF64ToI64, 0>;
def : ReadAdvance<ReadFMovI64ToF64, 0>;
def : ReadAdvance<ReadFClass32, 0>;
def : ReadAdvance<ReadFClass64, 0>;

//===----------------------------------------------------------------------===//
// Unsupported extensions
defm : UnsupportedSchedQ;
defm : UnsupportedSchedV;
defm : UnsupportedSchedZabha;
defm : UnsupportedSchedZba;
defm : UnsupportedSchedZbb;
defm : UnsupportedSchedZbc;
defm : UnsupportedSchedZbs;
defm : UnsupportedSchedZbkb;
defm : UnsupportedSchedZbkx;
defm : UnsupportedSchedZfa;
defm : UnsupportedSchedZfhmin;
defm : UnsupportedSchedSFB;
defm : UnsupportedSchedZvk;
defm : UnsupportedSchedXsf;
}
