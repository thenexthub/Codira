//===-- R600Schedule.td - R600 Scheduling definitions ------*- tablegen -*-===//
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
// R600 has a VLIW architecture.  On pre-cayman cards there are 5 instruction
// slots ALU.X, ALU.Y, ALU.Z, ALU.W, and TRANS.  For cayman cards, the TRANS
// slot has been removed.
//
//===----------------------------------------------------------------------===//


def ALU_X : FuncUnit;
def ALU_Y : FuncUnit;
def ALU_Z : FuncUnit;
def ALU_W : FuncUnit;
def TRANS : FuncUnit;

def AnyALU : InstrItinClass;
def VecALU : InstrItinClass;
def TransALU : InstrItinClass;
def XALU : InstrItinClass;

def R600_VLIW5_Itin : ProcessorItineraries <
  [ALU_X, ALU_Y, ALU_Z, ALU_W, TRANS, ALU_NULL],
  [],
  [
    InstrItinData<AnyALU, [InstrStage<1, [ALU_X, ALU_Y, ALU_Z, ALU_W, TRANS]>]>,
    InstrItinData<VecALU, [InstrStage<1, [ALU_X, ALU_Y, ALU_Z, ALU_W]>]>,
    InstrItinData<TransALU, [InstrStage<1, [TRANS]>]>,
    InstrItinData<XALU, [InstrStage<1, [ALU_X]>]>,
    InstrItinData<NullALU, [InstrStage<1, [ALU_NULL]>]>
  ]
>;

def R600_VLIW4_Itin : ProcessorItineraries <
  [ALU_X, ALU_Y, ALU_Z, ALU_W, ALU_NULL],
  [],
  [
    InstrItinData<AnyALU, [InstrStage<1, [ALU_X, ALU_Y, ALU_Z, ALU_W]>]>,
    InstrItinData<VecALU, [InstrStage<1, [ALU_X, ALU_Y, ALU_Z, ALU_W]>]>,
    InstrItinData<TransALU, [InstrStage<1, [ALU_NULL]>]>,
    InstrItinData<NullALU, [InstrStage<1, [ALU_NULL]>]>
  ]
>;
