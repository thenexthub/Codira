//===- ARMMachORelocationInfo.cpp -----------------------------------------===//
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

#include "ARMMCAsmInfo.h"
#include "MCTargetDesc/ARMMCTargetDesc.h"
#include "vm/core-c/Disassembler.h"
#include "vm/core/MC/MCDisassembler/MCRelocationInfo.h"
#include "vm/core/MC/MCExpr.h"

using namespace vm::core;

namespace {

class ARMMachORelocationInfo : public MCRelocationInfo {
public:
  ARMMachORelocationInfo(MCContext &Ctx) : MCRelocationInfo(Ctx) {}

  const MCExpr *createExprForCAPIVariantKind(const MCExpr *SubExpr,
                                             unsigned VariantKind) override {
    switch(VariantKind) {
    case LLVMDisassembler_VariantKind_ARM_HI16:
      return ARM::createUpper16(SubExpr, Ctx);
    case LLVMDisassembler_VariantKind_ARM_LO16:
      return ARM::createLower16(SubExpr, Ctx);
    default:
      return MCRelocationInfo::createExprForCAPIVariantKind(SubExpr,
                                                            VariantKind);
    }
  }
};

} // end anonymous namespace

/// createARMMachORelocationInfo - Construct an ARM Mach-O RelocationInfo.
MCRelocationInfo *toolchain::createARMMachORelocationInfo(MCContext &Ctx) {
  return new ARMMachORelocationInfo(Ctx);
}
