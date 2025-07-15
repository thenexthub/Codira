//===--- PrettyStackTrace.h - Crash trace information -----------*- C++ -*-===//
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
//
// This file defines RAII classes that give better diagnostic output
// about when, exactly, a crash is occurring.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_PRETTYSTACKTRACE_H
#define LANGUAGE_PRETTYSTACKTRACE_H

#include "language/AST/AnyFunctionRef.h"
#include "language/AST/FreestandingMacroExpansion.h"
#include "language/AST/Identifier.h"
#include "language/AST/Type.h"
#include "language/Basic/SourceLoc.h"
#include "toolchain/Support/PrettyStackTrace.h"
#include <optional>

namespace clang {
  class Type;
  class ASTContext;
}

namespace language {
  class ASTContext;
  class Decl;
  class Expr;
  class GenericSignature;
  class Pattern;
  class Stmt;
  class TypeRepr;

void printSourceLocDescription(toolchain::raw_ostream &out, SourceLoc loc,
                               const ASTContext &Context,
                               bool addNewline = true);

/// PrettyStackTraceLocation - Observe that we are doing some
/// processing starting at a fixed location.
class PrettyStackTraceLocation : public toolchain::PrettyStackTraceEntry {
  const ASTContext &Context;
  SourceLoc Loc;
  const char *Action;
public:
  PrettyStackTraceLocation(const ASTContext &C, const char *action,
                           SourceLoc loc)
    : Context(C), Loc(loc), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

void printDeclDescription(toolchain::raw_ostream &out, const Decl *D,
                          bool addNewline = true);

/// PrettyStackTraceDecl - Observe that we are processing a specific
/// declaration.
class PrettyStackTraceDecl : public toolchain::PrettyStackTraceEntry {
  const Decl *TheDecl;
  const char *Action;
public:
  PrettyStackTraceDecl(const char *action, const Decl *D)
    : TheDecl(D), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

/// PrettyStackTraceDecl - Observe that we are processing a specific
/// declaration with a given substitution map.
class PrettyStackTraceDeclAndSubst : public toolchain::PrettyStackTraceEntry {
  const Decl *decl;
  SubstitutionMap subst;
  const char *action;
public:
  PrettyStackTraceDeclAndSubst(const char *action, SubstitutionMap subst,
                       const Decl *decl)
      : decl(decl), subst(subst), action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

/// PrettyStackTraceAnyFunctionRef - Observe that we are processing a specific
/// function or closure literal.
class PrettyStackTraceAnyFunctionRef : public toolchain::PrettyStackTraceEntry {
  AnyFunctionRef TheRef;
  const char *Action;
public:
  PrettyStackTraceAnyFunctionRef(const char *action, AnyFunctionRef ref)
    : TheRef(ref), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

/// PrettyStackTraceFreestandingMacroExpansion -  Observe that we are
/// processing a specific freestanding macro expansion.
class PrettyStackTraceFreestandingMacroExpansion
    : public toolchain::PrettyStackTraceEntry {
  const FreestandingMacroExpansion *Expansion;
  const char *Action;

public:
  PrettyStackTraceFreestandingMacroExpansion(
      const char *action, const FreestandingMacroExpansion *expansion)
      : Expansion(expansion), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

void printExprDescription(toolchain::raw_ostream &out, const Expr *E,
                          const ASTContext &Context, bool addNewline = true);

/// PrettyStackTraceExpr - Observe that we are processing a specific
/// expression.
class PrettyStackTraceExpr : public toolchain::PrettyStackTraceEntry {
  const ASTContext &Context;
  Expr *TheExpr;
  const char *Action;
public:
  PrettyStackTraceExpr(const ASTContext &C, const char *action, Expr *E)
    : Context(C), TheExpr(E), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

void printStmtDescription(toolchain::raw_ostream &out, Stmt *S,
                          const ASTContext &Context, bool addNewline = true);

/// PrettyStackTraceStmt - Observe that we are processing a specific
/// statement.
class PrettyStackTraceStmt : public toolchain::PrettyStackTraceEntry {
  const ASTContext &Context;
  Stmt *TheStmt;
  const char *Action;
public:
  PrettyStackTraceStmt(const ASTContext &C, const char *action, Stmt *S)
    : Context(C), TheStmt(S), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

void printPatternDescription(toolchain::raw_ostream &out, Pattern *P,
                             const ASTContext &Context, bool addNewline = true);

/// PrettyStackTracePattern - Observe that we are processing a
/// specific pattern.
class PrettyStackTracePattern : public toolchain::PrettyStackTraceEntry {
  const ASTContext &Context;
  Pattern *ThePattern;
  const char *Action;
public:
  PrettyStackTracePattern(const ASTContext &C, const char *action, Pattern *P)
    : Context(C), ThePattern(P), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

void printTypeDescription(toolchain::raw_ostream &out, Type T,
                          const ASTContext &Context, bool addNewline = true);

/// PrettyStackTraceType - Observe that we are processing a specific type.
class PrettyStackTraceType : public toolchain::PrettyStackTraceEntry {
  const ASTContext &Context;
  Type TheType;
  const char *Action;
public:
  PrettyStackTraceType(const ASTContext &C, const char *action, Type type)
    : Context(C), TheType(type), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

/// PrettyStackTraceClangType - Observe that we are processing a
/// specific Clang type.
class PrettyStackTraceClangType : public toolchain::PrettyStackTraceEntry {
  const clang::ASTContext &Context;
  const clang::Type *TheType;
  const char *Action;
public:
  PrettyStackTraceClangType(clang::ASTContext &ctx,
                            const char *action, const clang::Type *type)
    : Context(ctx), TheType(type), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

/// Observe that we are processing a specific type representation.
class PrettyStackTraceTypeRepr : public toolchain::PrettyStackTraceEntry {
  const ASTContext &Context;
  TypeRepr *TheType;
  const char *Action;
public:
  PrettyStackTraceTypeRepr(const ASTContext &C, const char *action,
                           TypeRepr *type)
    : Context(C), TheType(type), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

/// PrettyStackTraceConformance - Observe that we are processing a
/// specific protocol conformance.
class PrettyStackTraceConformance : public toolchain::PrettyStackTraceEntry {
  const ProtocolConformance *Conformance;
  const char *Action;
public:
  PrettyStackTraceConformance(const char *action,
                              const ProtocolConformance *conformance)
    : Conformance(conformance), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

void printConformanceDescription(toolchain::raw_ostream &out,
                                 const ProtocolConformance *conformance,
                                 const ASTContext &Context,
                                 bool addNewline = true);

class PrettyStackTraceGenericSignature : public toolchain::PrettyStackTraceEntry {
  const char *Action;
  GenericSignature GenericSig;
  std::optional<unsigned> Requirement;

public:
  PrettyStackTraceGenericSignature(
      const char *action, GenericSignature genericSig,
      std::optional<unsigned> requirement = std::nullopt)
      : Action(action), GenericSig(genericSig), Requirement(requirement) {}

  void setRequirement(std::optional<unsigned> requirement) {
    Requirement = requirement;
  }

  void print(toolchain::raw_ostream &out) const override;
};

class PrettyStackTraceSelector : public toolchain::PrettyStackTraceEntry {
  ObjCSelector Selector;
  const char *Action;
public:
  PrettyStackTraceSelector(const char *action, ObjCSelector S)
    : Selector(S), Action(action) {}
  void print(toolchain::raw_ostream &OS) const override;
};

/// PrettyStackTraceDifferentiabilityWitness - Observe that we are processing a
/// specific differentiability witness.
class PrettyStackTraceDifferentiabilityWitness
    : public toolchain::PrettyStackTraceEntry {
  const SILDifferentiabilityWitnessKey Key;
  const char *Action;

public:
  PrettyStackTraceDifferentiabilityWitness(
      const char *action, const SILDifferentiabilityWitnessKey key)
      : Key(key), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

void printDifferentiabilityWitnessDescription(
    toolchain::raw_ostream &out, const SILDifferentiabilityWitnessKey key,
    bool addNewline = true);

/// PrettyStackTraceDeclContext - Observe that we are processing a
/// specific decl context.
class PrettyStackTraceDeclContext : public toolchain::PrettyStackTraceEntry {
  const DeclContext *DC;
  const char *Action;
public:
  PrettyStackTraceDeclContext(const char *action, const DeclContext *DC)
    : DC(DC), Action(action) {}
  virtual void print(toolchain::raw_ostream &OS) const override;
};

} // end namespace language

#endif
