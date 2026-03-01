//===-- DNBArch.cpp ---------------------------------------------*- C++ -*-===//
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
//  Created by Greg Clayton on 6/24/07.
//
//===----------------------------------------------------------------------===//

#include "DNBArch.h"
#include <cassert>
#include <mach/mach.h>

#include <map>

#include "DNBLog.h"

typedef std::map<uint32_t, DNBArchPluginInfo> CPUPluginInfoMap;

static uint32_t g_current_cpu_type = 0;
static uint32_t g_current_cpu_subtype = 0;
CPUPluginInfoMap g_arch_plugins;

static const DNBArchPluginInfo *GetArchInfo() {
  CPUPluginInfoMap::const_iterator pos =
      g_arch_plugins.find(g_current_cpu_type);
  if (pos != g_arch_plugins.end())
    return &pos->second;
  return NULL;
}

uint32_t DNBArchProtocol::GetCPUType() { return g_current_cpu_type; }
uint32_t DNBArchProtocol::GetCPUSubType() { return g_current_cpu_subtype; }

bool DNBArchProtocol::SetArchitecture(uint32_t cpu_type, uint32_t cpu_subtype) {
  g_current_cpu_type = cpu_type;
  g_current_cpu_subtype = cpu_subtype;
  bool result = g_arch_plugins.find(g_current_cpu_type) != g_arch_plugins.end();
  DNBLogThreadedIf(LOG_PROCESS,
                   "DNBArchProtocol::SetDefaultArchitecture (cpu_type=0x%8.8x, "
                   "cpu_subtype=0x%8.8x) => %i",
                   cpu_type, cpu_subtype, result);
  return result;
}

void DNBArchProtocol::RegisterArchPlugin(const DNBArchPluginInfo &arch_info) {
  if (arch_info.cpu_type)
    g_arch_plugins[arch_info.cpu_type] = arch_info;
}

uint32_t DNBArchProtocol::GetRegisterCPUType() {
  const DNBArchPluginInfo *arch_info = GetArchInfo();
  if (arch_info)
    return arch_info->cpu_type;
  return 0;
}

const DNBRegisterSetInfo *
DNBArchProtocol::GetRegisterSetInfo(nub_size_t *num_reg_sets) {
  const DNBArchPluginInfo *arch_info = GetArchInfo();
  if (arch_info)
    return arch_info->GetRegisterSetInfo(num_reg_sets);
  *num_reg_sets = 0;
  return NULL;
}

DNBArchProtocol *DNBArchProtocol::Create(MachThread *thread) {
  const DNBArchPluginInfo *arch_info = GetArchInfo();
  if (arch_info)
    return arch_info->Create(thread);
  return NULL;
}

const uint8_t *DNBArchProtocol::GetBreakpointOpcode(nub_size_t byte_size) {
  const DNBArchPluginInfo *arch_info = GetArchInfo();
  if (arch_info)
    return arch_info->GetBreakpointOpcode(byte_size);
  return NULL;
}
