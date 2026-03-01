//===-- ClangExternalASTSourceCallbacks.h -----------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGEXTERNALASTSOURCECALLBACKS_H
#define LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGEXTERNALASTSOURCECALLBACKS_H

#include "Plugins/TypeSystem/Clang/TypeSystemClang.h"
#include "clang/Basic/ASTSourceDescriptor.h"
#include <optional>

namespace clang {

class Module;

} // namespace clang

namespace lldb_private {

class ClangExternalASTSourceCallbacks : public clang::ExternalASTSource {
  /// LLVM RTTI support.
  static char ID;

public:
  /// LLVM RTTI support.
  bool isA(const void *ClassID) const override { return ClassID == &ID; }
  static bool classof(const clang::ExternalASTSource *s) { return s->isA(&ID); }

  ClangExternalASTSourceCallbacks(TypeSystemClang &ast) : m_ast(ast) {}

  void FindExternalLexicalDecls(
      const clang::DeclContext *DC,
      llvm::function_ref<bool(clang::Decl::Kind)> IsKindWeWant,
      llvm::SmallVectorImpl<clang::Decl *> &Result) override;

  bool
  FindExternalVisibleDeclsByName(const clang::DeclContext *DC,
                                 clang::DeclarationName Name,
                                 const clang::DeclContext *OriginalDC) override;

  void CompleteType(clang::TagDecl *tag_decl) override;

  void CompleteType(clang::ObjCInterfaceDecl *objc_decl) override;

  bool layoutRecordType(
      const clang::RecordDecl *Record, uint64_t &Size, uint64_t &Alignment,
      llvm::DenseMap<const clang::FieldDecl *, uint64_t> &FieldOffsets,
      llvm::DenseMap<const clang::CXXRecordDecl *, clang::CharUnits>
          &BaseOffsets,
      llvm::DenseMap<const clang::CXXRecordDecl *, clang::CharUnits>
          &VirtualBaseOffsets) override;

  TypeSystemClang &GetTypeSystem() const { return m_ast; }

  /// Module-related methods.
  /// \{
  std::optional<clang::ASTSourceDescriptor>
  getSourceDescriptor(unsigned ID) override;
  clang::Module *getModule(unsigned ID) override;
  OptionalClangModuleID RegisterModule(clang::Module *module);
  OptionalClangModuleID GetIDForModule(clang::Module *module);
  /// \}
private:
  TypeSystemClang &m_ast;
  std::vector<clang::Module *> m_modules;
  llvm::DenseMap<clang::Module *, unsigned> m_ids;
};

} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_EXPRESSIONPARSER_CLANG_CLANGEXTERNALASTSOURCECALLBACKS_H
