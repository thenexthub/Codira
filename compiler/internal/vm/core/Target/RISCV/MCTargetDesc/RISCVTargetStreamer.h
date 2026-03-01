//===-- RISCVTargetStreamer.h - RISC-V Target Streamer ---------*- C++ -*--===//
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

#ifndef LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVTARGETSTREAMER_H
#define LLVM_LIB_TARGET_RISCV_MCTARGETDESC_RISCVTARGETSTREAMER_H

#include "RISCV.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSubtargetInfo.h"

namespace vm::core {

class formatted_raw_ostream;

enum class RISCVOptionArchArgType {
  Full,
  Plus,
  Minus,
};

struct RISCVOptionArchArg {
  RISCVOptionArchArgType Type;
  std::string Value;

  RISCVOptionArchArg(RISCVOptionArchArgType Type, std::string Value)
      : Type(Type), Value(Value) {}
};

class RISCVTargetStreamer : public MCTargetStreamer {
  RISCVABI::ABI TargetABI = RISCVABI::ABI_Unknown;
  bool HasRVC = false;
  bool HasTSO = false;

public:
  RISCVTargetStreamer(MCStreamer &S);
  void finish() override;
  virtual void reset();

  virtual void emitDirectiveOptionArch(ArrayRef<RISCVOptionArchArg> Args);
  virtual void emitDirectiveOptionExact();
  virtual void emitDirectiveOptionNoExact();
  virtual void emitDirectiveOptionPIC();
  virtual void emitDirectiveOptionNoPIC();
  virtual void emitDirectiveOptionPop();
  virtual void emitDirectiveOptionPush();
  virtual void emitDirectiveOptionRelax();
  virtual void emitDirectiveOptionNoRelax();
  virtual void emitDirectiveOptionRVC();
  virtual void emitDirectiveOptionNoRVC();
  virtual void emitDirectiveVariantCC(MCSymbol &Symbol);
  virtual void emitAttribute(unsigned Attribute, unsigned Value);
  virtual void finishAttributeSection();
  virtual void emitTextAttribute(unsigned Attribute, StringRef String);
  virtual void emitIntTextAttribute(unsigned Attribute, unsigned IntValue,
                                    StringRef StringValue);

  void emitTargetAttributes(const MCSubtargetInfo &STI, bool EmitStackAlign);
  void setTargetABI(RISCVABI::ABI ABI);
  RISCVABI::ABI getTargetABI() const { return TargetABI; }
  void setFlagsFromFeatures(const MCSubtargetInfo &STI);
  bool hasRVC() const { return HasRVC; }
  bool hasTSO() const { return HasTSO; }
};

// This part is for ascii assembly output
class RISCVTargetAsmStreamer : public RISCVTargetStreamer {
  formatted_raw_ostream &OS;

  void finishAttributeSection() override;
  void emitAttribute(unsigned Attribute, unsigned Value) override;
  void emitTextAttribute(unsigned Attribute, StringRef String) override;
  void emitIntTextAttribute(unsigned Attribute, unsigned IntValue,
                            StringRef StringValue) override;

public:
  RISCVTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);

  void emitDirectiveOptionArch(ArrayRef<RISCVOptionArchArg> Args) override;
  void emitDirectiveOptionExact() override;
  void emitDirectiveOptionNoExact() override;
  void emitDirectiveOptionPIC() override;
  void emitDirectiveOptionNoPIC() override;
  void emitDirectiveOptionPop() override;
  void emitDirectiveOptionPush() override;
  void emitDirectiveOptionRelax() override;
  void emitDirectiveOptionNoRelax() override;
  void emitDirectiveOptionRVC() override;
  void emitDirectiveOptionNoRVC() override;
  void emitDirectiveVariantCC(MCSymbol &Symbol) override;
};

}
#endif
