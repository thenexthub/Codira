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

#include "Utils.h"
#include "language/AST/NameLookup.h"

using namespace language;
using namespace language::refactoring;

toolchain::StringRef
language::refactoring::correctNameInternal(ASTContext &Ctx, StringRef Name,
                                        ArrayRef<ValueDecl *> AllVisibles) {
  // If we find the collision.
  bool FoundCollision = false;

  // The suffixes we cannot use by appending to the original given name.
  toolchain::StringSet<> UsedSuffixes;
  for (auto VD : AllVisibles) {
    StringRef S = VD->getBaseName().userFacingName();
    if (!S.starts_with(Name))
      continue;
    StringRef Suffix = S.substr(Name.size());
    if (Suffix.empty())
      FoundCollision = true;
    else
      UsedSuffixes.insert(Suffix);
  }
  if (!FoundCollision)
    return Name;

  // Find the first suffix we can use.
  std::string SuffixToUse;
  for (unsigned I = 1;; I++) {
    SuffixToUse = std::to_string(I);
    if (UsedSuffixes.count(SuffixToUse) == 0)
      break;
  }
  return Ctx.getIdentifier((toolchain::Twine(Name) + SuffixToUse).str()).str();
}

toolchain::StringRef language::refactoring::correctNewDeclName(SourceLoc Loc,
                                                       DeclContext *DC,
                                                       StringRef Name) {

  // Collect all visible decls in the decl context.
  toolchain::SmallVector<ValueDecl *, 16> AllVisibles;
  VectorDeclConsumer Consumer(AllVisibles);
  ASTContext &Ctx = DC->getASTContext();
  lookupVisibleDecls(Consumer, Loc, DC, /*IncludeTopLevel*/ true);
  return correctNameInternal(Ctx, Name, AllVisibles);
}

/// If \p NTD is a protocol, return all the protocols it inherits from. If it's
/// a type, return all the protocols it conforms to.
SmallVector<ProtocolDecl *, 2>
language::refactoring::getAllProtocols(NominalTypeDecl *NTD) {
  if (auto Proto = dyn_cast<ProtocolDecl>(NTD)) {
    return SmallVector<ProtocolDecl *, 2>(
        Proto->getInheritedProtocols().begin(),
        Proto->getInheritedProtocols().end());
  } else {
    return NTD->getAllProtocols();
  }
}
