//===- PPCXCOFFStreamer.h - XCOFF Object Output -----------------*- C++ -*-===//
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
// This is a custom MCXCOFFStreamer for PowerPC.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_PPC_MCXCOFFSTREAMER_PPCXCOFFSTREAMER_H
#define LLVM_LIB_TARGET_PPC_MCXCOFFSTREAMER_PPCXCOFFSTREAMER_H

#include "vm/core/MC/MCXCOFFStreamer.h"

namespace vm::core {

class PPCXCOFFStreamer : public MCXCOFFStreamer {
public:
  PPCXCOFFStreamer(MCContext &Context, std::unique_ptr<MCAsmBackend> MAB,
                   std::unique_ptr<MCObjectWriter> OW,
                   std::unique_ptr<MCCodeEmitter> Emitter);

  void emitInstruction(const MCInst &Inst, const MCSubtargetInfo &STI) override;

private:
  void emitPrefixedInstruction(const MCInst &Inst, const MCSubtargetInfo &STI);
};

MCStreamer *createPPCXCOFFStreamer(const Triple &, MCContext &,
                                   std::unique_ptr<MCAsmBackend> &&MAB,
                                   std::unique_ptr<MCObjectWriter> &&OW,
                                   std::unique_ptr<MCCodeEmitter> &&Emitter);
} // end namespace vm::core

#endif // LLVM_LIB_TARGET_PPC_MCXCOFFSTREAMER_PPCXCOFFSTREAMER_H
