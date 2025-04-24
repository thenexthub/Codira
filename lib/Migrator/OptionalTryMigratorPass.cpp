//===--- OptionalTryMigratorPass.cpp -------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "language/AST/ASTVisitor.h"
#include "language/AST/Expr.h"
#include "language/AST/FileUnit.h"
#include "language/AST/Module.h"
#include "language/AST/NameLookup.h"
#include "language/AST/ParameterList.h"
#include "language/AST/Types.h"
#include "language/IDE/SourceEntityWalker.h"
#include "language/Migrator/ASTMigratorPass.h"
#include "language/Parse/Lexer.h"

using namespace language;
using namespace language::migrator;

namespace {
  
  class OptionalTryMigratorPass: public ASTMigratorPass,
  public SourceEntityWalker {
    
    bool explicitCastActiveForOptionalTry = false;
    
    bool walkToExprPre(Expr *E) override {
      if (dyn_cast<ParenExpr>(E) || E->isImplicit()) {
        // Look through parentheses and implicit expressions.
        return true;
      }
      
      if (isa<ExplicitCastExpr>(E)) {
        // If the user has already provided an explicit cast for the
        // 'try?', then we don't need to add one. So let's track whether
        // one is active
        explicitCastActiveForOptionalTry = true;
      }
      else if (const auto *optTryExpr = dyn_cast<OptionalTryExpr>(E)) {
        wrapTryInCastIfNeeded(optTryExpr);
        return false;
      }
      else if (explicitCastActiveForOptionalTry) {
        // If an explicit cast is active and we are entering a new
        // expression that is not an OptionalTryExpr, then the cast
        // does not apply to the OptionalTryExpr.
        explicitCastActiveForOptionalTry = false;
      }
      return true;
    }
    
    bool walkToExprPost(Expr *E) override {
      explicitCastActiveForOptionalTry = false;
      return true;
    }
    
    void wrapTryInCastIfNeeded(const OptionalTryExpr *optTryExpr) {
      if (explicitCastActiveForOptionalTry) {
        // There's already an explicit cast here; we don't need to add anything
        return;
      }
      
      if (!optTryExpr->getSubExpr()->getType()->getOptionalObjectType()) {
        // This 'try?' doesn't wrap an optional, so its behavior does not
        // change from Swift 4 to Swift 5
        return;
      }
      
      Type typeToPreserve = optTryExpr->getType();
      auto typeName = typeToPreserve->getStringAsComponent();
      
      auto range = optTryExpr->getSourceRange();
      auto charRange = Lexer::getCharSourceRangeFromSourceRange(SM, range);
      Editor.insertWrap("((", charRange, (Twine(") as ") + typeName + ")").str());
    }
    
  public:
    OptionalTryMigratorPass(EditorAdapter &Editor,
                       SourceFile *SF,
                       const MigratorOptions &Opts)
    : ASTMigratorPass(Editor, SF, Opts) {}
  };
  
} // end anonymous namespace

void migrator::runOptionalTryMigratorPass(EditorAdapter &Editor,
                                     SourceFile *SF,
                                     const MigratorOptions &Opts) {
  OptionalTryMigratorPass { Editor, SF, Opts }.walk(SF);
}
