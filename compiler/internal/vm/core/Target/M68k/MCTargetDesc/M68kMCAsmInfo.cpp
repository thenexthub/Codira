//===-- M68kMCAsmInfo.cpp - M68k Asm Properties -----------------*- C++ -*-===//
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
/// This file contains the definitions of the M68k MCAsmInfo properties.
///
//===----------------------------------------------------------------------===//

#include "M68kMCAsmInfo.h"

#include "vm/core/MC/MCExpr.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

const MCAsmInfo::AtSpecifier atSpecifiers[] = {
    {M68k::S_GOTOFF, "GOTOFF"},     {M68k::S_GOTPCREL, "GOTPCREL"},
    {M68k::S_GOTTPOFF, "GOTTPOFF"}, {M68k::S_PLT, "PLT"},
    {M68k::S_TLSGD, "TLSGD"},       {M68k::S_TLSLD, "TLSLD"},
    {M68k::S_TLSLDM, "TLSLDM"},     {M68k::S_TPOFF, "TPOFF"},
};

void M68kELFMCAsmInfo::anchor() {}

M68kELFMCAsmInfo::M68kELFMCAsmInfo(const Triple &T) {
  CodePointerSize = 4;
  CalleeSaveStackSlotSize = 4;

  IsLittleEndian = false;

  // Debug Information
  SupportsDebugInformation = true;

  // Exceptions handling
  ExceptionsType = ExceptionHandling::DwarfCFI;

  UseMotorolaIntegers = true;
  CommentString = ";";

  initializeAtSpecifiers(atSpecifiers);
}
