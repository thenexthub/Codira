//===--- SwiftInterfaceGenContext.h - ---------------------------*- C++ -*-===//
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

#ifndef LLVM_SOURCEKIT_LIB_SWIFTLANG_SWIFTINTERFACEGENCONTEXT_H
#define LLVM_SOURCEKIT_LIB_SWIFTLANG_SWIFTINTERFACEGENCONTEXT_H

#include "SourceKit/Core/LLVM.h"
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
  class SwiftInterfaceGenContext;
  typedef RefPtr<SwiftInterfaceGenContext> SwiftInterfaceGenContextRef;
  class ASTUnit;
  typedef IntrusiveRefCntPtr<ASTUnit> ASTUnitRef;

class SwiftInterfaceGenContext :
  public llvm::ThreadSafeRefCountedBase<SwiftInterfaceGenContext> {
public:
  static SwiftInterfaceGenContextRef
  create(StringRef DocumentName, bool IsModule, StringRef ModuleOrHeaderName,
         std::optional<StringRef> Group, swift::CompilerInvocation Invocation,
         std::string &ErrorMsg, bool SynthesizedExtensions,
         std::optional<StringRef> InterestedUSR);

  static SwiftInterfaceGenContextRef
    createForTypeInterface(swift::CompilerInvocation Invocation,
                           StringRef TypeUSR,
                           std::string &ErrorMsg);

  static SwiftInterfaceGenContextRef createForSwiftSource(StringRef DocumentName,
                                                          StringRef SourceFileName,
                                                          ASTUnitRef AstUnit,
                                                          swift::CompilerInvocation Invocation,
                                                          std::string &ErrMsg);

  ~SwiftInterfaceGenContext();

  StringRef getDocumentName() const;
  StringRef getModuleOrHeaderName() const;
  bool isModule() const;
  swift::ModuleDecl *getModuleDecl() const;

  bool matches(StringRef ModuleName, const swift::CompilerInvocation &Invok);

  /// Note: requires exclusive access to the underlying AST.
  void reportEditorInfo(EditorConsumer &Consumer) const;

  struct ResolvedEntity {
    const swift::ValueDecl *Dcl = nullptr;
    swift::ModuleEntity Mod;
    bool IsRef = false;

    ResolvedEntity() = default;
    ResolvedEntity(const swift::ValueDecl *Dcl, bool IsRef)
      : Dcl(Dcl), IsRef(IsRef) {}
    ResolvedEntity(const swift::ModuleEntity Mod, bool IsRef)
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

  void applyTo(swift::CompilerInvocation &CompInvok) const;

  class Implementation;

private:
  Implementation &Impl;

  SwiftInterfaceGenContext();
};

} // namespace SourceKit

#endif
