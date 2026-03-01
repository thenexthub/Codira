//===-- EmulateInstructionPPC64.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_INSTRUCTION_PPC64_EMULATEINSTRUCTIONPPC64_H
#define LLDB_SOURCE_PLUGINS_INSTRUCTION_PPC64_EMULATEINSTRUCTIONPPC64_H

#include "lldb/Core/EmulateInstruction.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Log.h"
#include <optional>

namespace lldb_private {

class EmulateInstructionPPC64 : public EmulateInstruction {
public:
  EmulateInstructionPPC64(const ArchSpec &arch);

  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "ppc64"; }

  static llvm::StringRef GetPluginDescriptionStatic();

  static EmulateInstruction *CreateInstance(const ArchSpec &arch,
                                            InstructionType inst_type);

  static bool
  SupportsEmulatingInstructionsOfTypeStatic(InstructionType inst_type) {
    switch (inst_type) {
    case eInstructionTypeAny:
    case eInstructionTypePrologueEpilogue:
      return true;

    case eInstructionTypePCModifying:
    case eInstructionTypeAll:
      return false;
    }
    return false;
  }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  bool SetTargetTriple(const ArchSpec &arch) override;

  bool SupportsEmulatingInstructionsOfType(InstructionType inst_type) override {
    return SupportsEmulatingInstructionsOfTypeStatic(inst_type);
  }

  bool ReadInstruction() override;

  bool EvaluateInstruction(uint32_t evaluate_options) override;

  bool TestEmulation(Stream &out_stream, ArchSpec &arch,
                     OptionValueDictionary *test_data) override {
    return false;
  }

  std::optional<RegisterInfo> GetRegisterInfo(lldb::RegisterKind reg_kind,
                                              uint32_t reg_num) override;

  bool CreateFunctionEntryUnwind(UnwindPlan &unwind_plan) override;

private:
  struct Opcode {
    uint32_t mask;
    uint32_t value;
    bool (EmulateInstructionPPC64::*callback)(uint32_t opcode);
    const char *name;
  };

  uint32_t m_fp = LLDB_INVALID_REGNUM;

  Opcode *GetOpcodeForInstruction(uint32_t opcode);

  bool EmulateMFSPR(uint32_t opcode);
  bool EmulateLD(uint32_t opcode);
  bool EmulateSTD(uint32_t opcode);
  bool EmulateOR(uint32_t opcode);
  bool EmulateADDI(uint32_t opcode);
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_INSTRUCTION_PPC64_EMULATEINSTRUCTIONPPC64_H
