//===-- BPFSubtarget.cpp - BPF Subtarget Information ----------------------===//
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
// This file implements the BPF specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "BPFSubtarget.h"
#include "BPF.h"
#include "BPFTargetMachine.h"
#include "GISel/BPFCallLowering.h"
#include "GISel/BPFLegalizerInfo.h"
#include "GISel/BPFRegisterBankInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/TargetParser/Host.h"

using namespace vm::core;

#define DEBUG_TYPE "bpf-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "BPFGenSubtargetInfo.inc"

static cl::opt<bool> Disable_ldsx("disable-ldsx", cl::Hidden, cl::init(false),
  cl::desc("Disable ldsx insns"));
static cl::opt<bool> Disable_movsx("disable-movsx", cl::Hidden, cl::init(false),
  cl::desc("Disable movsx insns"));
static cl::opt<bool> Disable_bswap("disable-bswap", cl::Hidden, cl::init(false),
  cl::desc("Disable bswap insns"));
static cl::opt<bool> Disable_sdiv_smod("disable-sdiv-smod", cl::Hidden,
  cl::init(false), cl::desc("Disable sdiv/smod insns"));
static cl::opt<bool> Disable_gotol("disable-gotol", cl::Hidden, cl::init(false),
  cl::desc("Disable gotol insn"));
static cl::opt<bool>
    Disable_StoreImm("disable-storeimm", cl::Hidden, cl::init(false),
                     cl::desc("Disable BPF_ST (immediate store) insn"));
static cl::opt<bool> Disable_load_acq_store_rel(
    "disable-load-acq-store-rel", cl::Hidden, cl::init(false),
    cl::desc("Disable load-acquire and store-release insns"));
static cl::opt<bool> Disable_gotox("disable-gotox", cl::Hidden, cl::init(false),
                                   cl::desc("Disable gotox insn"));

void BPFSubtarget::anchor() {}

BPFSubtarget &BPFSubtarget::initializeSubtargetDependencies(StringRef CPU,
                                                            StringRef FS) {
  initializeEnvironment();
  initSubtargetFeatures(CPU, FS);
  ParseSubtargetFeatures(CPU, /*TuneCPU*/ CPU, FS);
  return *this;
}

void BPFSubtarget::initializeEnvironment() {
  HasJmpExt = false;
  HasJmp32 = false;
  HasAlu32 = false;
  UseDwarfRIS = false;
  HasLdsx = false;
  HasMovsx = false;
  HasBswap = false;
  HasSdivSmod = false;
  HasGotol = false;
  HasStoreImm = false;
  HasLoadAcqStoreRel = false;
  HasGotox = false;
  AllowsMisalignedMemAccess = false;
}

void BPFSubtarget::initSubtargetFeatures(StringRef CPU, StringRef FS) {
  if (CPU.empty())
    CPU = "v3";
  if (CPU == "probe")
    CPU = sys::detail::getHostCPUNameForBPF();
  if (CPU == "generic" || CPU == "v1")
    return;
  if (CPU == "v2") {
    HasJmpExt = true;
    return;
  }
  if (CPU == "v3") {
    HasJmpExt = true;
    HasJmp32 = true;
    HasAlu32 = true;
    return;
  }
  if (CPU == "v4") {
    HasJmpExt = true;
    HasJmp32 = true;
    HasAlu32 = true;
    HasLdsx = !Disable_ldsx;
    HasMovsx = !Disable_movsx;
    HasBswap = !Disable_bswap;
    HasSdivSmod = !Disable_sdiv_smod;
    HasGotol = !Disable_gotol;
    HasStoreImm = !Disable_StoreImm;
    HasLoadAcqStoreRel = !Disable_load_acq_store_rel;
    HasGotox = !Disable_gotox;
    return;
  }
}

BPFSubtarget::BPFSubtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const TargetMachine &TM)
    : BPFGenSubtargetInfo(TT, CPU, /*TuneCPU*/ CPU, FS),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), FrameLowering(*this),
      TLInfo(TM, *this) {
  IsLittleEndian = TT.isLittleEndian();

  CallLoweringInfo.reset(new BPFCallLowering(*getTargetLowering()));
  Legalizer.reset(new BPFLegalizerInfo(*this));
  auto *RBI = new BPFRegisterBankInfo(*getRegisterInfo());
  RegBankInfo.reset(RBI);

  InstSelector.reset(createBPFInstructionSelector(
      *static_cast<const BPFTargetMachine *>(&TM), *this, *RBI));
}

const CallLowering *BPFSubtarget::getCallLowering() const {
  return CallLoweringInfo.get();
}

InstructionSelector *BPFSubtarget::getInstructionSelector() const {
  return InstSelector.get();
}

const LegalizerInfo *BPFSubtarget::getLegalizerInfo() const {
  return Legalizer.get();
}

const RegisterBankInfo *BPFSubtarget::getRegBankInfo() const {
  return RegBankInfo.get();
}
