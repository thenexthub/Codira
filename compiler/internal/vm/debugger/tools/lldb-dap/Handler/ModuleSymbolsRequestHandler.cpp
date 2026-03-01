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

#include "DAP.h"
#include "DAPError.h"
#include "Protocol/DAPTypes.h"
#include "RequestHandler.h"
#include "lldb/API/SBAddress.h"
#include "lldb/API/SBFileSpec.h"
#include "lldb/API/SBModule.h"
#include "lldb/API/SBModuleSpec.h"
#include "lldb/Utility/UUID.h"
#include "llvm/Support/Error.h"
#include <cstddef>

using namespace lldb_dap::protocol;
namespace lldb_dap {

llvm::Expected<ModuleSymbolsResponseBody>
ModuleSymbolsRequestHandler::Run(const ModuleSymbolsArguments &args) const {
  ModuleSymbolsResponseBody response;

  lldb::SBModuleSpec module_spec;
  if (!args.moduleId.empty()) {
    llvm::SmallVector<uint8_t, 20> uuid_bytes;
    if (!lldb_private::UUID::DecodeUUIDBytesFromString(args.moduleId,
                                                       uuid_bytes)
             .empty())
      return llvm::make_error<DAPError>("invalid module ID");

    module_spec.SetUUIDBytes(uuid_bytes.data(), uuid_bytes.size());
  }

  if (!args.moduleName.empty()) {
    lldb::SBFileSpec file_spec;
    file_spec.SetFilename(args.moduleName.c_str());
    module_spec.SetFileSpec(file_spec);
  }

  // Empty request, return empty response.
  if (!module_spec.IsValid())
    return response;

  std::vector<Symbol> &symbols = response.symbols;
  lldb::SBModule module = dap.target.FindModule(module_spec);
  if (!module.IsValid())
    return llvm::make_error<DAPError>("module not found");

  const size_t num_symbols = module.GetNumSymbols();
  const size_t start_index = args.startIndex.value_or(0);
  const size_t end_index =
      std::min(start_index + args.count.value_or(num_symbols), num_symbols);
  for (size_t i = start_index; i < end_index; ++i) {
    lldb::SBSymbol symbol = module.GetSymbolAtIndex(i);
    if (!symbol.IsValid())
      continue;

    Symbol dap_symbol;
    dap_symbol.id = symbol.GetID();
    dap_symbol.type = symbol.GetType();
    dap_symbol.isDebug = symbol.IsDebug();
    dap_symbol.isSynthetic = symbol.IsSynthetic();
    dap_symbol.isExternal = symbol.IsExternal();

    lldb::SBAddress start_address = symbol.GetStartAddress();
    if (start_address.IsValid()) {
      lldb::addr_t file_address = start_address.GetFileAddress();
      if (file_address != LLDB_INVALID_ADDRESS)
        dap_symbol.fileAddress = file_address;

      lldb::addr_t load_address = start_address.GetLoadAddress(dap.target);
      if (load_address != LLDB_INVALID_ADDRESS)
        dap_symbol.loadAddress = load_address;
    }

    dap_symbol.size = symbol.GetSize();
    if (const char *symbol_name = symbol.GetName())
      dap_symbol.name = symbol_name;
    symbols.push_back(std::move(dap_symbol));
  }

  return response;
}

} // namespace lldb_dap
