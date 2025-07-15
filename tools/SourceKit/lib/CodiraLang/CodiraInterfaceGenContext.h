//===--- CodiraInterfaceGenContext.h - ---------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_LANGUAGEINTERFACEGENCONTEXT_H
#define TOOLCHAIN_SOURCEKIT_LIB_LANGUAGELANG_LANGUAGEINTERFACEGENCONTEXT_H

#include "SourceKit/Core/Toolchain.h"
#include "language/AST/Module.h"
#include "language/Basic/ThreadSafeRefCounted.h"
#include <string>

namespace language {
  class CompilerInvocation;
  class ValueDecl;
  class ModuleDecl;
}

namespace SourceKit {
  class EditorConsumer;
  class CodiraInterfaceGenContext;
  typedef RefPtr<CodiraInterfaceGenContext> CodiraInterfaceGenContextRef;
  class ASTUnit;
  typedef IntrusiveRefCntPtr<ASTUnit> ASTUnitRef;

class CodiraInterfaceGenContext :
  public toolchain::ThreadSafeRefCountedBase<CodiraInterfaceGenContext> {
public:
  static CodiraInterfaceGenContextRef
  create(StringRef DocumentName, bool IsModule, StringRef ModuleOrHeaderName,
         std::optional<StringRef> Group, language::CompilerInvocation Invocation,
         std::string &ErrorMsg, bool SynthesizedExtensions,
         std::optional<StringRef> InterestedUSR);

  static CodiraInterfaceGenContextRef
    createForTypeInterface(language::CompilerInvocation Invocation,
                           StringRef TypeUSR,
                           std::string &ErrorMsg);

  static CodiraInterfaceGenContextRef createForCodiraSource(StringRef DocumentName,
                                                          StringRef SourceFileName,
                                                          ASTUnitRef AstUnit,
                                                          language::CompilerInvocation Invocation,
                                                          std::string &ErrMsg);

  ~CodiraInterfaceGenContext();

  StringRef getDocumentName() const;
  StringRef getModuleOrHeaderName() const;
  bool isModule() const;
  language::ModuleDecl *getModuleDecl() const;

  bool matches(StringRef ModuleName, const language::CompilerInvocation &Invok);

  /// Note: requires exclusive access to the underlying AST.
  void reportEditorInfo(EditorConsumer &Consumer) const;

  struct ResolvedEntity {
    const language::ValueDecl *Dcl = nullptr;
    language::ModuleEntity Mod;
    bool IsRef = false;

    ResolvedEntity() = default;
    ResolvedEntity(const language::ValueDecl *Dcl, bool IsRef)
      : Dcl(Dcl), IsRef(IsRef) {}
    ResolvedEntity(const language::ModuleEntity Mod, bool IsRef)
      : Mod(Mod), IsRef(IsRef) {}

    bool isResolved() const { return Dcl || Mod; }
  };

  /// Provides exclusive access to the underlying AST.
  void accessASTAsync(std::function<void()> Fn);

  /// Returns the resolved entity along with a boolean indicating if it is a
  /// reference or not.
  /// Note: requires exclusive access to the underlying AST. See accessASTAsync.
  ResolvedEntity resolveEntityForOffset(unsigned Offset) const;

  /// Searches for a declaration with the given USR and returns the
  /// (offset,length) pair into the interface source if it finds one.
  std::optional<std::pair<unsigned, unsigned>>
  findUSRRange(StringRef USR) const;

  void applyTo(language::CompilerInvocation &CompInvok) const;

  class Implementation;

private:
  Implementation &Impl;

  CodiraInterfaceGenContext();
};

} // namespace SourceKit

#endif
