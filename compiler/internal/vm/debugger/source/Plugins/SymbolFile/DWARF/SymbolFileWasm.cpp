//===----------------------------------------------------------------------===//
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

#include "SymbolFileWasm.h"
#include "Plugins/SymbolFile/DWARF/LogChannelDWARF.h"
#include "Utility/WasmVirtualRegisters.h"
#include "lldb/Utility/LLDBLog.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::plugin::dwarf;

SymbolFileWasm::SymbolFileWasm(ObjectFileSP objfile_sp,
                               SectionList *dwo_section_list)
    : SymbolFileDWARF(objfile_sp, dwo_section_list) {}

SymbolFileWasm::~SymbolFileWasm() = default;

lldb::offset_t
SymbolFileWasm::GetVendorDWARFOpcodeSize(const DataExtractor &data,
                                         const lldb::offset_t data_offset,
                                         const uint8_t op) const {
  if (op != llvm::dwarf::DW_OP_WASM_location)
    return LLDB_INVALID_OFFSET;

  lldb::offset_t offset = data_offset;
  const uint8_t wasm_op = data.GetU8(&offset);
  switch (wasm_op) {
  case 0: // LOCAL
  case 1: // GLOBAL_FIXED
  case 2: // OPERAND_STACK
    data.GetULEB128(&offset);
    break;
  case 3: // GLOBAL_RELOC
    data.GetU32(&offset);
    break;
  default:
    return LLDB_INVALID_OFFSET;
  }

  return offset - data_offset;
}

bool SymbolFileWasm::ParseVendorDWARFOpcode(uint8_t op,
                                            const DataExtractor &opcodes,
                                            lldb::offset_t &offset,
                                            RegisterContext *reg_ctx,
                                            lldb::RegisterKind reg_kind,
                                            std::vector<Value> &stack) const {
  if (op != llvm::dwarf::DW_OP_WASM_location)
    return false;

  uint32_t index = 0;
  uint8_t tag = eWasmTagNotAWasmLocation;

  /// |DWARF Location Index | WebAssembly Construct |
  /// |---------------------|-----------------------|
  /// |0                    | Local                 |
  /// |1 or 3               | Global                |
  /// |2                    | Operand Stack         |
  const uint8_t wasm_op = opcodes.GetU8(&offset);
  switch (wasm_op) {
  case 0: // LOCAL
    index = opcodes.GetULEB128(&offset);
    tag = eWasmTagLocal;
    break;
  case 1: // GLOBAL_FIXED
    index = opcodes.GetULEB128(&offset);
    tag = eWasmTagGlobal;
    break;
  case 2: // OPERAND_STACK
    index = opcodes.GetULEB128(&offset);
    tag = eWasmTagOperandStack;
    break;
  case 3: // GLOBAL_RELOC
    index = opcodes.GetU32(&offset);
    tag = eWasmTagGlobal;
    break;
  default:
    return false;
  }

  const uint32_t reg_num = GetWasmRegister(tag, index);

  Value tmp;
  llvm::Error error = DWARFExpression::ReadRegisterValueAsScalar(
      reg_ctx, reg_kind, reg_num, tmp);
  if (error) {
    LLDB_LOG_ERROR(GetLog(DWARFLog::DebugInfo), std::move(error), "{0}");
    return false;
  }

  stack.push_back(tmp);
  return true;
}
