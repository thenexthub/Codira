//===-- toolchain/Target/ARMTargetObjectFile.cpp - ARM Object Info Impl --------===//
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

#include "ARMTargetObjectFile.h"
#include "ARMSubtarget.h"
#include "ARMTargetMachine.h"
#include "MCTargetDesc/ARMMCAsmInfo.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCSectionELF.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/MC/SectionKind.h"
#include "vm/core/Target/TargetMachine.h"
#include <cassert>

using namespace vm::core;
using namespace dwarf;

//===----------------------------------------------------------------------===//
//                               ELF Target
//===----------------------------------------------------------------------===//

ARMElfTargetObjectFile::ARMElfTargetObjectFile() {
  PLTRelativeSpecifier = ARM::S_PREL31;
  SupportIndirectSymViaGOTPCRel = true;
}

void ARMElfTargetObjectFile::Initialize(MCContext &Ctx,
                                        const TargetMachine &TM) {
  const ARMBaseTargetMachine &ARM_TM = static_cast<const ARMBaseTargetMachine &>(TM);
  bool isAAPCS_ABI = ARM_TM.TargetABI == ARM::ARMABI::ARM_ABI_AAPCS;
  bool genExecuteOnly =
      ARM_TM.getMCSubtargetInfo()->hasFeature(ARM::FeatureExecuteOnly);

  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  InitializeELF(isAAPCS_ABI);

  if (isAAPCS_ABI) {
    LSDASection = nullptr;
  }

  // Make code section unreadable when in execute-only mode
  if (genExecuteOnly) {
    unsigned Type = ELF::SHT_PROGBITS;
    unsigned Flags =
        ELF::SHF_EXECINSTR | ELF::SHF_ALLOC | ELF::SHF_ARM_PURECODE;
    // Since we cannot modify flags for an existing section, we create a new
    // section with the right flags, and use 0 as the unique ID for
    // execute-only text
    TextSection =
        Ctx.getELFSection(".text", Type, Flags, 0, "", false, 0U, nullptr);
  }
}

MCRegister ARMElfTargetObjectFile::getStaticBase() const { return ARM::R9; }

const MCExpr *ARMElfTargetObjectFile::getIndirectSymViaGOTPCRel(
    const GlobalValue *GV, const MCSymbol *Sym, const MCValue &MV,
    int64_t Offset, MachineModuleInfo *MMI, MCStreamer &Streamer) const {
  int64_t FinalOffset = Offset + MV.getConstant();
  const MCExpr *Res =
      MCSymbolRefExpr::create(Sym, ARM::S_GOT_PREL, getContext());
  const MCExpr *Off = MCConstantExpr::create(FinalOffset, getContext());
  return MCBinaryExpr::createAdd(Res, Off, getContext());
}

const MCExpr *ARMElfTargetObjectFile::
getIndirectSymViaRWPI(const MCSymbol *Sym) const {
  return MCSymbolRefExpr::create(Sym, ARM::S_SBREL, getContext());
}

const MCExpr *ARMElfTargetObjectFile::getTTypeGlobalReference(
    const GlobalValue *GV, unsigned Encoding, const TargetMachine &TM,
    MachineModuleInfo *MMI, MCStreamer &Streamer) const {
  if (TM.getMCAsmInfo()->getExceptionHandlingType() != ExceptionHandling::ARM)
    return TargetLoweringObjectFileELF::getTTypeGlobalReference(
        GV, Encoding, TM, MMI, Streamer);

  assert(Encoding == DW_EH_PE_absptr && "Can handle absptr encoding only");

  return MCSymbolRefExpr::create(TM.getSymbol(GV), ARM::S_TARGET2,
                                 getContext());
}

const MCExpr *ARMElfTargetObjectFile::
getDebugThreadLocalSymbol(const MCSymbol *Sym) const {
  return MCSymbolRefExpr::create(Sym, ARM::S_TLSLDO, getContext());
}

static bool isExecuteOnlyFunction(const GlobalObject *GO, SectionKind SK,
                                  const TargetMachine &TM) {
  if (const Function *F = dyn_cast<Function>(GO))
    if (TM.getSubtarget<ARMSubtarget>(*F).genExecuteOnly() && SK.isText())
      return true;
  return false;
}

MCSection *ARMElfTargetObjectFile::getExplicitSectionGlobal(
    const GlobalObject *GO, SectionKind SK, const TargetMachine &TM) const {
  // Set execute-only access for the explicit section
  if (isExecuteOnlyFunction(GO, SK, TM))
    SK = SectionKind::getExecuteOnly();

  return TargetLoweringObjectFileELF::getExplicitSectionGlobal(GO, SK, TM);
}

MCSection *ARMElfTargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind SK, const TargetMachine &TM) const {
  // Place the global in the execute-only text section
  if (isExecuteOnlyFunction(GO, SK, TM))
    SK = SectionKind::getExecuteOnly();

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, SK, TM);
}
