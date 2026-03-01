//===- MipsInstrCompiler.td - Compiler Pseudos and Patterns -*- tablegen -*-===//
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
// This file describes the various pseudo instructions used by the compiler,
// as well as Pat patterns used during instruction selection.
//
//===----------------------------------------------------------------------===//


def shiftMask_32 : PatFrag<(ops node:$lhs), (and node:$lhs, imm), [{
  return isUnneededShiftMask(N, 5);
}]>;

def shiftMask_64 : PatFrag<(ops node:$src0), (and node:$src0, imm), [{
  return isUnneededShiftMask(N, 6);
}]>;

foreach width = [32, 64] in {
defvar shiftMask = !cast<SDPatternOperator>("shiftMask_"#width);
def mshl_#width : PatFrags<(ops node:$src0, node:$src1),
  [(shl node:$src0, node:$src1), (shl node:$src0, (shiftMask node:$src1))]>;

def msrl_#width : PatFrags<(ops node:$src0, node:$src1),
  [(srl node:$src0, node:$src1), (srl node:$src0, (shiftMask node:$src1))]>;

def msra_#width : PatFrags<(ops node:$src0, node:$src1),
  [(sra node:$src0, node:$src1), (sra node:$src0, (shiftMask node:$src1))]>;
}
