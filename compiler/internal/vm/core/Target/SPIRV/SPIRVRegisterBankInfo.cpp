//===- SPIRVRegisterBankInfo.cpp ------------------------------*- C++ -*---===//
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
// This file implements the targeting of the RegisterBankInfo class for SPIR-V.
//
//===----------------------------------------------------------------------===//

#include "SPIRVRegisterBankInfo.h"
#include "SPIRVRegisterInfo.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/CodeGen/RegisterBank.h"

#define GET_REGINFO_ENUM
#include "SPIRVGenRegisterInfo.inc"

#define GET_TARGET_REGBANK_IMPL
#include "SPIRVGenRegisterBank.inc"

using namespace vm::core;

// This required for .td selection patterns to work or we'd end up with RegClass
// checks being redundant as all the classes would be mapped to the same bank.
const RegisterBank &
SPIRVRegisterBankInfo::getRegBankFromRegClass(const TargetRegisterClass &RC,
                                              LLT Ty) const {
  if (RC.getID() == SPIRV::TYPERegClassID)
    return SPIRV::TYPERegBank;
  return SPIRV::IDRegBank;
}
