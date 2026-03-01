//===-- ArchitecturePPC64.cpp ---------------------------------------------===//
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

#include "Plugins/Architecture/PPC64/ArchitecturePPC64.h"
#include "lldb/Core/PluginManager.h"
#include "lldb/Symbol/Function.h"
#include "lldb/Symbol/Symbol.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Target/Target.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/ArchSpec.h"

#include "llvm/BinaryFormat/ELF.h"

using namespace lldb_private;
using namespace lldb;

LLDB_PLUGIN_DEFINE(ArchitecturePPC64)

void ArchitecturePPC64::Initialize() {
  PluginManager::RegisterPlugin(GetPluginNameStatic(),
                                "PPC64-specific algorithms",
                                &ArchitecturePPC64::Create);
}

void ArchitecturePPC64::Terminate() {
  PluginManager::UnregisterPlugin(&ArchitecturePPC64::Create);
}

std::unique_ptr<Architecture> ArchitecturePPC64::Create(const ArchSpec &arch) {
  if (arch.GetTriple().isPPC64() &&
      arch.GetTriple().getObjectFormat() == llvm::Triple::ObjectFormatType::ELF)
    return std::unique_ptr<Architecture>(new ArchitecturePPC64());
  return nullptr;
}

static int32_t GetLocalEntryOffset(const Symbol &sym) {
  unsigned char other = sym.GetFlags() >> 8 & 0xFF;
  return llvm::ELF::decodePPC64LocalEntryOffset(other);
}

size_t ArchitecturePPC64::GetBytesToSkip(Symbol &func,
                                         const Address &curr_addr) const {
  if (curr_addr.GetFileAddress() ==
      func.GetFileAddress() + GetLocalEntryOffset(func))
    return func.GetPrologueByteSize();
  return 0;
}

void ArchitecturePPC64::AdjustBreakpointAddress(const Symbol &func,
                                                Address &addr) const {
  int32_t loffs = GetLocalEntryOffset(func);
  if (!loffs)
    return;

  addr.SetOffset(addr.GetOffset() + loffs);
}
