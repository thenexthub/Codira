//==- AArch64SchedFalkor.td - Falkor Scheduling Definitions -*- tablegen -*-==//
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
// This file defines the machine model for Qualcomm Falkor to support
// instruction scheduling and other instruction cost heuristics.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Define the SchedMachineModel and provide basic properties for coarse grained
// instruction cost model.

def FalkorModel : SchedMachineModel {
  let IssueWidth = 8;          // 8 uops are dispatched per cycle.
  let MicroOpBufferSize = 128; // Out-of-order with temporary unified issue buffer.
  let LoopMicroOpBufferSize = 16;
  let LoadLatency = 3;         // Optimistic load latency.
  let MispredictPenalty = 11;  // Minimum branch misprediction penalty.
  let CompleteModel = 1;

  list<Predicate> UnsupportedFeatures = !listconcat(SVEUnsupported.F,
                                                    PAUnsupported.F,
                                                    SMEUnsupported.F,
                                                    [HasMTE, HasCSSC]);
  // FIXME: Remove when all errors have been fixed.
  let FullInstRWOverlapCheck = 0;
}

//===----------------------------------------------------------------------===//
// Define each kind of processor resource and number available on Falkor.

let SchedModel = FalkorModel in {

  def FalkorUnitB    : ProcResource<1>; // Branch
  def FalkorUnitLD   : ProcResource<1>; // Load pipe
  def FalkorUnitSD   : ProcResource<1>; // Store data
  def FalkorUnitST   : ProcResource<1>; // Store pipe
  def FalkorUnitX    : ProcResource<1>; // Complex arithmetic
  def FalkorUnitY    : ProcResource<1>; // Simple arithmetic
  def FalkorUnitZ    : ProcResource<1>; // Simple arithmetic

  def FalkorUnitVSD  : ProcResource<1>; // Vector store data
  def FalkorUnitVX   : ProcResource<1>; // Vector X-pipe
  def FalkorUnitVY   : ProcResource<1>; // Vector Y-pipe

  def FalkorUnitGTOV : ProcResource<1>; // Scalar to Vector
  def FalkorUnitVTOG : ProcResource<1>; // Vector to Scalar

  // Define the resource groups.
  def FalkorUnitXY   : ProcResGroup<[FalkorUnitX, FalkorUnitY]>;
  def FalkorUnitXYZ  : ProcResGroup<[FalkorUnitX, FalkorUnitY, FalkorUnitZ]>;
  def FalkorUnitXYZB : ProcResGroup<[FalkorUnitX, FalkorUnitY, FalkorUnitZ,
                                     FalkorUnitB]>;
  def FalkorUnitZB   : ProcResGroup<[FalkorUnitZ, FalkorUnitB]>;
  def FalkorUnitVXVY : ProcResGroup<[FalkorUnitVX, FalkorUnitVY]>;

}

//===----------------------------------------------------------------------===//
// Map the target-defined scheduler read/write resources and latency for
// Falkor.

let SchedModel = FalkorModel in {

// These WriteRes entries are not used in the Falkor sched model.
def : WriteRes<WriteImm, []>     { let Unsupported = 1; }
def : WriteRes<WriteI, []>       { let Unsupported = 1; }
def : WriteRes<WriteISReg, []>   { let Unsupported = 1; }
def : WriteRes<WriteIEReg, []>   { let Unsupported = 1; }
def : WriteRes<WriteExtr, []>    { let Unsupported = 1; }
def : WriteRes<WriteIS, []>      { let Unsupported = 1; }
def : WriteRes<WriteID32, []>    { let Unsupported = 1; }
def : WriteRes<WriteID64, []>    { let Unsupported = 1; }
def : WriteRes<WriteIM32, []>    { let Unsupported = 1; }
def : WriteRes<WriteIM64, []>    { let Unsupported = 1; }
def : WriteRes<WriteBr, []>      { let Unsupported = 1; }
def : WriteRes<WriteBrReg, []>   { let Unsupported = 1; }
def : WriteRes<WriteLD, []>      { let Unsupported = 1; }
def : WriteRes<WriteST, []>      { let Unsupported = 1; }
def : WriteRes<WriteSTP, []>     { let Unsupported = 1; }
def : WriteRes<WriteAdr, []>     { let Unsupported = 1; }
def : WriteRes<WriteLDIdx, []>   { let Unsupported = 1; }
def : WriteRes<WriteSTIdx, []>   { let Unsupported = 1; }
def : WriteRes<WriteF, []>       { let Unsupported = 1; }
def : WriteRes<WriteFCmp, []>    { let Unsupported = 1; }
def : WriteRes<WriteFCvt, []>    { let Unsupported = 1; }
def : WriteRes<WriteFCopy, []>   { let Unsupported = 1; }
def : WriteRes<WriteFImm, []>    { let Unsupported = 1; }
def : WriteRes<WriteFMul, []>    { let Unsupported = 1; }
def : WriteRes<WriteFDiv, []>    { let Unsupported = 1; }
def : WriteRes<WriteVd, []>      { let Unsupported = 1; }
def : WriteRes<WriteVq, []>      { let Unsupported = 1; }
def : WriteRes<WriteVLD, []>     { let Unsupported = 1; }
def : WriteRes<WriteVST, []>     { let Unsupported = 1; }
def : WriteRes<WriteSys, []>     { let Unsupported = 1; }
def : WriteRes<WriteBarrier, []> { let Unsupported = 1; }
def : WriteRes<WriteHint, []>    { let Unsupported = 1; }
def : WriteRes<WriteLDHi, []>    { let Unsupported = 1; }
def : WriteRes<WriteAtomic, []>  { let Unsupported = 1; }

// These ReadAdvance entries are not used in the Falkor sched model.
def : ReadAdvance<ReadI,       0>;
def : ReadAdvance<ReadISReg,   0>;
def : ReadAdvance<ReadIEReg,   0>;
def : ReadAdvance<ReadIM,      0>;
def : ReadAdvance<ReadIMA,     0>;
def : ReadAdvance<ReadID,      0>;
def : ReadAdvance<ReadExtrHi,  0>;
def : ReadAdvance<ReadAdrBase, 0>;
def : ReadAdvance<ReadVLD,     0>;
def : ReadAdvance<ReadST,      0>;

// Detailed Refinements
// -----------------------------------------------------------------------------
include "AArch64SchedFalkorDetails.td"

}
