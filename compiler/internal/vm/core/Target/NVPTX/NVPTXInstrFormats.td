//===- NVPTXInstrFormats.td - NVPTX Instruction Formats-------*- tblgen -*-===//
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
//  Describe NVPTX instructions format
//
//===----------------------------------------------------------------------===//

// Generic NVPTX Format

class NVPTXInst<dag outs, dag ins, string asmstr, list<dag> pattern = []>
  : Instruction {
  field bits<14> Inst;

  let Namespace = "NVPTX";
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;

  // TSFlagFields
  bit IsLoad = false;
  bit IsStore = false;

  bit IsTex = false;
  bit IsSust = false;
  bit IsSurfTexQuery = false;
  bit IsTexModeUnified = false;

  // The following field is encoded as log2 of the vector size minus one,
  // with 0 meaning the operation is not a surface instruction.  For example,
  // if IsSuld == 2, then the instruction is a suld instruction with vector size
  // 2**(2-1) = 2.
  bits<2> IsSuld = 0;

  let TSFlags{4}      = IsLoad;
  let TSFlags{5}      = IsStore;
  let TSFlags{6}      = IsTex;
  let TSFlags{8...7}  = IsSuld;
  let TSFlags{9}      = IsSust;
  let TSFlags{10}     = IsSurfTexQuery;
  let TSFlags{11}     = IsTexModeUnified;
}
