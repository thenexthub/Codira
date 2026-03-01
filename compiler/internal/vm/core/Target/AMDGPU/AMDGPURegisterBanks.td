//=- AMDGPURegisterBank.td - Describe the AMDGPU Banks -------*- tablegen -*-=//
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

def SGPRRegBank : RegisterBank<"SGPR",
  [SReg_LO16, SReg_32, SReg_64, SReg_96, SReg_128, SReg_160, SReg_192, SReg_224, SReg_256, SReg_288, SReg_320, SReg_352, SReg_384, SReg_512, SReg_1024]
>;

def VGPRRegBank : RegisterBank<"VGPR",
  [VGPR_16_Lo128, VGPR_16, VGPR_32, VReg_64, VReg_96, VReg_128, VReg_160, VReg_192, VReg_224, VReg_256, VReg_288, VReg_320, VReg_352, VReg_384, VReg_512, VReg_1024]
>;

// It is helpful to distinguish conditions from ordinary SGPRs.
def VCCRegBank : RegisterBank<"VCC", [SReg_32, SReg_64]>;

def AGPRRegBank : RegisterBank <"AGPR",
  [AGPR_LO16, AGPR_32, AReg_64, AReg_96, AReg_128, AReg_160, AReg_192, AReg_224, AReg_256, AReg_288, AReg_320, AReg_352, AReg_384, AReg_512, AReg_1024]
>;
