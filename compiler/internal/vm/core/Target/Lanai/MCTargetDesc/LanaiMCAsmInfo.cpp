//===-- LanaiMCAsmInfo.cpp - Lanai asm properties -----------------------===//
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
// This file contains the declarations of the LanaiMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "LanaiMCAsmInfo.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

void LanaiMCAsmInfo::anchor() {}

LanaiMCAsmInfo::LanaiMCAsmInfo(const Triple & /*TheTriple*/,
                               const MCTargetOptions &Options) {
  IsLittleEndian = false;
  PrivateGlobalPrefix = ".L";
  WeakRefDirective = "\t.weak\t";
  ExceptionsType = ExceptionHandling::DwarfCFI;

  // Lanai assembly requires ".section" before ".bss"
  UsesELFSectionDirectiveForBSS = true;

  // Use '!' as comment string to correspond with old toolchain.
  CommentString = "!";

  // Target supports emission of debugging information.
  SupportsDebugInformation = true;

  // Set the instruction alignment. Currently used only for address adjustment
  // in dwarf generation.
  MinInstAlignment = 4;
}

void LanaiMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                        const MCSpecifierExpr &Expr) const {
  if (Expr.getSpecifier() == 0) {
    printExpr(OS, *Expr.getSubExpr());
    return;
  }

  switch (Expr.getSpecifier()) {
  default:
    llvm_unreachable("Invalid kind!");
  case Lanai::S_ABS_HI:
    OS << "hi";
    break;
  case Lanai::S_ABS_LO:
    OS << "lo";
    break;
  }

  OS << '(';
  printExpr(OS, *Expr.getSubExpr());
  OS << ')';
}
