//===--- FreestandingMacroExpansion.h ------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_AST_FREESTANDING_MACRO_EXPANSION_H
#define LANGUAGE_AST_FREESTANDING_MACRO_EXPANSION_H

#include "language/AST/ASTAllocated.h"
#include "language/AST/ASTNode.h"
#include "language/AST/ConcreteDeclRef.h"
#include "language/AST/DeclNameLoc.h"
#include "language/AST/Identifier.h"
#include "language/Basic/SourceLoc.h"

namespace language {
class MacroExpansionDecl;
class MacroExpansionExpr;
class Expr;
class Decl;
class ArgumentList;

/// Information about a macro expansion that is common between macro
/// expansion declarations and expressions.
///
/// Instances of these types will be shared among paired macro expansion
/// declaration/expression nodes.
struct MacroExpansionInfo : ASTAllocated<MacroExpansionInfo> {
  SourceLoc SigilLoc;
  DeclNameRef ModuleName;
  DeclNameLoc ModuleNameLoc;
  DeclNameRef MacroName;
  DeclNameLoc MacroNameLoc;
  SourceLoc LeftAngleLoc, RightAngleLoc;
  toolchain::ArrayRef<TypeRepr *> GenericArgs;
  ArgumentList *ArgList;

  /// The referenced macro.
  ConcreteDeclRef macroRef;

  enum : unsigned { InvalidDiscriminator = 0xFFFF };

  unsigned Discriminator = InvalidDiscriminator;

  MacroExpansionInfo(SourceLoc sigilLoc, DeclNameRef moduleName,
                     DeclNameLoc moduleNameLoc, DeclNameRef macroName,
                     DeclNameLoc macroNameLoc, SourceLoc leftAngleLoc,
                     SourceLoc rightAngleLoc, ArrayRef<TypeRepr *> genericArgs,
                     ArgumentList *argList)
      : SigilLoc(sigilLoc), ModuleName(moduleName),
        ModuleNameLoc(moduleNameLoc), MacroName(macroName),
        MacroNameLoc(macroNameLoc), LeftAngleLoc(leftAngleLoc),
        RightAngleLoc(rightAngleLoc), GenericArgs(genericArgs),
        ArgList(argList) {}

  SourceLoc getLoc() const { return SigilLoc; }
  SourceRange getGenericArgsRange() const {
    return {LeftAngleLoc, RightAngleLoc};
  }
  SourceRange getSourceRange() const;
};

enum class FreestandingMacroKind {
  Expr, // MacroExpansionExpr.
  Decl, // MacroExpansionDecl.
};

/// A base class of either 'MacroExpansionExpr' or 'MacroExpansionDecl'.
class FreestandingMacroExpansion {
  toolchain::PointerIntPair<MacroExpansionInfo *, 1, FreestandingMacroKind>
    infoAndKind;

protected:
  FreestandingMacroExpansion(FreestandingMacroKind kind,
                             MacroExpansionInfo *info)
  : infoAndKind(info, kind) {}

public:
  MacroExpansionInfo *getExpansionInfo() const {
    return infoAndKind.getPointer();
  }
  FreestandingMacroKind getFreestandingMacroKind() const {
    return infoAndKind.getInt();
  }

  ASTNode getASTNode();

  SourceLoc getPoundLoc() const { return getExpansionInfo()->SigilLoc; }

  DeclNameLoc getModuleNameLoc() const {
    return getExpansionInfo()->ModuleNameLoc;
  }
  DeclNameRef getModuleName() const { return getExpansionInfo()->ModuleName; }
  DeclNameLoc getMacroNameLoc() const {
    return getExpansionInfo()->MacroNameLoc;
  }
  DeclNameRef getMacroName() const { return getExpansionInfo()->MacroName; }

  ArrayRef<TypeRepr *> getGenericArgs() const {
    return getExpansionInfo()->GenericArgs;
  }
  SourceRange getGenericArgsRange() const {
    return getExpansionInfo()->getGenericArgsRange();
  }

  ArgumentList *getArgs() const { return getExpansionInfo()->ArgList; }
  void setArgs(ArgumentList *args) { getExpansionInfo()->ArgList = args; }

  ConcreteDeclRef getMacroRef() const { return getExpansionInfo()->macroRef; }
  void setMacroRef(ConcreteDeclRef ref) { getExpansionInfo()->macroRef = ref; }

  DeclContext *getDeclContext() const;
  SourceRange getSourceRange() const;

  /// Returns a discriminator which determines this macro expansion's index
  /// in the sequence of macro expansions within the current context.
  unsigned getDiscriminator() const;

  /// Returns the raw discriminator, for debugging purposes only.
  unsigned getRawDiscriminator() const;
};

} // namespace language

#endif // LANGUAGE_AST_FREESTANDING_MACRO_EXPANSION_H
