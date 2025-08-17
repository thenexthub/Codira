//===- ASTStructuralEquivalence.h -------------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//
//  This file defines the StructuralEquivalenceContext class which checks for
//  structural equivalence between types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ASTSTRUCTURALEQUIVALENCE_H
#define LANGUAGE_CORE_AST_ASTSTRUCTURALEQUIVALENCE_H

#include "language/Core/AST/DeclBase.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/DenseSet.h"
#include <optional>
#include <queue>
#include <utility>

namespace language::Core {

class ASTContext;
class Decl;
class DiagnosticBuilder;
class QualType;
class RecordDecl;
class SourceLocation;

/// \brief Whether to perform a normal or minimal equivalence check.
/// In case of `Minimal`, we do not perform a recursive check of decls with
/// external storage.
enum class StructuralEquivalenceKind {
  Default,
  Minimal,
};

struct StructuralEquivalenceContext {
  /// Store declaration pairs already found to be non-equivalent.
  /// key: (from, to, IgnoreTemplateParmDepth)
  using NonEquivalentDeclSet = toolchain::DenseSet<std::tuple<Decl *, Decl *, int>>;

  /// The language options to use for making a structural equivalence check.
  const LangOptions &LangOpts;

  /// AST contexts for which we are checking structural equivalence.
  ASTContext &FromCtx, &ToCtx;

  // Queue of from-to Decl pairs that are to be checked to determine the final
  // result of equivalence of a starting Decl pair.
  std::queue<std::pair<Decl *, Decl *>> DeclsToCheck;

  // Set of from-to Decl pairs that are already visited during the check
  // (are in or were once in \c DeclsToCheck) of a starting Decl pair.
  toolchain::DenseSet<std::pair<Decl *, Decl *>> VisitedDecls;

  /// Declaration (from, to) pairs that are known not to be equivalent
  /// (which we have already complained about).
  NonEquivalentDeclSet &NonEquivalentDecls;

  StructuralEquivalenceKind EqKind;

  /// Whether we're being strict about the spelling of types when
  /// unifying two types.
  bool StrictTypeSpelling;

  /// Whether warn or error on tag type mismatches.
  bool ErrorOnTagTypeMismatch;

  /// Whether to complain about failures.
  bool Complain;

  /// \c true if the last diagnostic came from ToCtx.
  bool LastDiagFromC2 = false;

  /// Whether to ignore comparing the depth of template param(TemplateTypeParm)
  bool IgnoreTemplateParmDepth;

  StructuralEquivalenceContext(const LangOptions &LangOpts, ASTContext &FromCtx,
                               ASTContext &ToCtx,
                               NonEquivalentDeclSet &NonEquivalentDecls,
                               StructuralEquivalenceKind EqKind,
                               bool StrictTypeSpelling = false,
                               bool Complain = true,
                               bool ErrorOnTagTypeMismatch = false,
                               bool IgnoreTemplateParmDepth = false)
      : LangOpts(LangOpts), FromCtx(FromCtx), ToCtx(ToCtx),
        NonEquivalentDecls(NonEquivalentDecls), EqKind(EqKind),
        StrictTypeSpelling(StrictTypeSpelling),
        ErrorOnTagTypeMismatch(ErrorOnTagTypeMismatch), Complain(Complain),
        IgnoreTemplateParmDepth(IgnoreTemplateParmDepth) {}

  DiagnosticBuilder Diag1(SourceLocation Loc, unsigned DiagID);
  DiagnosticBuilder Diag2(SourceLocation Loc, unsigned DiagID);

  /// Determine whether the two declarations are structurally
  /// equivalent.
  /// Implementation functions (all static functions in
  /// ASTStructuralEquivalence.cpp) must never call this function because that
  /// will wreak havoc the internal state (\c DeclsToCheck and
  /// \c VisitedDecls members) and can cause faulty equivalent results.
  bool IsEquivalent(Decl *D1, Decl *D2);

  /// Determine whether the two types are structurally equivalent.
  /// Implementation functions (all static functions in
  /// ASTStructuralEquivalence.cpp) must never call this function because that
  /// will wreak havoc the internal state (\c DeclsToCheck and
  /// \c VisitedDecls members) and can cause faulty equivalent results.
  bool IsEquivalent(QualType T1, QualType T2);

  /// Determine whether the two statements are structurally equivalent.
  /// Implementation functions (all static functions in
  /// ASTStructuralEquivalence.cpp) must never call this function because that
  /// will wreak havoc the internal state (\c DeclsToCheck and
  /// \c VisitedDecls members) and can cause faulty equivalent results.
  bool IsEquivalent(Stmt *S1, Stmt *S2);

  /// Find the index of the given anonymous struct/union within its
  /// context.
  ///
  /// \returns Returns the index of this anonymous struct/union in its context,
  /// including the next assigned index (if none of them match). Returns an
  /// empty option if the context is not a record, i.e.. if the anonymous
  /// struct/union is at namespace or block scope.
  ///
  /// FIXME: This is needed by ASTImporter and ASTStructureEquivalence. It
  /// probably makes more sense in some other common place then here.
  static UnsignedOrNone findUntaggedStructOrUnionIndex(RecordDecl *Anon);

  // If ErrorOnTagTypeMismatch is set, return the error, otherwise get the
  // relevant warning for the input error diagnostic.
  unsigned getApplicableDiagnostic(unsigned ErrorDiagnostic);

private:
  /// Finish checking all of the structural equivalences.
  ///
  /// \returns true if the equivalence check failed (non-equivalence detected),
  /// false if equivalence was detected.
  bool Finish();

  /// Check for common properties at Finish.
  /// \returns true if D1 and D2 may be equivalent,
  /// false if they are for sure not.
  bool CheckCommonEquivalence(Decl *D1, Decl *D2);

  /// Check for class dependent properties at Finish.
  /// \returns true if D1 and D2 may be equivalent,
  /// false if they are for sure not.
  bool CheckKindSpecificEquivalence(Decl *D1, Decl *D2);
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_ASTSTRUCTURALEQUIVALENCE_H
