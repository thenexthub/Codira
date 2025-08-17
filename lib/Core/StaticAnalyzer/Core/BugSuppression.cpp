//===- BugSuppression.cpp - Suppression interface -------------------------===//
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

#include "language/Core/StaticAnalyzer/Core/BugReporter/BugSuppression.h"
#include "language/Core/AST/DynamicRecursiveASTVisitor.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "toolchain/Support/FormatVariadic.h"
#include "toolchain/Support/TimeProfiler.h"

using namespace language::Core;
using namespace ento;

namespace {

using Ranges = toolchain::SmallVectorImpl<SourceRange>;

inline bool hasSuppression(const Decl *D) {
  // FIXME: Implement diagnostic identifier arguments
  // (checker names, "hashtags").
  if (const auto *Suppression = D->getAttr<SuppressAttr>())
    return !Suppression->isGSL() &&
           (Suppression->diagnosticIdentifiers().empty());
  return false;
}
inline bool hasSuppression(const AttributedStmt *S) {
  // FIXME: Implement diagnostic identifier arguments
  // (checker names, "hashtags").
  return toolchain::any_of(S->getAttrs(), [](const Attr *A) {
    const auto *Suppression = dyn_cast<SuppressAttr>(A);
    return Suppression && !Suppression->isGSL() &&
           (Suppression->diagnosticIdentifiers().empty());
  });
}

template <class NodeType> inline SourceRange getRange(const NodeType *Node) {
  return Node->getSourceRange();
}
template <> inline SourceRange getRange(const AttributedStmt *S) {
  // Begin location for attributed statement node seems to be ALWAYS invalid.
  //
  // It is unlikely that we ever report any warnings on suppression
  // attribute itself, but even if we do, we wouldn't want that warning
  // to be suppressed by that same attribute.
  //
  // Long story short, we can use inner statement and it's not going to break
  // anything.
  return getRange(S->getSubStmt());
}

inline bool isLessOrEqual(SourceLocation LHS, SourceLocation RHS,
                          const SourceManager &SM) {
  // SourceManager::isBeforeInTranslationUnit tests for strict
  // inequality, when we need a non-strict comparison (bug
  // can be reported directly on the annotated note).
  // For this reason, we use the following equivalence:
  //
  //   A <= B <==> !(B < A)
  //
  return !SM.isBeforeInTranslationUnit(RHS, LHS);
}

inline bool fullyContains(SourceRange Larger, SourceRange Smaller,
                          const SourceManager &SM) {
  // Essentially this means:
  //
  //   Larger.fullyContains(Smaller)
  //
  // However, that method has a very trivial implementation and couldn't
  // compare regular locations and locations from macro expansions.
  // We could've converted everything into regular locations as a solution,
  // but the following solution seems to be the most bulletproof.
  return isLessOrEqual(Larger.getBegin(), Smaller.getBegin(), SM) &&
         isLessOrEqual(Smaller.getEnd(), Larger.getEnd(), SM);
}

class CacheInitializer : public DynamicRecursiveASTVisitor {
public:
  static void initialize(const Decl *D, Ranges &ToInit) {
    CacheInitializer(ToInit).TraverseDecl(const_cast<Decl *>(D));
  }

  bool VisitDecl(Decl *D) override {
    // Bug location could be somewhere in the init value of
    // a freshly declared variable.  Even though it looks like the
    // user applied attribute to a statement, it will apply to a
    // variable declaration, and this is where we check for it.
    return VisitAttributedNode(D);
  }

  bool VisitAttributedStmt(AttributedStmt *AS) override {
    // When we apply attributes to statements, it actually creates
    // a wrapper statement that only contains attributes and the wrapped
    // statement.
    return VisitAttributedNode(AS);
  }

private:
  template <class NodeType> bool VisitAttributedNode(NodeType *Node) {
    if (hasSuppression(Node)) {
      // TODO: In the future, when we come up with good stable IDs for checkers
      //       we can return a list of kinds to ignore, or all if no arguments
      //       were provided.
      addRange(getRange(Node));
    }
    // We should keep traversing AST.
    return true;
  }

  void addRange(SourceRange R) {
    if (R.isValid()) {
      Result.push_back(R);
    }
  }

  CacheInitializer(Ranges &R) : Result(R) {}
  Ranges &Result;
};

std::string timeScopeName(const Decl *DeclWithIssue) {
  if (!toolchain::timeTraceProfilerEnabled())
    return "";
  return toolchain::formatv(
             "BugSuppression::isSuppressed init suppressions cache for {0}",
             DeclWithIssue->getDeclKindName())
      .str();
}

toolchain::TimeTraceMetadata getDeclTimeTraceMetadata(const Decl *DeclWithIssue) {
  assert(DeclWithIssue);
  assert(toolchain::timeTraceProfilerEnabled());
  std::string Name = "<noname>";
  if (const auto *ND = dyn_cast<NamedDecl>(DeclWithIssue)) {
    Name = ND->getNameAsString();
  }
  const auto &SM = DeclWithIssue->getASTContext().getSourceManager();
  auto Line = SM.getPresumedLineNumber(DeclWithIssue->getBeginLoc());
  auto Fname = SM.getFilename(DeclWithIssue->getBeginLoc());
  return toolchain::TimeTraceMetadata{std::move(Name), Fname.str(),
                                 static_cast<int>(Line)};
}

} // end anonymous namespace

// TODO: Introduce stable IDs for checkers and check for those here
//       to be more specific.  Attribute without arguments should still
//       be considered as "suppress all".
//       It is already much finer granularity than what we have now
//       (i.e. removing the whole function from the analysis).
bool BugSuppression::isSuppressed(const BugReport &R) {
  PathDiagnosticLocation Location = R.getLocation();
  PathDiagnosticLocation UniqueingLocation = R.getUniqueingLocation();
  const Decl *DeclWithIssue = R.getDeclWithIssue();

  return isSuppressed(Location, DeclWithIssue, {}) ||
         isSuppressed(UniqueingLocation, DeclWithIssue, {});
}

bool BugSuppression::isSuppressed(const PathDiagnosticLocation &Location,
                                  const Decl *DeclWithIssue,
                                  DiagnosticIdentifierList Hashtags) {
  if (!Location.isValid())
    return false;

  if (!DeclWithIssue) {
    // FIXME: This defeats the purpose of passing DeclWithIssue to begin with.
    // If this branch is ever hit, we're re-doing all the work we've already
    // done as well as perform a lot of work we'll never need.
    // Gladly, none of our on-by-default checkers currently need it.
    DeclWithIssue = ACtx.getTranslationUnitDecl();
  } else {
    // This is the fast path. However, we should still consider the topmost
    // declaration that isn't TranslationUnitDecl, because we should respect
    // attributes on the entire declaration chain.
    while (true) {
      // Use the "lexical" parent. Eg., if the attribute is on a class, suppress
      // warnings in inline methods but not in out-of-line methods.
      const Decl *Parent =
          dyn_cast_or_null<Decl>(DeclWithIssue->getLexicalDeclContext());
      if (Parent == nullptr || isa<TranslationUnitDecl>(Parent))
        break;

      DeclWithIssue = Parent;
    }
  }

  // While some warnings are attached to AST nodes (mostly path-sensitive
  // checks), others are simply associated with a plain source location
  // or range.  Figuring out the node based on locations can be tricky,
  // so instead, we traverse the whole body of the declaration and gather
  // information on ALL suppressions.  After that we can simply check if
  // any of those suppressions affect the warning in question.
  //
  // Traversing AST of a function is not a heavy operation, but for
  // large functions with a lot of bugs it can make a dent in performance.
  // In order to avoid this scenario, we cache traversal results.
  auto InsertionResult = CachedSuppressionLocations.insert(
      std::make_pair(DeclWithIssue, CachedRanges{}));
  Ranges &SuppressionRanges = InsertionResult.first->second;
  if (InsertionResult.second) {
    toolchain::TimeTraceScope TimeScope(
        timeScopeName(DeclWithIssue),
        [DeclWithIssue]() { return getDeclTimeTraceMetadata(DeclWithIssue); });
    // We haven't checked this declaration for suppressions yet!
    CacheInitializer::initialize(DeclWithIssue, SuppressionRanges);
  }

  SourceRange BugRange = Location.asRange();
  const SourceManager &SM = Location.getManager();

  return toolchain::any_of(SuppressionRanges,
                      [BugRange, &SM](SourceRange Suppression) {
                        return fullyContains(Suppression, BugRange, SM);
                      });
}
