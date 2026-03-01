//===-- NVPTXMCExpr.cpp - NVPTX specific MC expression classes ------------===//
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

#include "NVPTXMCExpr.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/Support/Format.h"
using namespace vm::core;

#define DEBUG_TYPE "nvptx-mcexpr"

const NVPTXFloatMCExpr *
NVPTXFloatMCExpr::create(VariantKind Kind, const APFloat &Flt, MCContext &Ctx) {
  return new (Ctx) NVPTXFloatMCExpr(Kind, Flt);
}

void NVPTXFloatMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  bool Ignored;
  unsigned NumHex;
  APFloat APF = getAPFloat();

  switch (Kind) {
  default: llvm_unreachable("Invalid kind!");
  case VK_NVPTX_HALF_PREC_FLOAT:
    // ptxas does not have a way to specify half-precision floats.
    // Instead we have to print and load fp16 constants as .b16
    OS << "0x";
    NumHex = 4;
    APF.convert(APFloat::IEEEhalf(), APFloat::rmNearestTiesToEven, &Ignored);
    break;
  case VK_NVPTX_BFLOAT_PREC_FLOAT:
    OS << "0x";
    NumHex = 4;
    APF.convert(APFloat::BFloat(), APFloat::rmNearestTiesToEven, &Ignored);
    break;
  case VK_NVPTX_SINGLE_PREC_FLOAT:
    OS << "0f";
    NumHex = 8;
    APF.convert(APFloat::IEEEsingle(), APFloat::rmNearestTiesToEven, &Ignored);
    break;
  case VK_NVPTX_DOUBLE_PREC_FLOAT:
    OS << "0d";
    NumHex = 16;
    APF.convert(APFloat::IEEEdouble(), APFloat::rmNearestTiesToEven, &Ignored);
    break;
  }

  APInt API = APF.bitcastToAPInt();
  OS << format_hex_no_prefix(API.getZExtValue(), NumHex, /*Upper=*/true);
}

const NVPTXGenericMCSymbolRefExpr*
NVPTXGenericMCSymbolRefExpr::create(const MCSymbolRefExpr *SymExpr,
                                    MCContext &Ctx) {
  return new (Ctx) NVPTXGenericMCSymbolRefExpr(SymExpr);
}

void NVPTXGenericMCSymbolRefExpr::printImpl(raw_ostream &OS,
                                            const MCAsmInfo *MAI) const {
  OS << "generic(";
  MAI->printExpr(OS, *SymExpr);
  OS << ")";
}
