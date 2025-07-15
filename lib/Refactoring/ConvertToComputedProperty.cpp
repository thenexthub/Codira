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

#include "RefactoringActions.h"
#include "language/AST/TypeRepr.h"

using namespace language::refactoring;

bool RefactoringActionConvertToComputedProperty::isApplicable(
    const ResolvedRangeInfo &Info, DiagnosticEngine &Diag) {
  if (Info.Kind != RangeKind::SingleDecl) {
    return false;
  }

  if (Info.ContainedNodes.size() != 1) {
    return false;
  }

  auto D = Info.ContainedNodes[0].dyn_cast<Decl *>();
  if (!D) {
    return false;
  }

  auto Binding = dyn_cast<PatternBindingDecl>(D);
  if (!Binding) {
    return false;
  }

  auto SV = Binding->getSingleVar();
  if (!SV) {
    return false;
  }

  // willSet, didSet cannot be provided together with a getter
  for (auto AD : SV->getAllAccessors()) {
    if (AD->isObservingAccessor()) {
      return false;
    }
  }

  // 'lazy' must not be used on a computed property
  // NSCopying and IBOutlet attribute requires property to be mutable
  auto Attributies = SV->getAttrs();
  if (Attributies.hasAttribute<LazyAttr>() ||
      Attributies.hasAttribute<NSCopyingAttr>() ||
      Attributies.hasAttribute<IBOutletAttr>()) {
    return false;
  }

  // Property wrapper cannot be applied to a computed property
  if (SV->hasAttachedPropertyWrapper()) {
    return false;
  }

  // has an initializer
  return Binding->hasInitStringRepresentation(0);
}

bool RefactoringActionConvertToComputedProperty::performChange() {
  // Get an initialization
  auto D = RangeInfo.ContainedNodes[0].dyn_cast<Decl *>();
  auto Binding = dyn_cast<PatternBindingDecl>(D);
  SmallString<128> scratch;
  auto Init = Binding->getInitStringRepresentation(0, scratch);

  // Get type
  auto SV = Binding->getSingleVar();
  auto SVType = SV->getTypeInContext();
  auto TR = SV->getTypeReprOrParentPatternTypeRepr();

  SmallString<64> DeclBuffer;
  toolchain::raw_svector_ostream OS(DeclBuffer);
  StringRef Space = " ";
  StringRef NewLine = "\n";

  OS << tok::kw_var << Space;
  // Add var name
  OS << SV->getNameStr().str() << ":" << Space;
  // For computed property must write a type of var
  if (TR) {
    OS << Lexer::getCharSourceRangeFromSourceRange(SM, TR->getSourceRange())
              .str();
  } else {
    SVType.print(OS);
  }

  OS << Space << tok::l_brace << NewLine;
  // Add an initialization
  OS << tok::kw_return << Space << Init.str() << NewLine;
  OS << tok::r_brace;

  // Replace initializer to computed property
  auto ReplaceStartLoc = Binding->getLoc();
  auto ReplaceEndLoc = Binding->getSourceRange().End;
  auto ReplaceRange = SourceRange(ReplaceStartLoc, ReplaceEndLoc);
  auto ReplaceCharSourceRange =
      Lexer::getCharSourceRangeFromSourceRange(SM, ReplaceRange);
  EditConsumer.accept(SM, ReplaceCharSourceRange, DeclBuffer.str());
  return false; // success
}
