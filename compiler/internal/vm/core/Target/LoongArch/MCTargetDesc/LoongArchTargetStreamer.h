//===-- LoongArchTargetStreamer.h - LoongArch Target Streamer --*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHTARGETSTREAMER_H
#define LLVM_LIB_TARGET_LOONGARCH_MCTARGETDESC_LOONGARCHTARGETSTREAMER_H

#include "LoongArch.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/Support/FormattedStream.h"

namespace vm::core {
class LoongArchTargetStreamer : public MCTargetStreamer {
  LoongArchABI::ABI TargetABI = LoongArchABI::ABI_Unknown;

public:
  LoongArchTargetStreamer(MCStreamer &S);
  void setTargetABI(LoongArchABI::ABI ABI);
  LoongArchABI::ABI getTargetABI() const { return TargetABI; }

  virtual void emitDirectiveOptionPush();
  virtual void emitDirectiveOptionPop();
  virtual void emitDirectiveOptionRelax();
  virtual void emitDirectiveOptionNoRelax();
};

// This part is for ascii assembly output.
class LoongArchTargetAsmStreamer : public LoongArchTargetStreamer {
  formatted_raw_ostream &OS;

public:
  LoongArchTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);

  void emitDirectiveOptionPush() override;
  void emitDirectiveOptionPop() override;
  void emitDirectiveOptionRelax() override;
  void emitDirectiveOptionNoRelax() override;
};

} // end namespace vm::core
#endif
