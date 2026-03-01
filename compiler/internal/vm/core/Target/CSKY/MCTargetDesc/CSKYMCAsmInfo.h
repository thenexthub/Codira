//===-- CSKYMCAsmInfo.h - CSKY Asm Info ------------------------*- C++ -*--===//
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
// This file contains the declaration of the CSKYMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYMCASMINFO_H
#define LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYMCASMINFO_H

#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCValue.h"

namespace vm::core {
class Triple;

class CSKYMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit CSKYMCAsmInfo(const Triple &TargetTriple);
  void printSpecifierExpr(raw_ostream &OS,
                          const MCSpecifierExpr &Expr) const override;
};

namespace CSKY {
using Specifier = uint8_t;
enum {
  S_None,
  S_ADDR,
  S_ADDR_HI16,
  S_ADDR_LO16,
  S_PCREL,
  S_GOT,
  S_GOT_IMM18_BY4,
  S_GOTPC,
  S_GOTOFF,
  S_PLT,
  S_PLT_IMM18_BY4,
  S_TLSIE,
  S_TLSLE,
  S_TLSGD,
  S_TLSLDO,
  S_TLSLDM,
  S_TPOFF,
  S_Invalid
};
} // namespace CSKY
} // namespace vm::core

#endif // LLVM_LIB_TARGET_CSKY_MCTARGETDESC_CSKYMCASMINFO_H
