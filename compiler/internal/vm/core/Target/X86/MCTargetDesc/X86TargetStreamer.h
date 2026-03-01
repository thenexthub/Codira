//===- X86TargetStreamer.h ------------------------------*- C++ -*---------===//
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

#ifndef LLVM_LIB_TARGET_X86_MCTARGETDESC_X86TARGETSTREAMER_H
#define LLVM_LIB_TARGET_X86_MCTARGETDESC_X86TARGETSTREAMER_H

#include "vm/core/MC/MCStreamer.h"

namespace vm::core {

/// X86 target streamer implementing x86-only assembly directives.
class X86TargetStreamer : public MCTargetStreamer {
public:
  X86TargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

  virtual void emitCode16() {}
  virtual void emitCode32() {}
  virtual void emitCode64() {}

  virtual bool emitFPOProc(const MCSymbol *ProcSym, unsigned ParamsSize,
                           SMLoc L = {}) {
    return false;
  }
  virtual bool emitFPOEndPrologue(SMLoc L = {}) { return false; }
  virtual bool emitFPOEndProc(SMLoc L = {}) { return false; };
  virtual bool emitFPOData(const MCSymbol *ProcSym, SMLoc L = {}) {
    return false;
  }
  virtual bool emitFPOPushReg(MCRegister Reg, SMLoc L = {}) { return false; }
  virtual bool emitFPOStackAlloc(unsigned StackAlloc, SMLoc L = {}) {
    return false;
  }
  virtual bool emitFPOStackAlign(unsigned Align, SMLoc L = {}) { return false; }
  virtual bool emitFPOSetFrame(MCRegister Reg, SMLoc L = {}) { return false; }
};

/// Implements X86-only null emission.
inline MCTargetStreamer *createX86NullTargetStreamer(MCStreamer &S) {
  return new X86TargetStreamer(S);
}

} // end namespace vm::core

#endif
