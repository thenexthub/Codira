//===-- SymbolFileDWARFDwo.h ------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_SYMBOLFILEDWARFDWO_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_SYMBOLFILEDWARFDWO_H

#include "SymbolFileDWARF.h"
#include "lldb/lldb-private-enumerations.h"
#include <optional>

namespace lldb_private::plugin {
namespace dwarf {
class SymbolFileDWARFDwo : public SymbolFileDWARF {
  /// LLVM RTTI support.
  static char ID;

public:
  /// LLVM RTTI support.
  /// \{
  bool isA(const void *ClassID) const override {
    return ClassID == &ID || SymbolFileDWARF::isA(ClassID);
  }
  static bool classof(const SymbolFile *obj) { return obj->isA(&ID); }
  /// \}

  SymbolFileDWARFDwo(SymbolFileDWARF &m_base_symbol_file,
                     lldb::ObjectFileSP objfile, uint32_t id);

  ~SymbolFileDWARFDwo() override = default;

  DWARFCompileUnit *GetDWOCompileUnitForHash(uint64_t hash);

  void GetObjCMethods(
      ConstString class_name,
      llvm::function_ref<IterationAction(DWARFDIE die)> callback) override;

  llvm::Expected<lldb::TypeSystemSP>
  GetTypeSystemForLanguage(lldb::LanguageType language) override;

  DWARFDIE
  GetDIE(const DIERef &die_ref) override;

  lldb::offset_t GetVendorDWARFOpcodeSize(const DataExtractor &data,
                                          const lldb::offset_t data_offset,
                                          const uint8_t op) const override;

  uint64_t GetDebugInfoSize(bool load_all_debug_info = false) override;

  bool ParseVendorDWARFOpcode(uint8_t op, const DataExtractor &opcodes,
                              lldb::offset_t &offset, RegisterContext *reg_ctx,
                              lldb::RegisterKind reg_kind,
                              std::vector<Value> &stack) const override;

  void FindGlobalVariables(ConstString name,
                           const CompilerDeclContext &parent_decl_ctx,
                           uint32_t max_matches,
                           VariableList &variables) override;

  SymbolFileDWARF &GetBaseSymbolFile() const { return m_base_symbol_file; }

  bool GetDebugInfoIndexWasLoadedFromCache() const override;
  void SetDebugInfoIndexWasLoadedFromCache() override;
  bool GetDebugInfoIndexWasSavedToCache() const override;
  void SetDebugInfoIndexWasSavedToCache() override;
  bool GetDebugInfoHadFrameVariableErrors() const override;
  void SetDebugInfoHadFrameVariableErrors() override;

  SymbolFileDWARF *GetDIERefSymbolFile(const DIERef &die_ref) override;

protected:
  llvm::DenseMap<const DWARFDebugInfoEntry *, Type *> &GetDIEToType() override;

  DIEToVariableSP &GetDIEToVariable() override;

  llvm::DenseMap<lldb::opaque_compiler_type_t, DIERef> &
  GetForwardDeclCompilerTypeToDIE() override;

  UniqueDWARFASTTypeMap &GetUniqueDWARFASTTypeMap() override;

  DWARFDIE FindDefinitionDIE(const DWARFDIE &die) override;

  lldb::TypeSP
  FindCompleteObjCDefinitionTypeForDIE(const DWARFDIE &die,
                                       ConstString type_name,
                                       bool must_be_implementation) override;

  /// If this file contains exactly one compile unit, this function will return
  /// it. Otherwise it returns nullptr.
  DWARFCompileUnit *FindSingleCompileUnit();

  SymbolFileDWARF &m_base_symbol_file;
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_SYMBOLFILEDWARFDWO_H
