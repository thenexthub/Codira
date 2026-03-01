//===-- PDBASTParser.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_SYMBOLFILE_PDB_PDBASTPARSER_H
#define LLDB_SOURCE_PLUGINS_SYMBOLFILE_PDB_PDBASTPARSER_H

#include "lldb/lldb-forward.h"

#include "Plugins/ExpressionParser/Clang/ClangASTImporter.h"

class SymbolFilePDB;

namespace clang {
class CharUnits;
class CXXRecordDecl;
class FieldDecl;
class RecordDecl;
} // namespace clang

namespace lldb_private {
class TypeSystemClang;
class CompilerType;
} // namespace lldb_private

namespace llvm {
namespace pdb {
template <typename ChildType> class ConcreteSymbolEnumerator;

class PDBSymbol;
class PDBSymbolData;
class PDBSymbolFunc;
class PDBSymbolTypeBaseClass;
class PDBSymbolTypeBuiltin;
class PDBSymbolTypeUDT;
} // namespace pdb
} // namespace llvm

class PDBASTParser {
public:
  PDBASTParser(lldb_private::TypeSystemClang &ast);
  ~PDBASTParser();

  lldb::TypeSP CreateLLDBTypeFromPDBType(const llvm::pdb::PDBSymbol &type);
  bool CompleteTypeFromPDB(lldb_private::CompilerType &compiler_type);

  clang::Decl *GetDeclForSymbol(const llvm::pdb::PDBSymbol &symbol);

  clang::DeclContext *
  GetDeclContextForSymbol(const llvm::pdb::PDBSymbol &symbol);
  clang::DeclContext *
  GetDeclContextContainingSymbol(const llvm::pdb::PDBSymbol &symbol);

  void ParseDeclsForDeclContext(const clang::DeclContext *decl_context);

  clang::NamespaceDecl *FindNamespaceDecl(const clang::DeclContext *parent,
                                          llvm::StringRef name);

  lldb_private::ClangASTImporter &GetClangASTImporter() {
    return m_ast_importer;
  }

private:
  typedef llvm::DenseMap<clang::CXXRecordDecl *, lldb::user_id_t>
      CXXRecordDeclToUidMap;
  typedef llvm::DenseMap<lldb::user_id_t, clang::Decl *> UidToDeclMap;
  typedef std::set<clang::NamespaceDecl *> NamespacesSet;
  typedef llvm::DenseMap<clang::DeclContext *, NamespacesSet>
      ParentToNamespacesMap;
  typedef llvm::DenseMap<clang::DeclContext *, lldb::user_id_t>
      DeclContextToUidMap;
  typedef llvm::pdb::ConcreteSymbolEnumerator<llvm::pdb::PDBSymbolData>
      PDBDataSymbolEnumerator;
  typedef llvm::pdb::ConcreteSymbolEnumerator<llvm::pdb::PDBSymbolTypeBaseClass>
      PDBBaseClassSymbolEnumerator;
  typedef llvm::pdb::ConcreteSymbolEnumerator<llvm::pdb::PDBSymbolFunc>
      PDBFuncSymbolEnumerator;

  bool AddEnumValue(lldb_private::CompilerType enum_type,
                    const llvm::pdb::PDBSymbolData &data);
  bool CompleteTypeFromUDT(lldb_private::SymbolFile &symbol_file,
                           lldb_private::CompilerType &compiler_type,
                           llvm::pdb::PDBSymbolTypeUDT &udt);
  void
  AddRecordMembers(lldb_private::SymbolFile &symbol_file,
                   lldb_private::CompilerType &record_type,
                   PDBDataSymbolEnumerator &members_enum,
                   lldb_private::ClangASTImporter::LayoutInfo &layout_info);
  void
  AddRecordBases(lldb_private::SymbolFile &symbol_file,
                 lldb_private::CompilerType &record_type, int record_kind,
                 PDBBaseClassSymbolEnumerator &bases_enum,
                 lldb_private::ClangASTImporter::LayoutInfo &layout_info) const;
  void AddRecordMethods(lldb_private::SymbolFile &symbol_file,
                        lldb_private::CompilerType &record_type,
                        PDBFuncSymbolEnumerator &methods_enum);
  clang::CXXMethodDecl *
  AddRecordMethod(lldb_private::SymbolFile &symbol_file,
                  lldb_private::CompilerType &record_type,
                  const llvm::pdb::PDBSymbolFunc &method) const;

  lldb_private::TypeSystemClang &m_ast;
  lldb_private::ClangASTImporter m_ast_importer;

  CXXRecordDeclToUidMap m_forward_decl_to_uid;
  UidToDeclMap m_uid_to_decl;
  ParentToNamespacesMap m_parent_to_namespaces;
  NamespacesSet m_namespaces;
  DeclContextToUidMap m_decl_context_to_uid;
};

#endif // LLDB_SOURCE_PLUGINS_SYMBOLFILE_PDB_PDBASTPARSER_H
