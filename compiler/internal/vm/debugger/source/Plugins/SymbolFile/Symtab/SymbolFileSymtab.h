//===-- SymbolFileSymtab.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_SYMTAB_SYMBOLFILESYMTAB_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_SYMTAB_SYMBOLFILESYMTAB_H

#include <map>
#include <optional>
#include <vector>

#include "lldb/Symbol/SymbolFile.h"
#include "lldb/Symbol/Symtab.h"

class SymbolFileSymtab : public lldb_private::SymbolFileCommon {
  /// LLVM RTTI support.
  static char ID;

public:
  /// LLVM RTTI support.
  /// \{
  bool isA(const void *ClassID) const override {
    return ClassID == &ID || SymbolFileCommon::isA(ClassID);
  }
  static bool classof(const SymbolFile *obj) { return obj->isA(&ID); }
  /// \}

  // Constructors and Destructors
  SymbolFileSymtab(lldb::ObjectFileSP objfile_sp);

  // Static Functions
  static void Initialize();

  static void Terminate();

  static llvm::StringRef GetPluginNameStatic() { return "symtab"; }

  static llvm::StringRef GetPluginDescriptionStatic();

  static lldb_private::SymbolFile *
  CreateInstance(lldb::ObjectFileSP objfile_sp);

  uint32_t CalculateAbilities() override;

  // Compile Unit function calls
  lldb::LanguageType
  ParseLanguage(lldb_private::CompileUnit &comp_unit) override;

  size_t ParseFunctions(lldb_private::CompileUnit &comp_unit) override;

  bool ParseLineTable(lldb_private::CompileUnit &comp_unit) override;

  bool ParseDebugMacros(lldb_private::CompileUnit &comp_unit) override;

  bool ParseSupportFiles(lldb_private::CompileUnit &comp_unit,
                         lldb_private::SupportFileList &support_files) override;

  size_t ParseTypes(lldb_private::CompileUnit &comp_unit) override;

  bool ParseImportedModules(
      const lldb_private::SymbolContext &sc,
      std::vector<lldb_private::SourceModule> &imported_modules) override;

  size_t ParseBlocksRecursive(lldb_private::Function &func) override;

  size_t
  ParseVariablesForContext(const lldb_private::SymbolContext &sc) override;

  lldb_private::Type *ResolveTypeUID(lldb::user_id_t type_uid) override;
  std::optional<ArrayInfo> GetDynamicArrayInfoForUID(
      lldb::user_id_t type_uid,
      const lldb_private::ExecutionContext *exe_ctx) override;

  bool CompleteType(lldb_private::CompilerType &compiler_type) override;

  uint32_t ResolveSymbolContext(const lldb_private::Address &so_addr,
                                lldb::SymbolContextItem resolve_scope,
                                lldb_private::SymbolContext &sc) override;

  void GetTypes(lldb_private::SymbolContextScope *sc_scope,
                lldb::TypeClass type_mask,
                lldb_private::TypeList &type_list) override;

  // PluginInterface protocol
  llvm::StringRef GetPluginName() override { return GetPluginNameStatic(); }

protected:
  uint32_t CalculateNumCompileUnits() override;

  lldb::CompUnitSP ParseCompileUnitAtIndex(uint32_t index) override;

  lldb_private::Symtab::IndexCollection m_source_indexes;
  lldb_private::Symtab::IndexCollection m_func_indexes;
  lldb_private::Symtab::IndexCollection m_code_indexes;
  lldb_private::Symtab::IndexCollection m_data_indexes;
  lldb_private::Symtab::NameToIndexMap m_objc_class_name_to_index;
};

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_SYMTAB_SYMBOLFILESYMTAB_H
