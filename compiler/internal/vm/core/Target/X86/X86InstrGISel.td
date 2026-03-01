//===- X86InstrGISel.td - X86 GISel target specific opcodes -*- tablegen -*===//
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
// X86 GlobalISel target pseudo instruction definitions. This is kept
// separately from the other tablegen files for organizational purposes, but
// share the same infrastructure.
//
//===----------------------------------------------------------------------===//

class X86GenericInstruction : GenericInstruction { let Namespace = "X86"; }

def G_FILD : X86GenericInstruction {
  let OutOperandList = (outs type0:$dst);
  let InOperandList = (ins ptype1:$src);
  let hasSideEffects = false;
  let mayLoad = true;
}
def G_FIST : X86GenericInstruction {
  let OutOperandList = (outs);
  let InOperandList = (ins type0:$src1, ptype1:$src2);
  let hasSideEffects = false;
  let mayStore = true;
}

def G_FNSTCW16 : X86GenericInstruction {
  let OutOperandList = (outs);
  let InOperandList = (ins ptype0:$dst);
  let hasSideEffects = true;
  let mayStore = true;
}

def G_FLDCW16 : X86GenericInstruction {
  let OutOperandList = (outs);
  let InOperandList = (ins ptype0:$src);
  let hasSideEffects = true;
  let mayLoad = true;
}

def : GINodeEquiv<G_FILD, X86fild>;
def : GINodeEquiv<G_FIST, X86fp_to_mem>;
def : GINodeEquiv<G_FNSTCW16, X86fp_cwd_get16>;
def : GINodeEquiv<G_FLDCW16, X86fp_cwd_set16>;
