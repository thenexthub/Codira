//===-- BPFRegisterInfo.td - BPF Register defs -------------*- tablegen -*-===//
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
//  Declarations that describe the BPF register file
//===----------------------------------------------------------------------===//

let Namespace = "BPF" in {
  def sub_32 : SubRegIndex<32>;
}

class Wi<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "BPF";
}

// Registers are identified with 4-bit ID numbers.
// Ri - 64-bit integer registers
class Ri<bits<16> Enc, string n, list<Register> subregs>
  : RegisterWithSubRegs<n, subregs> {
  let HWEncoding = Enc;
  let Namespace = "BPF";
  let SubRegIndices = [sub_32];
}

foreach I = 0-11 in {
  // 32-bit Integer (alias to low part of 64-bit register).
  def W#I  : Wi<I,  "w"#I>,  DwarfRegNum<[I]>;
  // 64-bit Integer registers
  def R#I  : Ri<I,  "r"#I,  [!cast<Wi>("W"#I)]>,  DwarfRegNum<[I]>;
}

// Register classes.
def GPR32 : RegisterClass<"BPF", [i32], 64, (add
  (sequence "W%u", 1, 9),
  W0, // Return value
  W11, // Stack Ptr
  W10  // Frame Ptr
)>;

def GPR : RegisterClass<"BPF", [i64], 64, (add
  (sequence "R%u", 1, 9),
  R0, // Return value
  R11, // Stack Ptr
  R10  // Frame Ptr
)>;
