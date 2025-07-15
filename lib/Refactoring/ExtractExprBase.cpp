//===----------------------------------------------------------------------===//
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

#include "ExtractExprBase.h"
#include "ContextFinder.h"
#include "RefactoringActions.h"
#include "Utils.h"
#include "language/AST/DiagnosticsRefactoring.h"
#include "language/AST/Stmt.h"
#include "language/Basic/Assertions.h"

using namespace language::refactoring;

namespace {
/// This is to ensure all decl references in two expressions are identical.
struct ReferenceCollector : public SourceEntityWalker {
  SmallVector<ValueDecl *, 4> References;

  ReferenceCollector(Expr *E) { walk(E); }
  bool visitDeclReference(ValueDecl *D, SourceRange Range, TypeDecl *CtorTyRef,
                          ExtensionDecl *ExtTyRef, Type T,
                          ReferenceMetaData Data) override {
    References.emplace_back(D);
    return true;
  }
  bool operator==(const ReferenceCollector &Other) const {
    if (References.size() != Other.References.size())
      return false;
    return std::equal(References.begin(), References.end(),
                      Other.References.begin());
  }
};

struct SimilarExprCollector : public SourceEntityWalker {
  SourceManager &SM;

  /// The expression under selection.
  Expr *SelectedExpr;
  ArrayRef<Token> AllTokens;
  toolchain::SetVector<Expr *> &Bucket;

  /// The tokens included in the expression under selection.
  ArrayRef<Token> SelectedTokens;

  /// The referenced decls in the expression under selection.
  ReferenceCollector SelectedReferences;

  bool compareTokenContent(ArrayRef<Token> Left, ArrayRef<Token> Right) {
    if (Left.size() != Right.size())
      return false;
    return std::equal(Left.begin(), Left.end(), Right.begin(),
                      [](const Token &L, const Token &R) {
                        return L.getText() == R.getText();
                      });
  }

  /// Find all tokens included by an expression.
  ArrayRef<Token> getExprSlice(Expr *E) {
    return slice_token_array(AllTokens, E->getStartLoc(), E->getEndLoc());
  }

  SimilarExprCollector(SourceManager &SM, Expr *SelectedExpr,
                       ArrayRef<Token> AllTokens,
                       toolchain::SetVector<Expr *> &Bucket)
      : SM(SM), SelectedExpr(SelectedExpr), AllTokens(AllTokens),
        Bucket(Bucket), SelectedTokens(getExprSlice(SelectedExpr)),
        SelectedReferences(SelectedExpr) {}

  bool walkToExprPre(Expr *E) override {
    // We don't extract implicit expressions.
    if (E->isImplicit())
      return true;
    if (E->getKind() != SelectedExpr->getKind())
      return true;

    // First check the underlying token arrays have the same content.
    if (compareTokenContent(getExprSlice(E), SelectedTokens)) {
      ReferenceCollector CurrentReferences(E);

      // Next, check the referenced decls are same.
      if (CurrentReferences == SelectedReferences)
        Bucket.insert(E);
    }
    return true;
  }
};

} // namespace

ExtractCheckResult
language::refactoring::checkExtractConditions(const ResolvedRangeInfo &RangeInfo,
                                           DiagnosticEngine &DiagEngine) {
  SmallVector<CannotExtractReason, 2> AllReasons;
  // If any declared declaration is referred out of the given range, return
  // false.
  auto Declared = RangeInfo.DeclaredDecls;
  auto It = std::find_if(Declared.begin(), Declared.end(),
                         [](DeclaredDecl DD) { return DD.ReferredAfterRange; });
  if (It != Declared.end()) {
    DiagEngine.diagnose(It->VD->getLoc(),
                        diag::value_decl_referenced_out_of_range,
                        It->VD->getName());
    return ExtractCheckResult();
  }

  // We cannot extract a range with multi entry points.
  if (!RangeInfo.HasSingleEntry) {
    DiagEngine.diagnose(SourceLoc(), diag::multi_entry_range);
    return ExtractCheckResult();
  }

  // We cannot extract code that is not sure to exit or not.
  if (RangeInfo.exit() == ExitState::Unsure) {
    return ExtractCheckResult();
  }

  // We cannot extract expressions of l-value type.
  if (auto Ty = RangeInfo.getType()) {
    if (Ty->hasLValueType() || Ty->is<InOutType>())
      return ExtractCheckResult();

    // Disallow extracting error type expressions/statements
    // FIXME: diagnose what happened?
    if (Ty->hasError())
      return ExtractCheckResult();

    if (Ty->isVoid()) {
      AllReasons.emplace_back(CannotExtractReason::VoidType);
    }
  }

  // We cannot extract a range with orphaned loop keyword.
  switch (RangeInfo.Orphan) {
  case language::ide::OrphanKind::Continue:
    DiagEngine.diagnose(SourceLoc(), diag::orphan_loop_keyword, "continue");
    return ExtractCheckResult();
  case language::ide::OrphanKind::Break:
    DiagEngine.diagnose(SourceLoc(), diag::orphan_loop_keyword, "break");
    return ExtractCheckResult();
  case language::ide::OrphanKind::None:
    break;
  }

  // Guard statement can not be extracted.
  if (toolchain::any_of(RangeInfo.ContainedNodes,
                   [](ASTNode N) { return N.isStmt(StmtKind::Guard); })) {
    return ExtractCheckResult();
  }

  // Disallow extracting certain kinds of statements.
  if (RangeInfo.Kind == RangeKind::SingleStatement) {
    Stmt *S = RangeInfo.ContainedNodes[0].get<Stmt *>();

    // These aren't independent statement.
    if (isa<BraceStmt>(S) || isa<CaseStmt>(S))
      return ExtractCheckResult();
  }

  // Disallow extracting literals.
  if (RangeInfo.Kind == RangeKind::SingleExpression) {
    Expr *E = RangeInfo.ContainedNodes[0].get<Expr *>();

    // Until implementing the performChange() part of extracting trailing
    // closures, we disable them for now.
    if (isa<AbstractClosureExpr>(E))
      return ExtractCheckResult();

    if (isa<LiteralExpr>(E))
      AllReasons.emplace_back(CannotExtractReason::Literal);
  }

  switch (RangeInfo.RangeContext->getContextKind()) {
  case language::DeclContextKind::Initializer:
  case language::DeclContextKind::SubscriptDecl:
  case language::DeclContextKind::EnumElementDecl:
  case language::DeclContextKind::AbstractFunctionDecl:
  case language::DeclContextKind::AbstractClosureExpr:
  case language::DeclContextKind::TopLevelCodeDecl:
    break;

  case language::DeclContextKind::SerializedAbstractClosure:
  case language::DeclContextKind::SerializedTopLevelCodeDecl:
  case language::DeclContextKind::Package:
  case language::DeclContextKind::Module:
  case language::DeclContextKind::FileUnit:
  case language::DeclContextKind::GenericTypeDecl:
  case language::DeclContextKind::ExtensionDecl:
  case language::DeclContextKind::MacroDecl:
    return ExtractCheckResult();
  }
  return ExtractCheckResult(AllReasons);
}

bool RefactoringActionExtractExprBase::performChange() {
  // Check if the new name is ok.
  if (!Lexer::isIdentifier(PreferredName)) {
    DiagEngine.diagnose(SourceLoc(), diag::invalid_name, PreferredName);
    return true;
  }

  // Find the enclosing brace statement;
  ContextFinder Finder(*TheFile, RangeInfo.ContainedNodes.front(),
                       [](ASTNode N) { return N.isStmt(StmtKind::Brace); });

  auto *SelectedExpr = RangeInfo.ContainedNodes[0].get<Expr *>();
  Finder.resolve();
  SourceLoc InsertLoc;
  toolchain::SetVector<ValueDecl *> AllVisibleDecls;
  struct DeclCollector : public SourceEntityWalker {
    toolchain::SetVector<ValueDecl *> &Bucket;
    DeclCollector(toolchain::SetVector<ValueDecl *> &Bucket) : Bucket(Bucket) {}
    bool walkToDeclPre(Decl *D, CharSourceRange Range) override {
      if (auto *VD = dyn_cast<ValueDecl>(D))
        Bucket.insert(VD);
      return true;
    }
  } Collector(AllVisibleDecls);

  toolchain::SetVector<Expr *> AllExpressions;

  if (!Finder.getContexts().empty()) {

    // Get the innermost brace statement.
    auto BS =
        static_cast<BraceStmt *>(Finder.getContexts().back().get<Stmt *>());

    // Collect all value decls inside the brace statement.
    Collector.walk(BS);

    if (ExtractRepeated) {
      // Collect all expressions we are going to extract.
      SimilarExprCollector(SM, SelectedExpr,
                           slice_token_array(TheFile->getAllTokens(),
                                             BS->getStartLoc(),
                                             BS->getEndLoc()),
                           AllExpressions)
          .walk(BS);
    } else {
      AllExpressions.insert(SelectedExpr);
    }

    assert(!AllExpressions.empty() && "at least one expression is extracted.");
    for (auto Ele : BS->getElements()) {
      // Find the element that encloses the first expression under extraction.
      if (SM.rangeContains(Ele.getSourceRange(),
                           (*AllExpressions.begin())->getSourceRange())) {

        // Insert before the enclosing element.
        InsertLoc = Ele.getStartLoc();
      }
    }
  }

  // Complain about no inserting position.
  if (InsertLoc.isInvalid()) {
    DiagEngine.diagnose(SourceLoc(), diag::no_insert_position);
    return true;
  }

  // Correct name if collision happens.
  PreferredName = correctNameInternal(TheFile->getASTContext(), PreferredName,
                                      AllVisibleDecls.getArrayRef());

  // Print the type name of this expression.
  SmallString<16> TyBuffer;

  // We are not sure about the type of repeated expressions.
  if (!ExtractRepeated) {
    if (auto Ty = RangeInfo.getType()) {
      toolchain::raw_svector_ostream OS(TyBuffer);
      OS << ": ";
      Ty->getRValueType()->reconstituteSugar(true)->print(OS);
    }
  }

  SmallString<64> DeclBuffer;
  toolchain::raw_svector_ostream OS(DeclBuffer);
  unsigned StartOffset, EndOffset;
  OS << tok::kw_let << " ";
  StartOffset = DeclBuffer.size();
  OS << PreferredName;
  EndOffset = DeclBuffer.size();
  OS << TyBuffer.str() << " = " << RangeInfo.ContentRange.str() << "\n";

  NoteRegion DeclNameRegion{RefactoringRangeKind::BaseName,
                            /*StartLine=*/1,
                            /*StartColumn=*/StartOffset + 1,
                            /*EndLine=*/1,
                            /*EndColumn=*/EndOffset + 1,
                            /*ArgIndex*/ std::nullopt};

  // Perform code change.
  EditConsumer.accept(SM, InsertLoc, DeclBuffer.str(), {DeclNameRegion});

  // Replace all occurrences of the extracted expression.
  for (auto *E : AllExpressions) {
    EditConsumer.accept(
        SM, Lexer::getCharSourceRangeFromSourceRange(SM, E->getSourceRange()),
        PreferredName,
        {{RefactoringRangeKind::BaseName,
          /*StartLine=*/1, /*StartColumn-*/ 1, /*EndLine=*/1,
          /*EndColumn=*/static_cast<unsigned int>(PreferredName.size() + 1),
          /*ArgIndex*/ std::nullopt}});
  }
  return false;
}
