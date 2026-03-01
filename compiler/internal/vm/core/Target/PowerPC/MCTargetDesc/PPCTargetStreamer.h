//===- PPCTargetStreamer.h - PPC Target Streamer ----------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_POWERPC_PPCTARGETSTREAMER_H
#define LLVM_LIB_TARGET_POWERPC_PPCTARGETSTREAMER_H

#include "PPCMCAsmInfo.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCStreamer.h"

namespace vm::core {

class MCExpr;
class MCSymbol;
class MCSymbolELF;

class PPCTargetStreamer : public MCTargetStreamer {
public:
  PPCTargetStreamer(MCStreamer &S);
  ~PPCTargetStreamer() override;

  virtual void emitTCEntry(const MCSymbol &S, PPCMCExpr::Specifier Kind) {}
  virtual void emitMachine(StringRef CPU){};
  virtual void emitAbiVersion(int AbiVersion){};
  virtual void emitLocalEntry(MCSymbolELF *S, const MCExpr *LocalOffset){};
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_POWERPC_PPCTARGETSTREAMER_H
