//===-- DWARFASTParser.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFASTPARSER_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFASTPARSER_H

#include "DWARFDefines.h"
#include "lldb/Core/PluginInterface.h"
#include "lldb/Symbol/SymbolFile.h"
#include "lldb/Symbol/CompilerDecl.h"
#include "lldb/Symbol/CompilerDeclContext.h"
#include "lldb/lldb-enumerations.h"
#include <optional>

namespace lldb_private {
class CompileUnit;
class ExecutionContext;
}

namespace lldb_private::plugin {
namespace dwarf {
class DWARFDIE;
class SymbolFileDWARF;

class DWARFASTParser {
public:
  enum class Kind { DWARFASTParserClang };
  DWARFASTParser(Kind kind) : m_kind(kind) {}

  virtual ~DWARFASTParser() = default;

  virtual lldb::TypeSP ParseTypeFromDWARF(const SymbolContext &sc,
                                          const DWARFDIE &die,
                                          bool *type_is_new_ptr) = 0;

  virtual ConstString ConstructDemangledNameFromDWARF(const DWARFDIE &die) = 0;

  virtual Function *ParseFunctionFromDWARF(CompileUnit &comp_unit,
                                           const DWARFDIE &die,
                                           AddressRanges ranges) = 0;

  virtual bool CompleteTypeFromDWARF(const DWARFDIE &die, Type *type,
                                     const CompilerType &compiler_type) = 0;

  virtual CompilerDecl GetDeclForUIDFromDWARF(const DWARFDIE &die) = 0;

  virtual CompilerDeclContext
  GetDeclContextForUIDFromDWARF(const DWARFDIE &die) = 0;

  virtual CompilerDeclContext
  GetDeclContextContainingUIDFromDWARF(const DWARFDIE &die) = 0;

  virtual void EnsureAllDIEsInDeclContextHaveBeenParsed(
      CompilerDeclContext decl_context) = 0;

  virtual std::string GetDIEClassTemplateParams(DWARFDIE die) = 0;

  static std::optional<SymbolFile::ArrayInfo>
  ParseChildArrayInfo(const DWARFDIE &parent_die,
                      const ExecutionContext *exe_ctx = nullptr);

  lldb_private::Type *GetTypeForDIE(const DWARFDIE &die);

  static lldb::AccessType GetAccessTypeFromDWARF(uint32_t dwarf_accessibility);

  Kind GetKind() const { return m_kind; }

private:
  const Kind m_kind;
};
} // namespace dwarf
} // namespace lldb_private::plugin

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_DWARF_DWARFASTPARSER_H
