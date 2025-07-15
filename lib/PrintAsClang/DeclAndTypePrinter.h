//===--- DeclAndTypePrinter.h - Emit ObjC decls from Codira AST --*- C++ -*-===//
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

#ifndef LANGUAGE_PRINTASCLANG_DECLANDTYPEPRINTER_H
#define LANGUAGE_PRINTASCLANG_DECLANDTYPEPRINTER_H

#include "OutputLanguageMode.h"

#include "language/AST/Decl.h"
#include "language/AST/Module.h"
#include "language/AST/Type.h"
// for OptionalTypeKind
#include "language/ClangImporter/ClangImporter.h"
#include "toolchain/ADT/StringSet.h"

namespace clang {
  class NamedDecl;
} // end namespace clang

namespace language {

class AccessorDecl;
class PrimitiveTypeMapping;
class ValueDecl;
class CodiraToClangInteropContext;

/// Tracks which C++ declarations have been emitted in a lexical
/// C++ scope.
struct CxxDeclEmissionScope {
  /// Additional Codira declarations that are unrepresentable in C++.
  std::vector<const ValueDecl *> additionalUnrepresentableDeclarations;
  /// Records the C++ declaration names already emitted in this lexical scope.
  toolchain::StringSet<> emittedDeclarationNames;
  /// Records the names of the function overloads already emitted in this
  /// lexical scope.
  toolchain::StringMap<toolchain::SmallVector<const AbstractFunctionDecl *, 2>>
      emittedFunctionOverloads;
  toolchain::StringMap<const AccessorDecl *> emittedAccessorMethodNames;
};

/// Responsible for printing a Codira Decl or Type in Objective-C, to be
/// included in a Codira module's ObjC compatibility header.
class DeclAndTypePrinter {
public:
  using DelayedMemberSet = toolchain::SmallSetVector<const ValueDecl *, 32>;

private:
  class Implementation;
  friend class Implementation;

  ModuleDecl &M;
  raw_ostream &os;
  raw_ostream &prologueOS;
  raw_ostream &outOfLineDefinitionsOS;
  const DelayedMemberSet &objcDelayedMembers;
  CxxDeclEmissionScope *cxxDeclEmissionScope;
  PrimitiveTypeMapping &typeMapping;
  CodiraToClangInteropContext &interopContext;
  AccessLevel minRequiredAccess;
  bool requiresExposedAttribute;
  toolchain::StringSet<> &exposedModules;
  OutputLanguageMode outputLang;

  /// The name 'CFTypeRef'.
  ///
  /// Cached for convenience.
  Identifier ID_CFTypeRef;

  Implementation getImpl();

public:
  DeclAndTypePrinter(ModuleDecl &mod, raw_ostream &out, raw_ostream &prologueOS,
                     raw_ostream &outOfLineDefinitionsOS,
                     const DelayedMemberSet &delayed,
                     CxxDeclEmissionScope &topLevelEmissionScope,
                     PrimitiveTypeMapping &typeMapping,
                     CodiraToClangInteropContext &interopContext,
                     AccessLevel access, bool requiresExposedAttribute,
                     toolchain::StringSet<> &exposedModules,
                     OutputLanguageMode outputLang)
      : M(mod), os(out), prologueOS(prologueOS),
        outOfLineDefinitionsOS(outOfLineDefinitionsOS),
        objcDelayedMembers(delayed),
        cxxDeclEmissionScope(&topLevelEmissionScope), typeMapping(typeMapping),
        interopContext(interopContext), minRequiredAccess(access),
        requiresExposedAttribute(requiresExposedAttribute),
        exposedModules(exposedModules), outputLang(outputLang) {}

  PrimitiveTypeMapping &getTypeMapping() { return typeMapping; }

  CodiraToClangInteropContext &getInteropContext() { return interopContext; }

  CxxDeclEmissionScope &getCxxDeclEmissionScope() {
    return *cxxDeclEmissionScope;
  }

  DeclAndTypePrinter withOutputStream(raw_ostream &s) {
    return DeclAndTypePrinter(
        M, s, prologueOS, outOfLineDefinitionsOS, objcDelayedMembers,
        *cxxDeclEmissionScope, typeMapping, interopContext, minRequiredAccess,
        requiresExposedAttribute, exposedModules, outputLang);
  }

  void setCxxDeclEmissionScope(CxxDeclEmissionScope &scope) {
    cxxDeclEmissionScope = &scope;
  }

  /// Returns true if \p VD should be included in a compatibility header for
  /// the options the printer was constructed with.
  bool shouldInclude(const ValueDecl *VD);

  bool isZeroSized(const NominalTypeDecl *decl);

  /// Returns true if \p vd is visible given the current access level and thus
  /// can be included in the generated header.
  bool isVisible(const ValueDecl *vd) const;

  void print(const Decl *D);
  void print(Type ty, std::optional<OptionalTypeKind> overrideOptionalTypeKind =
                          std::nullopt);

  /// Prints the name of the type including generic arguments.
  void printTypeName(raw_ostream &os, Type ty, const ModuleDecl *moduleContext);

  void printAvailability(raw_ostream &os, const Decl *D);

  /// Is \p ED empty of members and protocol conformances to include?
  bool isEmptyExtensionDecl(const ExtensionDecl *ED);

  /// Returns the type that will be printed by PrintAsObjC for a parameter or
  /// result type resolved to this declaration.
  ///
  /// \warning This handles \c _ObjectiveCBridgeable types, but it doesn't
  /// currently know about other aspects of PrintAsObjC behavior, like known
  /// types.
  const TypeDecl *getObjCTypeDecl(const TypeDecl* TD);

  /// Prints a category declaring the given members.
  ///
  /// All members must have the same parent type. The list must not be empty.
  void
  printAdHocCategory(iterator_range<const ValueDecl * const *> members);

  /// Returns the name of an <os/object.h> type minus the leading "OS_",
  /// or an empty string if \p decl is not an <os/object.h> type.
  static StringRef maybeGetOSObjectBaseName(const clang::NamedDecl *decl);

  static std::pair<Type, OptionalTypeKind>
  getObjectTypeAndOptionality(const ValueDecl *D, Type ty);
};

bool isStringNestedType(const ValueDecl *VD, StringRef Typename);

} // end namespace language

#endif
