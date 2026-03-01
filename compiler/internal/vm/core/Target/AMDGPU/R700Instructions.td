//===-- R700Instructions.td - R700 Instruction defs  -------*- tablegen -*-===//
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
// TableGen definitions for instructions which are:
// - Available to R700 and newer VLIW4/VLIW5 GPUs
// - Available only on R700 family GPUs.
//
//===----------------------------------------------------------------------===//

def isR700 : Predicate<"Subtarget->getGeneration() == AMDGPUSubtarget::R700">;

let Predicates = [isR700] in {
  def SIN_r700 : SIN_Common<0x6E>;
  def COS_r700 : COS_Common<0x6F>;
}
