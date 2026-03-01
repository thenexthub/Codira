//===-- SparcTargetStreamer.h - Sparc Target Streamer ----------*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCTARGETSTREAMER_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCTARGETSTREAMER_H

#include "vm/core/MC/MCELFStreamer.h"
#include "vm/core/MC/MCStreamer.h"

namespace vm::core {

class formatted_raw_ostream;

class SparcTargetStreamer : public MCTargetStreamer {
  virtual void anchor();

public:
  SparcTargetStreamer(MCStreamer &S);
  /// Emit ".register <reg>, #ignore".
  virtual void emitSparcRegisterIgnore(unsigned reg){};
  /// Emit ".register <reg>, #scratch".
  virtual void emitSparcRegisterScratch(unsigned reg){};
};

// This part is for ascii assembly output
class SparcTargetAsmStreamer : public SparcTargetStreamer {
  formatted_raw_ostream &OS;

public:
  SparcTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
  void emitSparcRegisterIgnore(unsigned reg) override;
  void emitSparcRegisterScratch(unsigned reg) override;
};

// This part is for ELF object output
class SparcTargetELFStreamer : public SparcTargetStreamer {
public:
  SparcTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);
  MCELFStreamer &getStreamer();
  void emitSparcRegisterIgnore(unsigned reg) override {}
  void emitSparcRegisterScratch(unsigned reg) override {}
};
} // end namespace vm::core

#endif
