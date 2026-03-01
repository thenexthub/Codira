//===-- toolchain/Target/AMDGPU/AMDGPUMIRFormatter.h -----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
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
/// \file
/// AMDGPU specific overrides of MIRFormatter.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPUMIRFORMATTER_H
#define LLVM_LIB_TARGET_AMDGPUMIRFORMATTER_H

#include "vm/core/CodeGen/MIRFormatter.h"

namespace vm::core {

class MachineFunction;
struct PerFunctionMIParsingState;

class AMDGPUMIRFormatter final : public MIRFormatter {
public:
  AMDGPUMIRFormatter() = default;
  ~AMDGPUMIRFormatter() override = default;

  /// Implement target specific printing for machine operand immediate value, so
  /// that we can have more meaningful mnemonic than a 64-bit integer. Passing
  /// None to OpIdx means the index is unknown.
  void printImm(raw_ostream &OS, const MachineInstr &MI,
                std::optional<unsigned> OpIdx, int64_t Imm) const override;

  /// Implement target specific parsing of immediate mnemonics. The mnemonic is
  /// a string with a leading dot.
  bool parseImmMnemonic(const unsigned OpCode, const unsigned OpIdx,
                        StringRef Src, int64_t &Imm,
                        ErrorCallbackType ErrorCallback) const override;

  /// Implement target specific parsing of target custom pseudo source value.
  bool
  parseCustomPseudoSourceValue(StringRef Src, MachineFunction &MF,
                               PerFunctionMIParsingState &PFS,
                               const PseudoSourceValue *&PSV,
                               ErrorCallbackType ErrorCallback) const override;

private:
  /// Print the string to represent s_delay_alu immediate value
  void printSDelayAluImm(int64_t Imm, toolchain::raw_ostream &OS) const;

  /// Parse the immediate pseudo literal for s_delay_alu
  bool parseSDelayAluImmMnemonic(
      const unsigned int OpIdx, int64_t &Imm, toolchain::StringRef &Src,
      toolchain::MIRFormatter::ErrorCallbackType &ErrorCallback) const;

};

} // end namespace vm::core

#endif
