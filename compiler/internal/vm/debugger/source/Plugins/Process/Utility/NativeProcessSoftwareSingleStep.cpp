//===-- NativeProcessSoftwareSingleStep.cpp -------------------------------===//
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

#include "NativeProcessSoftwareSingleStep.h"

#include "lldb/Core/EmulateInstruction.h"
#include "lldb/Host/common/NativeRegisterContext.h"
#include "lldb/Utility/RegisterValue.h"

#include <unordered_map>

using namespace lldb;
using namespace lldb_private;

namespace {

struct EmulatorBaton {
  NativeProcessProtocol &m_process;
  NativeRegisterContext &m_reg_context;

  // eRegisterKindDWARF -> RegsiterValue
  std::unordered_map<uint32_t, RegisterValue> m_register_values;

  EmulatorBaton(NativeProcessProtocol &process,
                NativeRegisterContext &reg_context)
      : m_process(process), m_reg_context(reg_context) {}
};

} // anonymous namespace

static size_t ReadMemoryCallback(EmulateInstruction *instruction, void *baton,
                                 const EmulateInstruction::Context &context,
                                 lldb::addr_t addr, void *dst, size_t length) {
  EmulatorBaton *emulator_baton = static_cast<EmulatorBaton *>(baton);

  size_t bytes_read;
  emulator_baton->m_process.ReadMemory(addr, dst, length, bytes_read);
  return bytes_read;
}

static bool ReadRegisterCallback(EmulateInstruction *instruction, void *baton,
                                 const RegisterInfo *reg_info,
                                 RegisterValue &reg_value) {
  EmulatorBaton *emulator_baton = static_cast<EmulatorBaton *>(baton);

  auto it = emulator_baton->m_register_values.find(
      reg_info->kinds[eRegisterKindDWARF]);
  if (it != emulator_baton->m_register_values.end()) {
    reg_value = it->second;
    return true;
  }

  // The emulator only fill in the dwarf regsiter numbers (and in some case the
  // generic register numbers). Get the full register info from the register
  // context based on the dwarf register numbers.
  const RegisterInfo *full_reg_info =
      emulator_baton->m_reg_context.GetRegisterInfo(
          eRegisterKindDWARF, reg_info->kinds[eRegisterKindDWARF]);

  Status error =
      emulator_baton->m_reg_context.ReadRegister(full_reg_info, reg_value);
  if (error.Success())
    return true;

  return false;
}

static bool WriteRegisterCallback(EmulateInstruction *instruction, void *baton,
                                  const EmulateInstruction::Context &context,
                                  const RegisterInfo *reg_info,
                                  const RegisterValue &reg_value) {
  EmulatorBaton *emulator_baton = static_cast<EmulatorBaton *>(baton);
  emulator_baton->m_register_values[reg_info->kinds[eRegisterKindDWARF]] =
      reg_value;
  return true;
}

static size_t WriteMemoryCallback(EmulateInstruction *instruction, void *baton,
                                  const EmulateInstruction::Context &context,
                                  lldb::addr_t addr, const void *dst,
                                  size_t length) {
  return length;
}

static Status SetSoftwareBreakpoint(lldb::addr_t bp_addr, unsigned bp_size,
                                    NativeProcessProtocol &process) {
  Status error;
  error = process.SetBreakpoint(bp_addr, bp_size, /*hardware=*/false);

  // If setting the breakpoint fails because pc is out of the address
  // space, ignore it and let the debugee segfault.
  if (error.GetError() == EIO || error.GetError() == EFAULT)
    return Status();
  if (error.Fail())
    return error;

  return Status();
}

Status NativeProcessSoftwareSingleStep::SetupSoftwareSingleStepping(
    NativeThreadProtocol &thread) {
  Status error;
  NativeProcessProtocol &process = thread.GetProcess();
  NativeRegisterContext &register_context = thread.GetRegisterContext();
  const ArchSpec &arch = process.GetArchitecture();

  std::unique_ptr<EmulateInstruction> emulator_up(
      EmulateInstruction::FindPlugin(arch, eInstructionTypePCModifying,
                                     nullptr));
  if (emulator_up == nullptr)
    return Status::FromErrorString("Instruction emulator not found!");

  EmulatorBaton baton(process, register_context);
  emulator_up->SetBaton(&baton);
  emulator_up->SetReadMemCallback(&ReadMemoryCallback);
  emulator_up->SetReadRegCallback(&ReadRegisterCallback);
  emulator_up->SetWriteMemCallback(&WriteMemoryCallback);
  emulator_up->SetWriteRegCallback(&WriteRegisterCallback);

  auto bp_locaions_predictor =
      EmulateInstruction::CreateBreakpointLocationPredictor(
          std::move(emulator_up));

  auto bp_locations = bp_locaions_predictor->GetBreakpointLocations(error);
  if (error.Fail())
    return error;

  for (auto &&bp_addr : bp_locations) {
    auto bp_size = bp_locaions_predictor->GetBreakpointSize(bp_addr);
    if (auto err = bp_size.takeError())
      return Status(toString(std::move(err)));

    error = SetSoftwareBreakpoint(bp_addr, *bp_size, process);
    if (error.Fail())
      return error;
  }

  m_threads_stepping_with_breakpoint.insert({thread.GetID(), bp_locations});
  return error;
}
