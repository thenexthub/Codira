//===-- XtensaMCAsmInfo.cpp - Xtensa Asm Properties -----------------------===//
//
//                     The LLVM Compiler Infrastructure
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
// This file contains the declarations of the XtensaMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "XtensaMCAsmInfo.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

XtensaMCAsmInfo::XtensaMCAsmInfo(const Triple &TT) {
  CodePointerSize = 4;
  CalleeSaveStackSlotSize = 4;
  PrivateGlobalPrefix = ".L";
  CommentString = "#";
  ZeroDirective = "\t.space\t";
  Data64bitsDirective = "\t.quad\t";
  GlobalDirective = "\t.global\t";
  UsesELFSectionDirectiveForBSS = true;
  SupportsDebugInformation = true;
  ExceptionsType = ExceptionHandling::DwarfCFI;
  AlignmentIsInBytes = false;
}

void XtensaMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                         const MCSpecifierExpr &Expr) const {
  StringRef S = Xtensa::getSpecifierName(Expr.getSpecifier());
  if (!S.empty())
    OS << '%' << S << '(';
  printExpr(OS, *Expr.getSubExpr());
  if (!S.empty())
    OS << ')';
}

uint8_t Xtensa::parseSpecifier(StringRef name) { return 0; }

StringRef Xtensa::getSpecifierName(uint8_t S) {
  switch (S) {
  default:
    llvm_unreachable("Invalid ELF symbol kind");
  }
}
