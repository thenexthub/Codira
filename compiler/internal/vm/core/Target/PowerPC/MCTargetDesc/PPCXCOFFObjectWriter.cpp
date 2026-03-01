//===-- PPCXCOFFObjectWriter.cpp - PowerPC XCOFF Writer -------------------===//
//
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

#include "MCTargetDesc/PPCFixupKinds.h"
#include "MCTargetDesc/PPCMCTargetDesc.h"
#include "PPCMCAsmInfo.h"
#include "vm/core/BinaryFormat/XCOFF.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/MC/MCXCOFFObjectWriter.h"

using namespace vm::core;

namespace {
class PPCXCOFFObjectWriter : public MCXCOFFObjectTargetWriter {
  static constexpr uint8_t SignBitMask = 0x80;

public:
  PPCXCOFFObjectWriter(bool Is64Bit);

  std::pair<uint8_t, uint8_t>
  getRelocTypeAndSignSize(const MCValue &Target, const MCFixup &Fixup,
                          bool IsPCRel) const override;
};
} // end anonymous namespace

PPCXCOFFObjectWriter::PPCXCOFFObjectWriter(bool Is64Bit)
    : MCXCOFFObjectTargetWriter(Is64Bit) {}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createPPCXCOFFObjectWriter(bool Is64Bit) {
  return std::make_unique<PPCXCOFFObjectWriter>(Is64Bit);
}

std::pair<uint8_t, uint8_t> PPCXCOFFObjectWriter::getRelocTypeAndSignSize(
    const MCValue &Target, const MCFixup &Fixup, bool IsPCRel) const {
  const auto Specifier = Target.getSpecifier();
  // People from AIX OS team says AIX link editor does not care about
  // the sign bit in the relocation entry "most" of the time.
  // The system assembler seems to set the sign bit on relocation entry
  // based on similar property of IsPCRel. So we will do the same here.
  // TODO: More investigation on how assembler decides to set the sign
  // bit, and we might want to match that.
  const uint8_t EncodedSignednessIndicator = IsPCRel ? SignBitMask : 0u;

  // The magic number we use in SignAndSize has a strong relationship with
  // the corresponding MCFixupKind. In most cases, it's the MCFixupKind
  // number - 1, because SignAndSize encodes the bit length being
  // relocated minus 1.
  switch ((unsigned)Fixup.getKind()) {
  default:
    report_fatal_error("Unimplemented fixup kind.");
  case XCOFF::RelocationType::R_REF:
    return {XCOFF::RelocationType::R_REF, 0};
  case PPC::fixup_ppc_half16: {
    const uint8_t SignAndSizeForHalf16 = EncodedSignednessIndicator | 15;
    switch (Specifier) {
    default:
      report_fatal_error("Unsupported modifier for half16 fixup.");
    case PPC::S_None:
      return {XCOFF::RelocationType::R_TOC, SignAndSizeForHalf16};
    case PPC::S_U:
      return {XCOFF::RelocationType::R_TOCU, SignAndSizeForHalf16};
    case PPC::S_L:
      return {XCOFF::RelocationType::R_TOCL, SignAndSizeForHalf16};
    case PPC::S_AIX_TLSLE:
      return {XCOFF::RelocationType::R_TLS_LE, SignAndSizeForHalf16};
    case PPC::S_AIX_TLSLD:
      return {XCOFF::RelocationType::R_TLS_LD, SignAndSizeForHalf16};
    }
  } break;
  case PPC::fixup_ppc_half16ds:
  case PPC::fixup_ppc_half16dq: {
    if (IsPCRel)
      report_fatal_error("Invalid PC-relative relocation.");
    switch (Specifier) {
    default:
      llvm_unreachable("Unsupported Modifier");
    case PPC::S_None:
      return {XCOFF::RelocationType::R_TOC, 15};
    case PPC::S_L:
      return {XCOFF::RelocationType::R_TOCL, 15};
    case PPC::S_AIX_TLSLE:
      return {XCOFF::RelocationType::R_TLS_LE, 15};
    case PPC::S_AIX_TLSLD:
      return {XCOFF::RelocationType::R_TLS_LD, 15};
    }
  } break;
  case PPC::fixup_ppc_br24:
    // Branches are 4 byte aligned, so the 24 bits we encode in
    // the instruction actually represents a 26 bit offset.
    return {XCOFF::RelocationType::R_RBR, EncodedSignednessIndicator | 25};
  case PPC::fixup_ppc_br24abs:
    return {XCOFF::RelocationType::R_RBA, EncodedSignednessIndicator | 25};
  case FK_Data_4:
  case FK_Data_8:
    const uint8_t SignAndSizeForFKData =
        EncodedSignednessIndicator |
        ((unsigned)Fixup.getKind() == FK_Data_4 ? 31 : 63);
    switch (Specifier) {
    default:
      report_fatal_error("Unsupported modifier");
    case PPC::S_AIX_TLSGD:
      return {XCOFF::RelocationType::R_TLS, SignAndSizeForFKData};
    case PPC::S_AIX_TLSGDM:
      return {XCOFF::RelocationType::R_TLSM, SignAndSizeForFKData};
    case PPC::S_AIX_TLSIE:
      return {XCOFF::RelocationType::R_TLS_IE, SignAndSizeForFKData};
    case PPC::S_AIX_TLSLE:
      return {XCOFF::RelocationType::R_TLS_LE, SignAndSizeForFKData};
    case PPC::S_AIX_TLSLD:
      return {XCOFF::RelocationType::R_TLS_LD, SignAndSizeForFKData};
    case PPC::S_AIX_TLSML:
      return {XCOFF::RelocationType::R_TLSML, SignAndSizeForFKData};
    case PPC::S_None:
      return {XCOFF::RelocationType::R_POS, SignAndSizeForFKData};
    }
  }
}
