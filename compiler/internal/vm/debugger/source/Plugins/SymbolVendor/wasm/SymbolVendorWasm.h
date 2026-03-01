//===-- SymbolVendorWasm.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLVENDOR_WASM_SYMBOLVENDORWASM_H
#define LLDB_SOURCE_PLUGINS_SYMBOLVENDOR_WASM_SYMBOLVENDORWASM_H

#include "lldb/Symbol/SymbolVendor.h"
#include "lldb/lldb-private.h"

namespace lldb_private {
namespace wasm {

class SymbolVendorWasm : public lldb_private::SymbolVendor {
public:
  SymbolVendorWasm(const lldb::ModuleSP &module_sp);

  static void Initialize();
  static void Terminate();
  static llvm::StringRef GetPluginNameStatic() { return "WASM"; }
  static llvm::StringRef GetPluginDescriptionStatic();

  static lldb_private::SymbolVendor *
  CreateInstance(const lldb::ModuleSP &module_sp,
                 lldb_private::Stream *feedback_strm);

  /// PluginInterface protocol.
  /// \{
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
  /// \}
};

} // namespace wasm
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SYMBOLVENDOR_WASM_SYMBOLVENDORWASM_H
