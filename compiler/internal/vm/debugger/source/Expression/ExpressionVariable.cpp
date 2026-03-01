//===-- ExpressionVariable.cpp --------------------------------------------===//
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

#include "lldb/Expression/ExpressionVariable.h"
#include "lldb/Expression/IRExecutionUnit.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"
#include <optional>

using namespace lldb_private;

char ExpressionVariable::ID;

ExpressionVariable::ExpressionVariable() : m_flags(0) {}

uint8_t *ExpressionVariable::GetValueBytes() {
  std::optional<uint64_t> byte_size =
      llvm::expectedToOptional(m_frozen_sp->GetByteSize());
  if (byte_size && *byte_size) {
    if (m_frozen_sp->GetDataExtractor().GetByteSize() < *byte_size) {
      m_frozen_sp->GetValue().ResizeData(*byte_size);
      m_frozen_sp->GetValue().GetData(m_frozen_sp->GetDataExtractor());
    }
    return const_cast<uint8_t *>(
        m_frozen_sp->GetDataExtractor().GetDataStart());
  }
  return nullptr;
}

char PersistentExpressionState::ID;

PersistentExpressionState::PersistentExpressionState() = default;

PersistentExpressionState::~PersistentExpressionState() = default;

lldb::addr_t PersistentExpressionState::LookupSymbol(ConstString name) {
  SymbolMap::iterator si = m_symbol_map.find(name.GetCString());

  if (si != m_symbol_map.end())
    return si->second;
  else
    return LLDB_INVALID_ADDRESS;
}

void PersistentExpressionState::RegisterExecutionUnit(
    lldb::IRExecutionUnitSP &execution_unit_sp) {
  Log *log = GetLog(LLDBLog::Expressions);

  m_execution_units.insert(execution_unit_sp);

  LLDB_LOGF(log, "Registering JITted Functions:\n");

  for (const IRExecutionUnit::JittedFunction &jitted_function :
       execution_unit_sp->GetJittedFunctions()) {
    if (jitted_function.m_external &&
        jitted_function.m_name != execution_unit_sp->GetFunctionName() &&
        jitted_function.m_remote_addr != LLDB_INVALID_ADDRESS) {
      m_symbol_map[jitted_function.m_name.GetCString()] =
          jitted_function.m_remote_addr;
      LLDB_LOGF(log, "  Function: %s at 0x%" PRIx64 ".",
                jitted_function.m_name.GetCString(),
                jitted_function.m_remote_addr);
    }
  }

  LLDB_LOGF(log, "Registering JIIted Symbols:\n");

  for (const IRExecutionUnit::JittedGlobalVariable &global_var :
       execution_unit_sp->GetJittedGlobalVariables()) {
    if (global_var.m_remote_addr != LLDB_INVALID_ADDRESS) {
      // Demangle the name before inserting it, so that lookups by the ConstStr
      // of the demangled name will find the mangled one (needed for looking up
      // metadata pointers.)
      Mangled mangler(global_var.m_name);
      mangler.GetDemangledName();
      m_symbol_map[global_var.m_name.GetCString()] = global_var.m_remote_addr;
      LLDB_LOGF(log, "  Symbol: %s at 0x%" PRIx64 ".",
                global_var.m_name.GetCString(), global_var.m_remote_addr);
    }
  }
}
