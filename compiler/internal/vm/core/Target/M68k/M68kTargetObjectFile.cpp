//===-- M68kELFTargetObjectFile.cpp - M68k Object Files ---------*- C++ -*-===//
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
/// This file contains definitions for M68k ELF object file lowering.
///
//===----------------------------------------------------------------------===//

#include "M68kTargetObjectFile.h"

#include "M68kSubtarget.h"
#include "M68kTargetMachine.h"

#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCSectionELF.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;

static cl::opt<unsigned> SSThreshold(
    "m68k-ssection-threshold", cl::Hidden,
    cl::desc("Small data and bss section threshold size (default=8)"),
    cl::init(8));

void M68kELFTargetObjectFile::Initialize(MCContext &Ctx,
                                         const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);
  InitializeELF(TM.Options.UseInitArray);

  this->TM = &static_cast<const M68kTargetMachine &>(TM);

  // FIXME do we need `.sdata` and `.sbss` explicitly?
  SmallDataSection = getContext().getELFSection(
      ".sdata", ELF::SHT_PROGBITS, ELF::SHF_WRITE | ELF::SHF_ALLOC);

  SmallBSSSection = getContext().getELFSection(".sbss", ELF::SHT_NOBITS,
                                               ELF::SHF_WRITE | ELF::SHF_ALLOC);
}
