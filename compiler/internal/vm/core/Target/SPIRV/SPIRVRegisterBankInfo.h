//===- SPIRVRegisterBankInfo.h -----------------------------------*- C++ -*-==//
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
// This file declares the targeting of the RegisterBankInfo class for SPIR-V.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVREGISTERBANKINFO_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVREGISTERBANKINFO_H

#include "vm/core/CodeGen/RegisterBankInfo.h"

#define GET_REGBANK_DECLARATIONS
#include "SPIRVGenRegisterBank.inc"

namespace vm::core {

class TargetRegisterInfo;

class SPIRVGenRegisterBankInfo : public RegisterBankInfo {
protected:
#define GET_TARGET_REGBANK_CLASS
#include "SPIRVGenRegisterBank.inc"
};

// This class provides the information for the target register banks.
class SPIRVRegisterBankInfo final : public SPIRVGenRegisterBankInfo {
public:
  const RegisterBank &getRegBankFromRegClass(const TargetRegisterClass &RC,
                                             LLT Ty) const override;
};
} // namespace vm::core
#endif // LLVM_LIB_TARGET_SPIRV_SPIRVREGISTERBANKINFO_H
