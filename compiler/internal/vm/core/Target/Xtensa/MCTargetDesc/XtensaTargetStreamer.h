//===-- XtensaTargetStreamer.h - Xtensa Target Streamer --------*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_XTENSA_XTENSATARGETSTREAMER_H
#define LLVM_LIB_TARGET_XTENSA_XTENSATARGETSTREAMER_H

#include "vm/core/MC/MCELFStreamer.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/SMLoc.h"

namespace vm::core {
class formatted_raw_ostream;

class XtensaTargetStreamer : public MCTargetStreamer {
public:
  XtensaTargetStreamer(MCStreamer &S);

  // Emit literal label and literal Value to the literal section. If literal
  // section is not switched yet (SwitchLiteralSection is true) then switch to
  // literal section.
  virtual void emitLiteral(MCSymbol *LblSym, const MCExpr *Value,
                           bool SwitchLiteralSection, SMLoc L = SMLoc()) = 0;

  virtual void emitLiteralPosition() = 0;

  // Switch to the literal section. The BaseSection name is used to construct
  // literal section name.
  virtual void startLiteralSection(MCSection *BaseSection) = 0;
};

class XtensaTargetAsmStreamer : public XtensaTargetStreamer {
  formatted_raw_ostream &OS;

public:
  XtensaTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
  void emitLiteral(MCSymbol *LblSym, const MCExpr *Value,
                   bool SwitchLiteralSection, SMLoc L) override;
  void emitLiteralPosition() override;
  void startLiteralSection(MCSection *Section) override;
};

class XtensaTargetELFStreamer : public XtensaTargetStreamer {
public:
  XtensaTargetELFStreamer(MCStreamer &S);
  MCELFStreamer &getStreamer();
  void emitLiteral(MCSymbol *LblSym, const MCExpr *Value,
                   bool SwitchLiteralSection, SMLoc L) override;
  void emitLiteralPosition() override {}
  void startLiteralSection(MCSection *Section) override;
};
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_XTENSA_XTENSATARGETSTREAMER_H
