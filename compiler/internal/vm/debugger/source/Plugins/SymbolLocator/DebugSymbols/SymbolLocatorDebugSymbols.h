//===-- SymbolLocatorDebugSymbols.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLLOCATOR_DEBUGSYMBOLS_SYMBOLLOCATORDEBUGSYMBOLS_H
#define LLDB_SOURCE_PLUGINS_SYMBOLLOCATOR_DEBUGSYMBOLS_SYMBOLLOCATORDEBUGSYMBOLS_H

#include "lldb/Symbol/SymbolLocator.h"
#include "lldb/lldb-private.h"

namespace lldb_private {

class SymbolLocatorDebugSymbols : public SymbolLocator {
public:
  SymbolLocatorDebugSymbols();

  static void Initialize();
  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "DebugSymbols"; }
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

  // Locate the object and symbol file given a module specification.
  //
  // Locating the file can try to download the file from a corporate build
  // repository, or using any other means necessary to locate both the
  // unstripped object file and the debug symbols. The force_lookup argument
  // controls whether the external program is called unconditionally to find
  // the symbol file, or if the user's settings are checked to see if they've
  // enabled the external program before calling.
  static bool DownloadObjectAndSymbolFile(ModuleSpec &module_spec,
                                          Status &error, bool force_lookup,
                                          bool copy_executable);

  static std::optional<FileSpec>
  FindSymbolFileInBundle(const FileSpec &dsym_bundle_fspec, const UUID *uuid,
                         const ArchSpec *arch);
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_SYMBOLLOCATOR_DEBUGSYMBOLS_SYMBOLLOCATORDEBUGSYMBOLS_H
