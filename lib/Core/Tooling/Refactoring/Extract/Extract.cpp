//===--- Extract.cpp - Clang refactoring library --------------------------===//
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
///
/// \file
/// Implements the "extract" refactoring that can pull code into
/// new functions, methods or declare new variables.
///
//===----------------------------------------------------------------------===//

#include "language/Core/Tooling/Refactoring/Extract/Extract.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ExprObjC.h"
#include "language/Core/Rewrite/Core/Rewriter.h"
#include "language/Core/Tooling/Refactoring/Extract/SourceExtraction.h"
#include <optional>

namespace language::Core {
namespace tooling {

namespace {

/// Returns true if \c E is a simple literal or a reference expression that
/// should not be extracted.
bool isSimpleExpression(const Expr *E) {
  if (!E)
    return false;
  switch (E->IgnoreParenCasts()->getStmtClass()) {
  case Stmt::DeclRefExprClass:
  case Stmt::PredefinedExprClass:
  case Stmt::IntegerLiteralClass:
  case Stmt::FloatingLiteralClass:
  case Stmt::ImaginaryLiteralClass:
  case Stmt::CharacterLiteralClass:
  case Stmt::StringLiteralClass:
    return true;
  default:
    return false;
  }
}

SourceLocation computeFunctionExtractionLocation(const Decl *D) {
  if (isa<CXXMethodDecl>(D)) {
    // Code from method that is defined in class body should be extracted to a
    // function defined just before the class.
    while (const auto *RD = dyn_cast<CXXRecordDecl>(D->getLexicalDeclContext()))
      D = RD;
  }
  return D->getBeginLoc();
}

} // end anonymous namespace

const RefactoringDescriptor &ExtractFunction::describe() {
  static const RefactoringDescriptor Descriptor = {
      "extract-function",
      "Extract Function",
      "(WIP action; use with caution!) Extracts code into a new function",
  };
  return Descriptor;
}

Expected<ExtractFunction>
ExtractFunction::initiate(RefactoringRuleContext &Context,
                          CodeRangeASTSelection Code,
                          std::optional<std::string> DeclName) {
  // We would like to extract code out of functions/methods/blocks.
  // Prohibit extraction from things like global variable / field
  // initializers and other top-level expressions.
  if (!Code.isInFunctionLikeBodyOfCode())
    return Context.createDiagnosticError(
        diag::err_refactor_code_outside_of_function);

  if (Code.size() == 1) {
    // Avoid extraction of simple literals and references.
    if (isSimpleExpression(dyn_cast<Expr>(Code[0])))
      return Context.createDiagnosticError(
          diag::err_refactor_extract_simple_expression);

    // Property setters can't be extracted.
    if (const auto *PRE = dyn_cast<ObjCPropertyRefExpr>(Code[0])) {
      if (!PRE->isMessagingGetter())
        return Context.createDiagnosticError(
            diag::err_refactor_extract_prohibited_expression);
    }
  }

  return ExtractFunction(std::move(Code), DeclName);
}

// FIXME: Support C++ method extraction.
// FIXME: Support Objective-C method extraction.
Expected<AtomicChanges>
ExtractFunction::createSourceReplacements(RefactoringRuleContext &Context) {
  const Decl *ParentDecl = Code.getFunctionLikeNearestParent();
  assert(ParentDecl && "missing parent");

  // Compute the source range of the code that should be extracted.
  SourceRange ExtractedRange(Code[0]->getBeginLoc(),
                             Code[Code.size() - 1]->getEndLoc());
  // FIXME (Alex L): Add code that accounts for macro locations.

  ASTContext &AST = Context.getASTContext();
  SourceManager &SM = AST.getSourceManager();
  const LangOptions &LangOpts = AST.getLangOpts();
  Rewriter ExtractedCodeRewriter(SM, LangOpts);

  // FIXME: Capture used variables.

  // Compute the return type.
  QualType ReturnType = AST.VoidTy;
  // FIXME (Alex L): Account for the return statement in extracted code.
  // FIXME (Alex L): Check for lexical expression instead.
  bool IsExpr = Code.size() == 1 && isa<Expr>(Code[0]);
  if (IsExpr) {
    // FIXME (Alex L): Get a more user-friendly type if needed.
    ReturnType = cast<Expr>(Code[0])->getType();
  }

  // FIXME: Rewrite the extracted code performing any required adjustments.

  // FIXME: Capture any field if necessary (method -> function extraction).

  // FIXME: Sort captured variables by name.

  // FIXME: Capture 'this' / 'self' if necessary.

  // FIXME: Compute the actual parameter types.

  // Compute the location of the extracted declaration.
  SourceLocation ExtractedDeclLocation =
      computeFunctionExtractionLocation(ParentDecl);
  // FIXME: Adjust the location to account for any preceding comments.

  // FIXME: Adjust with PP awareness like in Sema to get correct 'bool'
  // treatment.
  PrintingPolicy PP = AST.getPrintingPolicy();
  // FIXME: PP.UseStdFunctionForLambda = true;
  PP.SuppressStrongLifetime = true;
  PP.SuppressLifetimeQualifiers = true;
  PP.SuppressUnwrittenScope = true;

  ExtractionSemicolonPolicy Semicolons = ExtractionSemicolonPolicy::compute(
      Code[Code.size() - 1], ExtractedRange, SM, LangOpts);
  AtomicChange Change(SM, ExtractedDeclLocation);
  // Create the replacement for the extracted declaration.
  {
    std::string ExtractedCode;
    toolchain::raw_string_ostream OS(ExtractedCode);
    // FIXME: Use 'inline' in header.
    OS << "static ";
    ReturnType.print(OS, PP, DeclName);
    OS << '(';
    // FIXME: Arguments.
    OS << ')';

    // Function body.
    OS << " {\n";
    if (IsExpr && !ReturnType->isVoidType())
      OS << "return ";
    OS << ExtractedCodeRewriter.getRewrittenText(ExtractedRange);
    if (Semicolons.isNeededInExtractedFunction())
      OS << ';';
    OS << "\n}\n\n";
    auto Err = Change.insert(SM, ExtractedDeclLocation, OS.str());
    if (Err)
      return std::move(Err);
  }

  // Create the replacement for the call to the extracted declaration.
  {
    std::string ReplacedCode;
    toolchain::raw_string_ostream OS(ReplacedCode);

    OS << DeclName << '(';
    // FIXME: Forward arguments.
    OS << ')';
    if (Semicolons.isNeededInOriginalFunction())
      OS << ';';

    auto Err = Change.replace(
        SM, CharSourceRange::getTokenRange(ExtractedRange), OS.str());
    if (Err)
      return std::move(Err);
  }

  // FIXME: Add support for assocciated symbol location to AtomicChange to mark
  // the ranges of the name of the extracted declaration.
  return AtomicChanges{std::move(Change)};
}

} // end namespace tooling
} // end namespace language::Core
