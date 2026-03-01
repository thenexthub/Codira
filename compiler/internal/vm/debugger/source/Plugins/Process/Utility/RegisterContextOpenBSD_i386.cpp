//===-- RegisterContextOpenBSD_i386.cpp -----------------------------------===//
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
//===---------------------------------------------------------------------===//

#include "RegisterContextOpenBSD_i386.h"
#include "RegisterContextPOSIX_x86.h"

using namespace lldb_private;
using namespace lldb;

// /usr/include/machine/reg.h
struct GPR {
  uint32_t eax;
  uint32_t ecx;
  uint32_t edx;
  uint32_t ebx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint32_t eip;
  uint32_t eflags;
  uint32_t cs;
  uint32_t ss;
  uint32_t ds;
  uint32_t es;
  uint32_t fs;
  uint32_t gs;
};

struct dbreg {
  uint32_t dr[8]; /* debug registers */
                  /* Index 0-3: debug address registers */
                  /* Index 4-5: reserved */
                  /* Index 6: debug status */
                  /* Index 7: debug control */
};

using FPR_i386 = FXSAVE;

struct UserArea {
  GPR gpr;
  FPR_i386 i387;
};

#define DR_SIZE sizeof(uint32_t)
#define DR_OFFSET(reg_index) (LLVM_EXTENSION offsetof(dbreg, dr[reg_index]))

// Include RegisterInfos_i386 to declare our g_register_infos_i386 structure.
#define DECLARE_REGISTER_INFOS_I386_STRUCT
#include "RegisterInfos_i386.h"
#undef DECLARE_REGISTER_INFOS_I386_STRUCT

RegisterContextOpenBSD_i386::RegisterContextOpenBSD_i386(
    const ArchSpec &target_arch)
    : RegisterInfoInterface(target_arch) {}

size_t RegisterContextOpenBSD_i386::GetGPRSize() const { return sizeof(GPR); }

const RegisterInfo *RegisterContextOpenBSD_i386::GetRegisterInfo() const {
  switch (GetTargetArchitecture().GetMachine()) {
  case llvm::Triple::x86:
    return g_register_infos_i386;
  default:
    assert(false && "Unhandled target architecture.");
    return nullptr;
  }
}

uint32_t RegisterContextOpenBSD_i386::GetRegisterCount() const {
  return static_cast<uint32_t>(sizeof(g_register_infos_i386) /
                               sizeof(g_register_infos_i386[0]));
}
