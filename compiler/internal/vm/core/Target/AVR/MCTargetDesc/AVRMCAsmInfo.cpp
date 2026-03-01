//===-- AVRMCAsmInfo.cpp - AVR asm properties -----------------------------===//
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
// This file contains the declarations of the AVRMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "AVRMCAsmInfo.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

AVRMCAsmInfo::AVRMCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {
  CodePointerSize = 2;
  CalleeSaveStackSlotSize = 2;
  CommentString = ";";
  SeparatorString = "$";
  UsesELFSectionDirectiveForBSS = true;
  SupportsDebugInformation = true;
}

namespace {
const struct ModifierEntry {
  const char *const Spelling;
  AVRMCExpr::Specifier specifier;
} ModifierNames[] = {
    {"lo8", AVR::S_LO8},       {"hi8", AVR::S_HI8},
    {"hh8", AVR::S_HH8}, // synonym with hlo8
    {"hlo8", AVR::S_HH8},      {"hhi8", AVR::S_HHI8},

    {"pm", AVR::S_PM},         {"pm_lo8", AVR::S_PM_LO8},
    {"pm_hi8", AVR::S_PM_HI8}, {"pm_hh8", AVR::S_PM_HH8},

    {"lo8_gs", AVR::S_LO8_GS}, {"hi8_gs", AVR::S_HI8_GS},
    {"gs", AVR::S_GS},
};

} // end of anonymous namespace

AVRMCExpr::Specifier AVRMCExpr::parseSpecifier(StringRef Name) {
  const auto &Modifier =
      toolchain::find_if(ModifierNames, [&Name](ModifierEntry const &Mod) {
        return Mod.Spelling == Name;
      });

  if (Modifier != std::end(ModifierNames)) {
    return Modifier->specifier;
  }
  return AVR::S_AVR_NONE;
}

const char *AVRMCExpr::getName() const {
  const auto &Modifier =
      toolchain::find_if(ModifierNames, [this](ModifierEntry const &Mod) {
        return Mod.specifier == getSpecifier();
      });

  if (Modifier != std::end(ModifierNames)) {
    return Modifier->Spelling;
  }
  return nullptr;
}

AVR::Fixups AVRMCExpr::getFixupKind() const {
  AVR::Fixups Kind = AVR::Fixups::LastTargetFixupKind;

  switch (getSpecifier()) {
  case AVR::S_LO8:
    Kind = isNegated() ? AVR::fixup_lo8_ldi_neg : AVR::fixup_lo8_ldi;
    break;
  case AVR::S_HI8:
    Kind = isNegated() ? AVR::fixup_hi8_ldi_neg : AVR::fixup_hi8_ldi;
    break;
  case AVR::S_HH8:
    Kind = isNegated() ? AVR::fixup_hh8_ldi_neg : AVR::fixup_hh8_ldi;
    break;
  case AVR::S_HHI8:
    Kind = isNegated() ? AVR::fixup_ms8_ldi_neg : AVR::fixup_ms8_ldi;
    break;

  case AVR::S_PM_LO8:
    Kind = isNegated() ? AVR::fixup_lo8_ldi_pm_neg : AVR::fixup_lo8_ldi_pm;
    break;
  case AVR::S_PM_HI8:
    Kind = isNegated() ? AVR::fixup_hi8_ldi_pm_neg : AVR::fixup_hi8_ldi_pm;
    break;
  case AVR::S_PM_HH8:
    Kind = isNegated() ? AVR::fixup_hh8_ldi_pm_neg : AVR::fixup_hh8_ldi_pm;
    break;
  case AVR::S_PM:
  case AVR::S_GS:
    Kind = AVR::fixup_16_pm;
    break;
  case AVR::S_LO8_GS:
    Kind = AVR::fixup_lo8_ldi_gs;
    break;
  case AVR::S_HI8_GS:
    Kind = AVR::fixup_hi8_ldi_gs;
    break;

  default:
    llvm_unreachable("Uninitialized expression");
  }

  return Kind;
}

void AVRMCAsmInfo::printSpecifierExpr(raw_ostream &OS,
                                      const MCSpecifierExpr &Expr) const {
  auto &E = static_cast<const AVRMCExpr &>(Expr);
  assert(E.getSpecifier() != AVR::S_AVR_NONE);
  OS << E.getName() << '(';
  if (E.isNegated())
    OS << '-' << '(';
  printExpr(OS, *E.getSubExpr());
  if (E.isNegated())
    OS << ')';
  OS << ')';
}

int64_t AVRMCExpr::evaluateAsInt64(int64_t Value) const {
  if (Negated)
    Value *= -1;

  switch (getSpecifier()) {
  case AVR::S_LO8:
    Value &= 0xff;
    break;
  case AVR::S_HI8:
    Value &= 0xff00;
    Value >>= 8;
    break;
  case AVR::S_HH8:
    Value &= 0xff0000;
    Value >>= 16;
    break;
  case AVR::S_HHI8:
    Value &= 0xff000000;
    Value >>= 24;
    break;
  case AVR::S_PM_LO8:
  case AVR::S_LO8_GS:
    Value >>= 1; // Program memory addresses must always be shifted by one.
    Value &= 0xff;
    break;
  case AVR::S_PM_HI8:
  case AVR::S_HI8_GS:
    Value >>= 1; // Program memory addresses must always be shifted by one.
    Value &= 0xff00;
    Value >>= 8;
    break;
  case AVR::S_PM_HH8:
    Value >>= 1; // Program memory addresses must always be shifted by one.
    Value &= 0xff0000;
    Value >>= 16;
    break;
  case AVR::S_PM:
  case AVR::S_GS:
    Value >>= 1; // Program memory addresses must always be shifted by one.
    break;

  case AVR::S_AVR_NONE:
  default:
    llvm_unreachable("Uninitialized expression.");
  }
  return static_cast<uint64_t>(Value) & 0xff;
}

// bool AVRMCExpr::evaluateAsRelocatableImpl(MCValue &Result,
//                                           const MCAssembler *Asm) const {
bool AVRMCAsmInfo::evaluateAsRelocatableImpl(const MCSpecifierExpr &Expr,
                                             MCValue &Result,
                                             const MCAssembler *Asm) const {
  auto &E = static_cast<const AVRMCExpr &>(Expr);
  MCValue Value;
  bool isRelocatable = E.getSubExpr()->evaluateAsRelocatable(Value, Asm);
  if (!isRelocatable)
    return false;

  if (Value.isAbsolute()) {
    Result = MCValue::get(E.evaluateAsInt64(Value.getConstant()));
  } else {
    if (!Asm || !Asm->hasLayout())
      return false;

    auto Spec = AVR::S_None;
    if (Value.getSpecifier())
      return false;
    assert(!Value.getSubSym());
    if (E.getSpecifier() == AVR::S_PM)
      Spec = AVR::S_PM;

    // TODO: don't attach specifier to MCSymbolRefExpr.
    Result =
        MCValue::get(Value.getAddSym(), nullptr, Value.getConstant(), Spec);
  }

  return true;
}

bool AVRMCExpr::evaluateAsConstant(int64_t &Result) const {
  MCValue Value;
  bool isRelocatable = getSubExpr()->evaluateAsRelocatable(Value, nullptr);
  if (!isRelocatable)
    return false;

  if (Value.isAbsolute()) {
    Result = evaluateAsInt64(Value.getConstant());
    return true;
  }

  return false;
}
