//===-- RISCVTargetParser.cpp - Parser for target features ------*- C++ -*-===//
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
// This file implements a target parser to recognise hardware features
// for RISC-V CPUs.
//
//===----------------------------------------------------------------------===//

#include "vm/core/TargetParser/RISCVTargetParser.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/TargetParser/RISCVISAInfo.h"

namespace vm::core {
namespace RISCV {

enum CPUKind : unsigned {
#define PROC(ENUM, NAME, DEFAULT_MARCH, FAST_SCALAR_UNALIGN,                   \
             FAST_VECTOR_UNALIGN, MVENDORID, MARCHID, MIMPID)                  \
  CK_##ENUM,
#define TUNE_PROC(ENUM, NAME) CK_##ENUM,
#include "vm/core/TargetParser/RISCVTargetParserDef.inc"
};

constexpr CPUInfo RISCVCPUInfo[] = {
#define PROC(ENUM, NAME, DEFAULT_MARCH, FAST_SCALAR_UNALIGN,                   \
             FAST_VECTOR_UNALIGN, MVENDORID, MARCHID, MIMPID)                  \
  {                                                                            \
      NAME,                                                                    \
      DEFAULT_MARCH,                                                           \
      FAST_SCALAR_UNALIGN,                                                     \
      FAST_VECTOR_UNALIGN,                                                     \
      {MVENDORID, MARCHID, MIMPID},                                            \
  },
#include "vm/core/TargetParser/RISCVTargetParserDef.inc"
};

static const CPUInfo *getCPUInfoByName(StringRef CPU) {
  for (auto &C : RISCVCPUInfo)
    if (C.Name == CPU)
      return &C;
  return nullptr;
}

bool hasFastScalarUnalignedAccess(StringRef CPU) {
  const CPUInfo *Info = getCPUInfoByName(CPU);
  return Info && Info->FastScalarUnalignedAccess;
}

bool hasFastVectorUnalignedAccess(StringRef CPU) {
  const CPUInfo *Info = getCPUInfoByName(CPU);
  return Info && Info->FastVectorUnalignedAccess;
}

bool hasValidCPUModel(StringRef CPU) { return getCPUModel(CPU).isValid(); }

CPUModel getCPUModel(StringRef CPU) {
  const CPUInfo *Info = getCPUInfoByName(CPU);
  if (!Info)
    return {0, 0, 0};
  return Info->Model;
}

StringRef getCPUNameFromCPUModel(const CPUModel &Model) {
  if (!Model.isValid())
    return "";

  for (auto &C : RISCVCPUInfo)
    if (C.Model == Model)
      return C.Name;
  return "";
}

bool parseCPU(StringRef CPU, bool IsRV64) {
  const CPUInfo *Info = getCPUInfoByName(CPU);

  if (!Info)
    return false;
  return Info->is64Bit() == IsRV64;
}

bool parseTuneCPU(StringRef TuneCPU, bool IsRV64) {
  std::optional<CPUKind> Kind =
      toolchain::StringSwitch<std::optional<CPUKind>>(TuneCPU)
#define TUNE_PROC(ENUM, NAME) .Case(NAME, CK_##ENUM)
  #include "vm/core/TargetParser/RISCVTargetParserDef.inc"
      .Default(std::nullopt);

  if (Kind.has_value())
    return true;

  // Fallback to parsing as a CPU.
  return parseCPU(TuneCPU, IsRV64);
}

StringRef getMArchFromMcpu(StringRef CPU) {
  const CPUInfo *Info = getCPUInfoByName(CPU);
  if (!Info)
    return "";
  return Info->DefaultMarch;
}

void fillValidCPUArchList(SmallVectorImpl<StringRef> &Values, bool IsRV64) {
  for (const auto &C : RISCVCPUInfo) {
    if (IsRV64 == C.is64Bit())
      Values.emplace_back(C.Name);
  }
}

void fillValidTuneCPUArchList(SmallVectorImpl<StringRef> &Values, bool IsRV64) {
  for (const auto &C : RISCVCPUInfo) {
    if (IsRV64 == C.is64Bit())
      Values.emplace_back(C.Name);
  }
#define TUNE_PROC(ENUM, NAME) Values.emplace_back(StringRef(NAME));
#include "vm/core/TargetParser/RISCVTargetParserDef.inc"
}

// This function is currently used by IREE, so it's not dead code.
void getFeaturesForCPU(StringRef CPU,
                       SmallVectorImpl<std::string> &EnabledFeatures,
                       bool NeedPlus) {
  StringRef MarchFromCPU = toolchain::RISCV::getMArchFromMcpu(CPU);
  if (MarchFromCPU == "")
    return;

  EnabledFeatures.clear();
  auto RII = RISCVISAInfo::parseArchString(
      MarchFromCPU, /* EnableExperimentalExtension */ true);

  if (toolchain::errorToBool(RII.takeError()))
    return;

  std::vector<std::string> FeatStrings =
      (*RII)->toFeatures(/* AddAllExtensions */ false);
  for (const auto &F : FeatStrings)
    if (NeedPlus)
      EnabledFeatures.push_back(F);
    else
      EnabledFeatures.push_back(F.substr(1));
}

} // namespace RISCV

namespace RISCVVType {
// Encode VTYPE into the binary format used by the the VSETVLI instruction which
// is used by our MC layer representation.
//
// Bits | Name       | Description
// -----+------------+------------------------------------------------
// 8    | altfmt     | Alternative format for bf16/ofp8
// 7    | vma        | Vector mask agnostic
// 6    | vta        | Vector tail agnostic
// 5:3  | vsew[2:0]  | Standard element width (SEW) setting
// 2:0  | vlmul[2:0] | Vector register group multiplier (LMUL) setting
unsigned encodeVTYPE(VLMUL VLMul, unsigned SEW, bool TailAgnostic,
                     bool MaskAgnostic, bool AltFmt) {
  assert(isValidSEW(SEW) && "Invalid SEW");
  unsigned VLMulBits = static_cast<unsigned>(VLMul);
  unsigned VSEWBits = encodeSEW(SEW);
  unsigned VTypeI = (VSEWBits << 3) | (VLMulBits & 0x7);
  if (TailAgnostic)
    VTypeI |= 0x40;
  if (MaskAgnostic)
    VTypeI |= 0x80;
  if (AltFmt)
    VTypeI |= 0x100;

  return VTypeI;
}

unsigned encodeXSfmmVType(unsigned SEW, unsigned Widen, bool AltFmt) {
  assert(isValidSEW(SEW) && "Invalid SEW");
  assert((Widen == 1 || Widen == 2 || Widen == 4) && "Invalid Widen");
  unsigned VSEWBits = encodeSEW(SEW);
  unsigned TWiden = Log2_32(Widen) + 1;
  unsigned VTypeI = (VSEWBits << 3) | AltFmt << 8 | TWiden << 9;
  return VTypeI;
}

std::pair<unsigned, bool> decodeVLMUL(VLMUL VLMul) {
  switch (VLMul) {
  default:
    llvm_unreachable("Unexpected LMUL value!");
  case LMUL_1:
  case LMUL_2:
  case LMUL_4:
  case LMUL_8:
    return std::make_pair(1 << static_cast<unsigned>(VLMul), false);
  case LMUL_F2:
  case LMUL_F4:
  case LMUL_F8:
    return std::make_pair(1 << (8 - static_cast<unsigned>(VLMul)), true);
  }
}

void printVType(unsigned VType, raw_ostream &OS) {
  unsigned Sew = getSEW(VType);
  OS << "e" << Sew;

  bool AltFmt = RISCVVType::isAltFmt(VType);
  if (AltFmt)
    OS << "alt";

  unsigned LMul;
  bool Fractional;
  std::tie(LMul, Fractional) = decodeVLMUL(getVLMUL(VType));

  if (Fractional)
    OS << ", mf";
  else
    OS << ", m";
  OS << LMul;

  if (isTailAgnostic(VType))
    OS << ", ta";
  else
    OS << ", tu";

  if (isMaskAgnostic(VType))
    OS << ", ma";
  else
    OS << ", mu";
}

void printXSfmmVType(unsigned VType, raw_ostream &OS) {
  OS << "e" << getSEW(VType) << ", w" << getXSfmmWiden(VType);
}

unsigned getSEWLMULRatio(unsigned SEW, VLMUL VLMul) {
  unsigned LMul;
  bool Fractional;
  std::tie(LMul, Fractional) = decodeVLMUL(VLMul);

  // Convert LMul to a fixed point value with 3 fractional bits.
  LMul = Fractional ? (8 / LMul) : (LMul * 8);

  assert(SEW >= 8 && "Unexpected SEW value");
  return (SEW * 8) / LMul;
}

std::optional<VLMUL> getSameRatioLMUL(unsigned Ratio, unsigned EEW) {
  unsigned EMULFixedPoint = (EEW * 8) / Ratio;
  bool Fractional = EMULFixedPoint < 8;
  unsigned EMUL = Fractional ? 8 / EMULFixedPoint : EMULFixedPoint / 8;
  if (!isValidLMUL(EMUL, Fractional))
    return std::nullopt;
  return RISCVVType::encodeLMUL(EMUL, Fractional);
}

} // namespace RISCVVType

} // namespace vm::core
