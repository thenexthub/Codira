//===-- SparcSubtarget.cpp - SPARC Subtarget Information ------------------===//
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
// This file implements the SPARC specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "SparcSubtarget.h"
#include "SparcSelectionDAGInfo.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/MathExtras.h"

using namespace vm::core;

#define DEBUG_TYPE "sparc-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "SparcGenSubtargetInfo.inc"

void SparcSubtarget::anchor() { }

SparcSubtarget &SparcSubtarget::initializeSubtargetDependencies(
    StringRef CPU, StringRef TuneCPU, StringRef FS) {
  const Triple &TT = getTargetTriple();
  // Determine default and user specified characteristics
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = TT.isSPARC64() ? "v9" : "v8";

  if (TuneCPU.empty())
    TuneCPU = CPUName;

  // Parse features string.
  ParseSubtargetFeatures(CPUName, TuneCPU, FS);

  if (!Is64Bit && TT.isSPARC64()) {
    FeatureBitset Features = getFeatureBits();
    setFeatureBits(Features.set(Sparc::Feature64Bit));
    Is64Bit = true;
  }

  // Popc is a v9-only instruction.
  if (!IsV9)
    UsePopc = false;

  return *this;
}

SparcSubtarget::SparcSubtarget(const StringRef &CPU, const StringRef &TuneCPU,
                               const StringRef &FS, const TargetMachine &TM)
    : SparcGenSubtargetInfo(TM.getTargetTriple(), CPU, TuneCPU, FS),
      ReserveRegister(TM.getMCRegisterInfo()->getNumRegs()),
      InstrInfo(initializeSubtargetDependencies(CPU, TuneCPU, FS)),
      TLInfo(TM, *this), FrameLowering(*this) {
  TSInfo = std::make_unique<SparcSelectionDAGInfo>();
}

SparcSubtarget::~SparcSubtarget() = default;

const SelectionDAGTargetInfo *SparcSubtarget::getSelectionDAGInfo() const {
  return TSInfo.get();
}

void SparcSubtarget::initLibcallLoweringInfo(LibcallLoweringInfo &Info) const {
  if (hasHardQuad())
    return;

  // Setup Runtime library names.
  if (is64Bit() && !useSoftFloat()) {
    Info.setLibcallImpl(RTLIB::ADD_F128, RTLIB::impl__Qp_add);
    Info.setLibcallImpl(RTLIB::SUB_F128, RTLIB::impl__Qp_sub);
    Info.setLibcallImpl(RTLIB::MUL_F128, RTLIB::impl__Qp_mul);
    Info.setLibcallImpl(RTLIB::DIV_F128, RTLIB::impl__Qp_div);
    Info.setLibcallImpl(RTLIB::SQRT_F128, RTLIB::impl__Qp_sqrt);
    Info.setLibcallImpl(RTLIB::FPTOSINT_F128_I32, RTLIB::impl__Qp_qtoi);
    Info.setLibcallImpl(RTLIB::FPTOUINT_F128_I32, RTLIB::impl__Qp_qtoui);
    Info.setLibcallImpl(RTLIB::SINTTOFP_I32_F128, RTLIB::impl__Qp_itoq);
    Info.setLibcallImpl(RTLIB::UINTTOFP_I32_F128, RTLIB::impl__Qp_uitoq);
    Info.setLibcallImpl(RTLIB::FPTOSINT_F128_I64, RTLIB::impl__Qp_qtox);
    Info.setLibcallImpl(RTLIB::FPTOUINT_F128_I64, RTLIB::impl__Qp_qtoux);
    Info.setLibcallImpl(RTLIB::SINTTOFP_I64_F128, RTLIB::impl__Qp_xtoq);
    Info.setLibcallImpl(RTLIB::UINTTOFP_I64_F128, RTLIB::impl__Qp_uxtoq);
    Info.setLibcallImpl(RTLIB::FPEXT_F32_F128, RTLIB::impl__Qp_stoq);
    Info.setLibcallImpl(RTLIB::FPEXT_F64_F128, RTLIB::impl__Qp_dtoq);
    Info.setLibcallImpl(RTLIB::FPROUND_F128_F32, RTLIB::impl__Qp_qtos);
    Info.setLibcallImpl(RTLIB::FPROUND_F128_F64, RTLIB::impl__Qp_qtod);
  } else if (!useSoftFloat()) {
    Info.setLibcallImpl(RTLIB::ADD_F128, RTLIB::impl__Q_add);
    Info.setLibcallImpl(RTLIB::SUB_F128, RTLIB::impl__Q_sub);
    Info.setLibcallImpl(RTLIB::MUL_F128, RTLIB::impl__Q_mul);
    Info.setLibcallImpl(RTLIB::DIV_F128, RTLIB::impl__Q_div);
    Info.setLibcallImpl(RTLIB::SQRT_F128, RTLIB::impl__Q_sqrt);
    Info.setLibcallImpl(RTLIB::FPTOSINT_F128_I32, RTLIB::impl__Q_qtoi);
    Info.setLibcallImpl(RTLIB::FPTOUINT_F128_I32, RTLIB::impl__Q_qtou);
    Info.setLibcallImpl(RTLIB::SINTTOFP_I32_F128, RTLIB::impl__Q_itoq);
    Info.setLibcallImpl(RTLIB::UINTTOFP_I32_F128, RTLIB::impl__Q_utoq);
    Info.setLibcallImpl(RTLIB::FPEXT_F32_F128, RTLIB::impl__Q_stoq);
    Info.setLibcallImpl(RTLIB::FPEXT_F64_F128, RTLIB::impl__Q_dtoq);
    Info.setLibcallImpl(RTLIB::FPROUND_F128_F32, RTLIB::impl__Q_qtos);
    Info.setLibcallImpl(RTLIB::FPROUND_F128_F64, RTLIB::impl__Q_qtod);
  }
}

int SparcSubtarget::getAdjustedFrameSize(int frameSize) const {

  if (is64Bit()) {
    // All 64-bit stack frames must be 16-byte aligned, and must reserve space
    // for spilling the 16 window registers at %sp+BIAS..%sp+BIAS+128.
    frameSize += 128;
    // Frames with calls must also reserve space for 6 outgoing arguments
    // whether they are used or not. LowerCall_64 takes care of that.
    frameSize = alignTo(frameSize, 16);
  } else {
    // Emit the correct save instruction based on the number of bytes in
    // the frame. Minimum stack frame size according to V8 ABI is:
    //   16 words for register window spill
    //    1 word for address of returned aggregate-value
    // +  6 words for passing parameters on the stack
    // ----------
    //   23 words * 4 bytes per word = 92 bytes
    frameSize += 92;

    // Round up to next doubleword boundary -- a double-word boundary
    // is required by the ABI.
    frameSize = alignTo(frameSize, 8);
  }
  return frameSize;
}

bool SparcSubtarget::enableMachineScheduler() const {
  return true;
}
