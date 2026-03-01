//===-- SymbolLocatorDebuginfod.h -------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLLOCATOR_DEBUGINFOD_SYMBOLLOCATORDEBUGINFOD_H
#define LLDB_SOURCE_PLUGINS_SYMBOLLOCATOR_DEBUGINFOD_SYMBOLLOCATORDEBUGINFOD_H

#include "lldb/Core/Debugger.h"
#include "lldb/Symbol/SymbolLocator.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class SymbolLocatorDebuginfod : public SymbolLocator {
public:
  SymbolLocatorDebuginfod();

  static void Initialize();
  static void Terminate();
  static void DebuggerInitialize(Debugger &debugger);

  static llvm::StringRef GetPluginNameStatic() { return "debuginfod"; }
  static llvm::StringRef GetPluginDescriptionStatic();

  static lldb_private::SymbolLocator *CreateInstance();

  /// PluginInterface protocol.
  /// \{
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }
  /// \}

  // Locate the executable file given a module specification.
  //
  // Locating the file should happen only on the local computer or using the
  // current computers global settings.
  static std::optional<ModuleSpec>
  LocateExecutableObjectFile(const ModuleSpec &module_spec);

  // Locate the symbol file given a module specification.
  //
  // Locating the file should happen only on the local computer or using the
  // current computers global settings.
  static std::optional<FileSpec>
  LocateExecutableSymbolFile(const ModuleSpec &module_spec,
                             const FileSpecList &default_search_paths);
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SYMBOLLOCATOR_DEBUGINFOD_SYMBOLLOCATORDEBUGINFOD_H
