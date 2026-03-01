//===---EmulateInstructionLoongArch.h--------------------------------------===//
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

#ifndef LLDB_SOURCE_PLUGINS_INSTRUCTION_LOONGARCH_EMULATEINSTRUCTIONLOONGARCH_H
#define LLDB_SOURCE_PLUGINS_INSTRUCTION_LOONGARCH_EMULATEINSTRUCTIONLOONGARCH_H

#include "lldb/Core/EmulateInstruction.h"
#include "lldb/Interpreter/OptionValue.h"
#include "lldb/Utility/Log.h"
#include "lldb/Utility/Status.h"
#include <optional>

namespace lldb_private {

class EmulateInstructionLoongArch : public EmulateInstruction {
public:
  static llvm::StringRef GetPluginNameStatic() { return "LoongArch"; }

  static llvm::StringRef GetPluginDescriptionStatic() {
    return "Emulate instructions for the LoongArch architecture.";
  }

  static bool SupportsThisInstructionType(InstructionType inst_type) {
    return inst_type == eInstructionTypePCModifying;
  }

  static bool SupportsThisArch(const ArchSpec &arch);

  static lldb_private::EmulateInstruction *
  CreateInstance(const lldb_private::ArchSpec &arch, InstructionType inst_type);

  static void Initialize();

  static void Terminate();

public:
  EmulateInstructionLoongArch(const ArchSpec &arch) : EmulateInstruction(arch) {
    m_arch_subtype = arch.GetMachine();
  }

  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

  bool SupportsEmulatingInstructionsOfType(InstructionType inst_type) override {
    return SupportsThisInstructionType(inst_type);
  }

  bool SetTargetTriple(const ArchSpec &arch) override;
  bool ReadInstruction() override;
  bool EvaluateInstruction(uint32_t options) override;
  bool TestEmulation(Stream &out_stream, ArchSpec &arch,
                     OptionValueDictionary *test_data) override;

  std::optional<RegisterInfo> GetRegisterInfo(lldb::RegisterKind reg_kind,
                                              uint32_t reg_num) override;
  bool IsLoongArch64() { return m_arch_subtype == llvm::Triple::loongarch64; }
  bool TestExecute(uint32_t inst);

private:
  struct Opcode {
    uint32_t mask;
    uint32_t value;
    bool (EmulateInstructionLoongArch::*callback)(uint32_t opcode);
    const char *name;
  };

  llvm::Triple::ArchType m_arch_subtype;
  Opcode *GetOpcodeForInstruction(uint32_t inst);

  bool EmulateBEQZ(uint32_t inst);
  bool EmulateBNEZ(uint32_t inst);
  bool EmulateBCEQZ(uint32_t inst);
  bool EmulateBCNEZ(uint32_t inst);
  bool EmulateJIRL(uint32_t inst);
  bool EmulateB(uint32_t inst);
  bool EmulateBL(uint32_t inst);
  bool EmulateBEQ(uint32_t inst);
  bool EmulateBNE(uint32_t inst);
  bool EmulateBLT(uint32_t inst);
  bool EmulateBGE(uint32_t inst);
  bool EmulateBLTU(uint32_t inst);
  bool EmulateBGEU(uint32_t inst);
  bool EmulateNonJMP(uint32_t inst);

  bool EmulateBEQZ64(uint32_t inst);
  bool EmulateBNEZ64(uint32_t inst);
  bool EmulateBCEQZ64(uint32_t inst);
  bool EmulateBCNEZ64(uint32_t inst);
  bool EmulateJIRL64(uint32_t inst);
  bool EmulateB64(uint32_t inst);
  bool EmulateBL64(uint32_t inst);
  bool EmulateBEQ64(uint32_t inst);
  bool EmulateBNE64(uint32_t inst);
  bool EmulateBLT64(uint32_t inst);
  bool EmulateBGE64(uint32_t inst);
  bool EmulateBLTU64(uint32_t inst);
  bool EmulateBGEU64(uint32_t inst);
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_INSTRUCTION_LOONGARCH_EMULATEINSTRUCTIONLOONGARCH_H
