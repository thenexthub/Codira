//===-- X86MCAsmInfo.h - X86 asm properties --------------------*- C++ -*--===//
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
// This file contains the declaration of the X86MCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_X86_MCTARGETDESC_X86MCASMINFO_H
#define LLVM_LIB_TARGET_X86_MCTARGETDESC_X86MCASMINFO_H

#include "MCTargetDesc/X86MCExpr.h"
#include "vm/core/MC/MCAsmInfoCOFF.h"
#include "vm/core/MC/MCAsmInfoDarwin.h"
#include "vm/core/MC/MCAsmInfoELF.h"
#include "vm/core/MC/MCExpr.h"

namespace vm::core {
class Triple;

class X86MCAsmInfoDarwin : public MCAsmInfoDarwin {
  virtual void anchor();

public:
  explicit X86MCAsmInfoDarwin(const Triple &Triple);
};

struct X86_64MCAsmInfoDarwin : public X86MCAsmInfoDarwin {
  explicit X86_64MCAsmInfoDarwin(const Triple &Triple);
  const MCExpr *
  getExprForPersonalitySymbol(const MCSymbol *Sym, unsigned Encoding,
                              MCStreamer &Streamer) const override;
};

class X86ELFMCAsmInfo : public MCAsmInfoELF {
  void anchor() override;

public:
  explicit X86ELFMCAsmInfo(const Triple &Triple);
};

class X86MCAsmInfoMicrosoft : public MCAsmInfoMicrosoft {
  void anchor() override;

public:
  explicit X86MCAsmInfoMicrosoft(const Triple &Triple);
};

class X86MCAsmInfoMicrosoftMASM : public X86MCAsmInfoMicrosoft {
  void anchor() override;

public:
  explicit X86MCAsmInfoMicrosoftMASM(const Triple &Triple);
};

class X86MCAsmInfoGNUCOFF : public MCAsmInfoGNUCOFF {
  void anchor() override;

public:
  explicit X86MCAsmInfoGNUCOFF(const Triple &Triple);
};

namespace X86 {
using Specifier = uint16_t;

enum {
  S_None,
  S_COFF_SECREL,

  S_ABS8 = MCSymbolRefExpr::FirstTargetSpecifier,
  S_DTPOFF,
  S_DTPREL,
  S_GOT,
  S_GOTENT,
  S_GOTNTPOFF,
  S_GOTOFF,
  S_GOTPCREL,
  S_GOTPCREL_NORELAX,
  S_GOTREL,
  S_GOTTPOFF,
  S_INDNTPOFF,
  S_NTPOFF,
  S_PCREL,
  S_PLT,
  S_PLTOFF,
  S_SIZE,
  S_TLSCALL,
  S_TLSDESC,
  S_TLSGD,
  S_TLSLD,
  S_TLSLDM,
  S_TLVP,
  S_TLVPPAGE,
  S_TLVPPAGEOFF,
  S_TPOFF,
};
} // namespace X86
} // namespace vm::core

#endif
