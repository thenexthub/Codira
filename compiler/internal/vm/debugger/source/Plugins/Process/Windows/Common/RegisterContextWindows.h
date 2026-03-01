//===-- RegisterContextWindows.h --------------------------------*- C++ -*-===//
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

#ifndef liblldb_RegisterContextWindows_H_
#define liblldb_RegisterContextWindows_H_

#include "lldb/Target/RegisterContext.h"
#include "lldb/lldb-forward.h"

namespace lldb_private {

class Thread;

class RegisterContextWindows : public lldb_private::RegisterContext {
public:
  // Constructors and Destructors
  RegisterContextWindows(Thread &thread, uint32_t concrete_frame_idx);

  virtual ~RegisterContextWindows();

  // Subclasses must override these functions
  void InvalidateAllRegisters() override;

  bool ReadAllRegisterValues(lldb::WritableDataBufferSP &data_sp) override;

  bool WriteAllRegisterValues(const lldb::DataBufferSP &data_sp) override;

  uint32_t ConvertRegisterKindToRegisterNumber(lldb::RegisterKind kind,
                                               uint32_t num) override;

  bool HardwareSingleStep(bool enable) override;

  static constexpr uint32_t GetNumHardwareBreakpointSlots() {
    return NUM_HARDWARE_BREAKPOINT_SLOTS;
  }

  bool AddHardwareBreakpoint(uint32_t slot, lldb::addr_t address, uint32_t size,
                             bool read, bool write);
  bool RemoveHardwareBreakpoint(uint32_t slot);

  uint32_t GetTriggeredHardwareBreakpointSlotId();

protected:
  static constexpr unsigned NUM_HARDWARE_BREAKPOINT_SLOTS = 4;

  virtual bool CacheAllRegisterValues();
  virtual bool ApplyAllRegisterValues();

  CONTEXT m_context;
  bool m_context_stale;
};
} // namespace lldb_private

#endif // #ifndef liblldb_RegisterContextWindows_H_
