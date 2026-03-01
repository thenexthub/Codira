//===-- PPCRegisterInfoMMA.td - The PowerPC Register File --*- tablegen -*-===//
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
// Register info for registers related to MMA. These are the ACC and UACC
// registers.
//
//===----------------------------------------------------------------------===//

let Namespace = "PPC" in {
def sub_pair0 : SubRegIndex<256>;
def sub_pair1 : SubRegIndex<256, 256>;
}

// ACC - One of the 8 512-bit VSX accumulators.
class ACC<bits<3> num, string n, list<Register> subregs> : PPCReg<n> {
  let HWEncoding{2-0} = num;
  let SubRegs = subregs;
}

// UACC - One of the 8 512-bit VSX accumulators prior to being primed.
// Without using this register class, the register allocator has no way to
// differentiate a primed accumulator from an unprimed accumulator.
// This may result in invalid copies between primed and unprimed accumulators.
class UACC<bits<3> num, string n, list<Register> subregs> : PPCReg<n> {
  let HWEncoding{2-0} = num;
  let SubRegs = subregs;
}

// SPE Accumulator for multiply-accumulate SPE operations.  Never directly
// accessed, so there's no real encoding for it.
def SPEACC: DwarfRegNum<[99, 111]>;

let SubRegIndices = [sub_pair0, sub_pair1] in {
  def ACC0 : ACC<0, "acc0", [VSRp0, VSRp1]>, DwarfRegNum<[-1, -1]>;
  def ACC1 : ACC<1, "acc1", [VSRp2, VSRp3]>, DwarfRegNum<[-1, -1]>;
  def ACC2 : ACC<2, "acc2", [VSRp4, VSRp5]>, DwarfRegNum<[-1, -1]>;
  def ACC3 : ACC<3, "acc3", [VSRp6, VSRp7]>, DwarfRegNum<[-1, -1]>;
  def ACC4 : ACC<4, "acc4", [VSRp8, VSRp9]>, DwarfRegNum<[-1, -1]>;
  def ACC5 : ACC<5, "acc5", [VSRp10, VSRp11]>, DwarfRegNum<[-1, -1]>;
  def ACC6 : ACC<6, "acc6", [VSRp12, VSRp13]>, DwarfRegNum<[-1, -1]>;
  def ACC7 : ACC<7, "acc7", [VSRp14, VSRp15]>, DwarfRegNum<[-1, -1]>;
}
def ACCRC : RegisterClass<"PPC", [v512i1], 128, (add ACC0, ACC1, ACC2, ACC3,
                                                      ACC4, ACC5, ACC6, ACC7)> {
  // The AllocationPriority is in the range [0, 31]. Assigned the ACC registers
  // the highest possible priority in this range to force the register allocator
  // to assign these registers first. This is done because the ACC registers
  // must represent 4 advacent vector registers. For example ACC1 must be
  // VS4 - VS7.
  let AllocationPriority = 31;

  // We want to allocate these registers even before we allocate
  // global ranges.
  let GlobalPriority = true;
  let Size = 512;
}

let SubRegIndices = [sub_pair0, sub_pair1] in {
  def UACC0 : UACC<0, "acc0", [VSRp0, VSRp1]>, DwarfRegNum<[-1, -1]>;
  def UACC1 : UACC<1, "acc1", [VSRp2, VSRp3]>, DwarfRegNum<[-1, -1]>;
  def UACC2 : UACC<2, "acc2", [VSRp4, VSRp5]>, DwarfRegNum<[-1, -1]>;
  def UACC3 : UACC<3, "acc3", [VSRp6, VSRp7]>, DwarfRegNum<[-1, -1]>;
  def UACC4 : UACC<4, "acc4", [VSRp8, VSRp9]>, DwarfRegNum<[-1, -1]>;
  def UACC5 : UACC<5, "acc5", [VSRp10, VSRp11]>, DwarfRegNum<[-1, -1]>;
  def UACC6 : UACC<6, "acc6", [VSRp12, VSRp13]>, DwarfRegNum<[-1, -1]>;
  def UACC7 : UACC<7, "acc7", [VSRp14, VSRp15]>, DwarfRegNum<[-1, -1]>;
}
def UACCRC : RegisterClass<"PPC", [v512i1], 128,
                           (add UACC0, UACC1, UACC2, UACC3,
                                UACC4, UACC5, UACC6, UACC7)> {
  // The AllocationPriority for the UACC registers is still high and must be at
  // least 32 as we want to allocate these registers before we allocate other
  // global ranges. The value must be less than the AllocationPriority of the
  // ACC registers.
  let AllocationPriority = 4;
  let GlobalPriority = true;
  let Size = 512;
}

// FIXME: This allocation order may increase stack frame size when allocating
// non-volatile registers.
//
// Placing Altivec registers first and allocate the rest as underlying VSX
// ones, to reduce interference with accumulator registers (lower 32 VSRs).
// This reduces copies when loading for accumulators, which is common use for
// paired VSX registers.
def VSRpRC :
  RegisterClass<"PPC", [v256i1], 128,
                (add VSRp17, VSRp18, VSRp16, VSRp19, VSRp20, VSRp21,
                     VSRp22, VSRp23, VSRp24, VSRp25, VSRp31, VSRp30,
                     VSRp29, VSRp28, VSRp27, VSRp26,
                     (sequence "VSRp%u", 0, 6),
                     (sequence "VSRp%u", 15, 7))> {
  // Give the VSRp registers a non-zero AllocationPriority. The value is less
  // than 32 as these registers should not always be allocated before global
  // ranges and the value should be less than the AllocationPriority - 32 for
  // the UACC registers. Even global VSRp registers should be allocated after
  // the UACC registers have been chosen.
  let AllocationPriority = 2;
  let Size = 256;
}




