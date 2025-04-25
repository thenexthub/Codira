//===--- Parameter.cpp - Functions & closures parameters ------------------===//
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
// This file defines the Parameter class, the ParameterList class and support
// logic.
//
//===----------------------------------------------------------------------===//

#include "language/AST/ParameterList.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Expr.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
using namespace language;

/// TODO: unique and reuse the () parameter list in ASTContext, it is common to
/// many methods.  Other parameter lists cannot be uniqued because the decls
/// within them are always different anyway (they have different DeclContext's).
ParameterList *
ParameterList::create(const ASTContext &C, SourceLoc LParenLoc,
                      ArrayRef<ParamDecl*> params, SourceLoc RParenLoc) {
  assert(LParenLoc.isValid() == RParenLoc.isValid() &&
         "Either both paren locs are valid or neither are");
  
  auto byteSize = totalSizeToAlloc<ParamDecl *>(params.size());
  auto rawMem = C.Allocate(byteSize, alignof(ParameterList));
  
  //  Placement initialize the ParameterList and the Parameter's.
  auto PL = ::new (rawMem) ParameterList(LParenLoc, params.size(), RParenLoc);

  std::uninitialized_copy(params.begin(), params.end(), PL->getArray().begin());

  return PL;
}

/// Change the DeclContext of any contained parameters to the specified
/// DeclContext.
void ParameterList::setDeclContextOfParamDecls(DeclContext *DC) {
  for (ParamDecl *P : *this) {
    P->setDeclContext(DC);
    if (auto initContext = P->getCachedDefaultArgumentInitContext())
      (*initContext)->changeFunction(DC);
  }
}

/// Make a duplicate copy of this parameter list.  This allocates copies of
/// the ParamDecls, so they can be reparented into a new DeclContext.
ParameterList *ParameterList::clone(const ASTContext &C,
                                    OptionSet<CloneFlags> options) const {
  // TODO(distributed): copy types thanks to flag in options
  // If this list is empty, don't actually bother with a copy.
  if (size() == 0)
    return const_cast<ParameterList*>(this);
  
  SmallVector<ParamDecl*, 8> params(begin(), end());

  // Remap the ParamDecls inside of the ParameterList.
  unsigned i = 0;
  for (auto &decl : params) {
    auto defaultArgKind = decl->getDefaultArgumentKind();

    // If we're inheriting a default argument, mark it as such.
    // FIXME: Figure out how to clone default arguments as well.
    if (options & Inherited) {
      switch (defaultArgKind) {
      case DefaultArgumentKind::Normal:
      case DefaultArgumentKind::StoredProperty:
        defaultArgKind = DefaultArgumentKind::Inherited;
        break;

      default:
        break;
      }
    } else {
      defaultArgKind = DefaultArgumentKind::None;
    }

    decl = ParamDecl::cloneWithoutType(C, decl, defaultArgKind);
    if (options & Implicit)
      decl->setImplicit();

    // If the argument isn't named, give the parameter a name so that
    // silgen will produce a value for it.
    if (decl->getName().empty() && (options & NamedArguments)) {
      llvm::SmallString<16> s;
      { llvm::raw_svector_ostream(s) << "__argument" << ++i; }
      decl->setName(C.getIdentifier(s));
    }
  }
  
  return create(C, params);
}

void ParameterList::getParams(
                        SmallVectorImpl<AnyFunctionType::Param> &params) const {
  for (auto P : *this)
    params.push_back(P->toFunctionParam());
}


/// Return the full source range of this parameter list.
SourceRange ParameterList::getSourceRange() const {
  // If we have locations for the parens, then they define our range.
  if (LParenLoc.isValid())
    return { LParenLoc, RParenLoc };
  
  // Otherwise, try the first and last parameter.
  if (size() != 0) {
    auto Start = get(0)->getStartLoc();
    auto End = getArray().back()->getEndLoc();
    if (Start.isValid() && End.isValid())
      return { Start, End };
  }

  return SourceRange();
}

