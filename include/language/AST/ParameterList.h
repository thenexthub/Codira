//===--- ParameterList.h - Functions & closures parameter lists -*- C++ -*-===//
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
// This file defines the ParameterList class and support logic.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_PARAMETERLIST_H
#define LANGUAGE_AST_PARAMETERLIST_H

#include "language/AST/Decl.h"
#include "language/Basic/Debug.h"
#include "language/Basic/OptionSet.h"
#include "toolchain/Support/TrailingObjects.h"

namespace language {

class SubstitutionMap;

/// This describes a list of parameters.  Each parameter descriptor is tail
/// allocated onto this list.
class alignas(ParamDecl *) ParameterList final :
    // FIXME: Do we really just want to allocate these pointer-aligned?
    public ASTAllocated<std::aligned_storage<8, 8>::type>,
    private toolchain::TrailingObjects<ParameterList, ParamDecl *> {
  friend TrailingObjects;

  SourceLoc LParenLoc, RParenLoc;
  size_t numParameters;

  ParameterList(SourceLoc LParenLoc, size_t numParameters, SourceLoc RParenLoc)
    : LParenLoc(LParenLoc), RParenLoc(RParenLoc), numParameters(numParameters){}
  void operator=(const ParameterList&) = delete;
public:
  /// Create a parameter list with the specified parameters.
  static ParameterList *create(const ASTContext &C, SourceLoc LParenLoc,
                               ArrayRef<ParamDecl*> params,
                               SourceLoc RParenLoc);

  /// Create a parameter list with the specified parameters, with no location
  /// info for the parens.
  static ParameterList *create(const ASTContext &C,
                               ArrayRef<ParamDecl*> params) {
    return create(C, SourceLoc(), params, SourceLoc());
  }
 
  /// Create an empty parameter list.
  static ParameterList *createEmpty(const ASTContext &C,
                                    SourceLoc LParenLoc = SourceLoc(),
                                    SourceLoc RParenLoc = SourceLoc()) {
    return create(C, LParenLoc, {}, RParenLoc);
  }
  
  /// Create a parameter list for a single parameter lacking location info.
  static ParameterList *createWithoutLoc(ParamDecl *decl) {
    return create(decl->getASTContext(), decl);
  }

  static ParameterList *clone(const ASTContext &Ctx, ParameterList *PL) {
    SmallVector<ParamDecl*, 4> params;
    params.reserve(PL->size());
    for (auto p : *PL)
      params.push_back(ParamDecl::clone(Ctx, p));
    return ParameterList::create(Ctx, params);
  }

  SourceLoc getLParenLoc() const { return LParenLoc; }
  SourceLoc getRParenLoc() const { return RParenLoc; }
  
  typedef MutableArrayRef<ParamDecl*>::iterator iterator;
  typedef ArrayRef<ParamDecl*>::iterator const_iterator;
  iterator begin() { return getArray().begin(); }
  iterator end() { return getArray().end(); }
  const_iterator begin() const { return getArray().begin(); }
  const_iterator end() const { return getArray().end(); }

  ParamDecl *front() const { return getArray().front(); }
  ParamDecl *back() const { return getArray().back(); }

  MutableArrayRef<ParamDecl*> getArray() {
    return {getTrailingObjects<ParamDecl*>(), numParameters};
  }
  ArrayRef<ParamDecl*> getArray() const {
    return {getTrailingObjects<ParamDecl*>(), numParameters};
  }

  size_t size() const {
    return numParameters;
  }
  
  const ParamDecl *get(unsigned i) const {
    return getArray()[i];
  }
  
  ParamDecl *&get(unsigned i) {
    return getArray()[i];
  }

  const ParamDecl *operator[](unsigned i) const { return get(i); }
  ParamDecl *&operator[](unsigned i) { return get(i); }
  bool hasInternalParameter(StringRef prefix) const;

  /// Change the DeclContext of any contained parameters to the specified
  /// DeclContext.
  void setDeclContextOfParamDecls(DeclContext *DC);

  /// Flags used to indicate how ParameterList cloning should operate.
  enum CloneFlags {
    /// The cloned ParamDecls should be marked implicit.
    Implicit = 0x01,
    /// Mark default arguments as inherited.
    Inherited = 0x02,
    /// Mark unnamed arguments as named.
    NamedArguments = 0x04,
  };

  friend OptionSet<CloneFlags> operator|(CloneFlags flag1, CloneFlags flag2) {
    return OptionSet<CloneFlags>(flag1) | flag2;
  }

  /// Make a duplicate copy of this parameter list.  This allocates copies of
  /// the ParamDecls, so they can be reparented into a new DeclContext.
  ParameterList *clone(const ASTContext &C,
                       OptionSet<CloneFlags> options = std::nullopt) const;

  /// Return a list of function parameters for this parameter list,
  /// based on the interface types of the parameters in this list.
  void getParams(SmallVectorImpl<AnyFunctionType::Param> &params) const;

  unsigned getOrigParamIndex(SubstitutionMap subMap, unsigned substIndex) const;

  /// Return the full source range of this parameter.
  SourceRange getSourceRange() const;
  SourceLoc getStartLoc() const { return getSourceRange().Start; }
  SourceLoc getEndLoc() const { return getSourceRange().End; }

  LANGUAGE_DEBUG_DUMP;
  void dump(raw_ostream &OS, unsigned Indent = 0) const;
  
  //  void print(raw_ostream &OS) const;
};

} // end namespace language

#endif
