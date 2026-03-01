//===-- PPCTargetObjectFile.cpp - PPC Object Info -------------------------===//
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

#include "PPCTargetObjectFile.h"
#include "MCTargetDesc/PPCMCAsmInfo.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"

using namespace vm::core;

void
PPC64LinuxTargetObjectFile::
Initialize(MCContext &Ctx, const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
}

MCSection *PPC64LinuxTargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {
  // Here override ReadOnlySection to DataRelROSection for PPC64 SVR4 ABI
  // when we have a constant that contains global relocations.  This is
  // necessary because of this ABI's handling of pointers to functions in
  // a shared library.  The address of a function is actually the address
  // of a function descriptor, which resides in the .opd section.  Generated
  // code uses the descriptor directly rather than going via the GOT as some
  // other ABIs do, which means that initialized function pointers must
  // reference the descriptor.  The linker must convert copy relocs of
  // pointers to functions in shared libraries into dynamic relocations,
  // because of an ordering problem with initialization of copy relocs and
  // PLT entries.  The dynamic relocation will be initialized by the dynamic
  // linker, so we must use DataRelROSection instead of ReadOnlySection.
  // For more information, see the description of ELIMINATE_COPY_RELOCS in
  // GNU ld.
  if (Kind.isReadOnly()) {
    const auto *GVar = dyn_cast<GlobalVariable>(GO);

    if (GVar && GVar->isConstant() &&
        GVar->getInitializer()->needsDynamicRelocation())
      Kind = SectionKind::getReadOnlyWithRel();
  }

  return TargetLoweringObjectFileELF::SelectSectionForGlobal(GO, Kind, TM);
}

const MCExpr *PPC64LinuxTargetObjectFile::
getDebugThreadLocalSymbol(const MCSymbol *Sym) const {
  const MCExpr *Expr =
      MCSymbolRefExpr::create(Sym, PPC::S_DTPREL, getContext());
  return MCBinaryExpr::createAdd(Expr,
                                 MCConstantExpr::create(0x8000, getContext()),
                                 getContext());
}

