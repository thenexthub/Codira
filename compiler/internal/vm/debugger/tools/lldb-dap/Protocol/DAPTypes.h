//===-- ProtocolTypes.h ---------------------------------------------------===//
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
// This file contains private DAP types used in the protocol.
//
// Each struct has a toJSON and fromJSON function, that converts between
// the struct and a JSON representation. (See JSON.h)
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_LLDB_DAP_PROTOCOL_DAP_TYPES_H
#define LLDB_TOOLS_LLDB_DAP_PROTOCOL_DAP_TYPES_H

#include "lldb/lldb-defines.h"
#include "lldb/lldb-types.h"
#include "llvm/Support/JSON.h"
#include <optional>
#include <string>

namespace lldb_dap::protocol {

/// Data used to help lldb-dap resolve breakpoints persistently across different
/// sessions. This information is especially useful for assembly breakpoints,
/// because `sourceReference` can change across sessions. For regular source
/// breakpoints the path and line are the same For each session.
struct PersistenceData {
  /// The source module path.
  std::string module_path;

  /// The symbol name of the Source.
  std::string symbol_name;
};
bool fromJSON(const llvm::json::Value &, PersistenceData &, llvm::json::Path);
llvm::json::Value toJSON(const PersistenceData &);

/// Custom source data used by lldb-dap.
/// This data should help lldb-dap identify sources correctly across different
/// sessions.
struct SourceLLDBData {
  /// Data that helps lldb resolve this source persistently across different
  /// sessions.
  std::optional<PersistenceData> persistenceData;
};
bool fromJSON(const llvm::json::Value &, SourceLLDBData &, llvm::json::Path);
llvm::json::Value toJSON(const SourceLLDBData &);

struct Symbol {
  /// The symbol id, usually the original symbol table index.
  uint32_t id = 0;

  /// True if this symbol is debug information in a symbol.
  bool isDebug = false;

  /// True if this symbol is not actually in the symbol table, but synthesized
  /// from other info in the object file.
  bool isSynthetic = false;

  /// True if this symbol is globally visible.
  bool isExternal = false;

  /// The symbol type.
  lldb::SymbolType type = lldb::eSymbolTypeInvalid;

  /// The symbol file address.
  lldb::addr_t fileAddress = LLDB_INVALID_ADDRESS;

  /// The symbol load address.
  std::optional<lldb::addr_t> loadAddress;

  /// The symbol size.
  lldb::addr_t size = 0;

  /// The symbol name.
  std::string name;
};
bool fromJSON(const llvm::json::Value &, Symbol &, llvm::json::Path);
llvm::json::Value toJSON(const Symbol &);

} // namespace lldb_dap::protocol

#endif
