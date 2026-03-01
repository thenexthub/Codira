//===- MCAsmInfoDarwin.cpp - Darwin asm properties ------------------------===//
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
// This file defines target asm properties related what form asm statements
// should take in general on Darwin-based targets
//
//===----------------------------------------------------------------------===//

#include "vm/core/MC/MCAsmInfoDarwin.h"
#include "vm/core/BinaryFormat/MachO.h"
#include "vm/core/MC/MCDirectives.h"
#include "vm/core/MC/MCSectionMachO.h"

using namespace vm::core;

bool MCAsmInfoDarwin::isSectionAtomizableBySymbols(
    const MCSection &Section) {
  const MCSectionMachO &SMO = static_cast<const MCSectionMachO &>(Section);

  // Sections holding 1 byte strings are atomized based on the data they
  // contain.
  // Sections holding 2 byte strings require symbols in order to be atomized.
  // There is no dedicated section for 4 byte strings.
  if (SMO.getType() == MachO::S_CSTRING_LITERALS)
    return false;

  if (SMO.getSegmentName() == "__DATA" && SMO.getName() == "__cfstring")
    return false;

  if (SMO.getSegmentName() == "__DATA" && SMO.getName() == "__objc_classrefs")
    return false;

  switch (SMO.getType()) {
  default:
    return true;

  // These sections are atomized at the element boundaries without using
  // symbols.
  case MachO::S_4BYTE_LITERALS:
  case MachO::S_8BYTE_LITERALS:
  case MachO::S_16BYTE_LITERALS:
  case MachO::S_LITERAL_POINTERS:
  case MachO::S_NON_LAZY_SYMBOL_POINTERS:
  case MachO::S_LAZY_SYMBOL_POINTERS:
  case MachO::S_THREAD_LOCAL_VARIABLE_POINTERS:
  case MachO::S_MOD_INIT_FUNC_POINTERS:
  case MachO::S_MOD_TERM_FUNC_POINTERS:
  case MachO::S_INTERPOSING:
    return false;
  }
}

MCAsmInfoDarwin::MCAsmInfoDarwin() {
  // Common settings for all Darwin targets.
  // Syntax:
  LinkerPrivateGlobalPrefix = "l";
  HasSingleParameterDotFile = false;
  HasSubsectionsViaSymbols = true;

  AlignmentIsInBytes = false;
  COMMDirectiveAlignmentIsInBytes = false;
  LCOMMDirectiveAlignmentType = LCOMM::Log2Alignment;
  InlineAsmStart = " InlineAsm Start";
  InlineAsmEnd = " InlineAsm End";

  // Directives:
  HasWeakDefCanBeHiddenDirective = true;
  WeakRefDirective = "\t.weak_reference ";
  ZeroDirective = "\t.space\t";  // ".space N" emits N zeros.

  HiddenVisibilityAttr = MCSA_PrivateExtern;
  HiddenDeclarationVisibilityAttr = MCSA_Invalid;

  // Doesn't support protected visibility.
  ProtectedVisibilityAttr = MCSA_Invalid;

  HasDotTypeDotSizeDirective = false;
  HasNoDeadStrip = true;

  DwarfUsesRelocationsAcrossSections = false;
  SetDirectiveSuppressesReloc = true;
}

bool MCAsmInfoDarwin::useCodeAlign(const MCSection &Sec) const {
  return static_cast<const MCSectionMachO &>(Sec).hasAttribute(
      MachO::S_ATTR_PURE_INSTRUCTIONS);
}
