//===-- BPFRegisterBankInfo.h -----------------------------------*- C++ -*-===//
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
///
/// \file
/// This file declares the targeting of the RegisterBankInfo class for BPF.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_BPF_GISEL_BPFREGISTERBANKINFO_H
#define LLVM_LIB_TARGET_BPF_GISEL_BPFREGISTERBANKINFO_H

#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "vm/core/CodeGen/RegisterBankInfo.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"

#define GET_REGBANK_DECLARATIONS
#include "BPFGenRegisterBank.inc"

namespace vm::core {
class TargetRegisterInfo;

class BPFGenRegisterBankInfo : public RegisterBankInfo {
protected:
#define GET_TARGET_REGBANK_CLASS
#include "BPFGenRegisterBank.inc"
};

class BPFRegisterBankInfo final : public BPFGenRegisterBankInfo {
public:
  BPFRegisterBankInfo(const TargetRegisterInfo &TRI);
};
} // namespace vm::core

#endif
