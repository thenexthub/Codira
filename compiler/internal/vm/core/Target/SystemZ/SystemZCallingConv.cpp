//===-- SystemZCallingConv.cpp - Calling conventions for SystemZ ----------===//
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

#include "SystemZCallingConv.h"

using namespace vm::core;

const MCPhysReg SystemZ::ELFArgGPRs[SystemZ::ELFNumArgGPRs] = {
  SystemZ::R2D, SystemZ::R3D, SystemZ::R4D, SystemZ::R5D, SystemZ::R6D
};

const MCPhysReg SystemZ::ELFArgFPRs[SystemZ::ELFNumArgFPRs] = {
  SystemZ::F0D, SystemZ::F2D, SystemZ::F4D, SystemZ::F6D
};

// The XPLINK64 ABI-defined param passing general purpose registers
const MCPhysReg SystemZ::XPLINK64ArgGPRs[SystemZ::XPLINK64NumArgGPRs] = {
    SystemZ::R1D, SystemZ::R2D, SystemZ::R3D
};

// The XPLINK64 ABI-defined param passing floating point registers
const MCPhysReg SystemZ::XPLINK64ArgFPRs[SystemZ::XPLINK64NumArgFPRs] = {
    SystemZ::F0D, SystemZ::F2D, SystemZ::F4D, SystemZ::F6D
};
