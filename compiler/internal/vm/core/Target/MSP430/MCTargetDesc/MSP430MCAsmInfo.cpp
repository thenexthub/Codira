//===-- MSP430MCAsmInfo.cpp - MSP430 asm properties -----------------------===//
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
// This file contains the declarations of the MSP430MCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "MSP430MCAsmInfo.h"
using namespace vm::core;

void MSP430MCAsmInfo::anchor() { }

MSP430MCAsmInfo::MSP430MCAsmInfo(const Triple &TT) {
  // Since MSP430-GCC already generates 32-bit DWARF information, we will
  // also store 16-bit pointers as 32-bit pointers in DWARF, because using
  // 32-bit DWARF pointers is already a working and tested path for LLDB
  // as well.
  CodePointerSize = 4;
  CalleeSaveStackSlotSize = 2;

  CommentString = ";";
  SeparatorString = "{";

  AlignmentIsInBytes = false;
  UsesELFSectionDirectiveForBSS = true;

  SupportsDebugInformation = true;

  ExceptionsType = ExceptionHandling::DwarfCFI;
}
