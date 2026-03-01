//===- ARC.td - Describe the ARC Target Machine ------------*- tablegen -*-===//
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

include "toolchain/Target/Target.td"

//===----------------------------------------------------------------------===//
// ARC Subtarget features
//===----------------------------------------------------------------------===//

def FeatureNORM
    : SubtargetFeature<"norm", "Xnorm", "true",
                       "Enable support for norm instruction.">;

//===----------------------------------------------------------------------===//
// Registers, calling conventions, instruction descriptions
//===----------------------------------------------------------------------===//

include "ARCRegisterInfo.td"
include "ARCInstrInfo.td"
include "ARCCallingConv.td"

defm : RemapAllTargetPseudoPointerOperands<GPR32>;

def ARCInstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def ARC : Target {
  let InstructionSet = ARCInstrInfo;
}
