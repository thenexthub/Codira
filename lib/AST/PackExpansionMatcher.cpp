//===--- PackExpansionMatcher.cpp - Matching pack expansions --------------===//
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
// Utilities for structural matching of sequences of types containing pack
// expansions.
//
//===----------------------------------------------------------------------===//

#include "language/AST/PackExpansionMatcher.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Type.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "toolchain/ADT/SmallVector.h"
#include <algorithm>

using namespace language;

template <>
Identifier TypeListPackMatcher<TupleTypeElt>::getElementLabel(
    const TupleTypeElt &elt) const {
  return elt.getName();
}

template <>
Type TypeListPackMatcher<TupleTypeElt>::getElementType(
    const TupleTypeElt &elt) const {
  return elt.getType();
}

template <>
ParameterTypeFlags TypeListPackMatcher<TupleTypeElt>::getElementFlags(
    const TupleTypeElt &elt) const {
  return ParameterTypeFlags();
}

template <>
Identifier TypeListPackMatcher<AnyFunctionType::Param>::getElementLabel(
    const AnyFunctionType::Param &elt) const {
  return elt.getLabel();
}

template <>
Type TypeListPackMatcher<AnyFunctionType::Param>::getElementType(
    const AnyFunctionType::Param &elt) const {
  return elt.getPlainType();
}

template <>
ParameterTypeFlags TypeListPackMatcher<AnyFunctionType::Param>::getElementFlags(
    const AnyFunctionType::Param &elt) const {
  return elt.getParameterFlags();
}

template <>
Identifier TypeListPackMatcher<Type>::getElementLabel(const Type &elt) const {
  return Identifier();
}

template <>
Type TypeListPackMatcher<Type>::getElementType(const Type &elt) const {
  return elt;
}

template <>
ParameterTypeFlags
TypeListPackMatcher<Type>::getElementFlags(const Type &elt) const {
  return ParameterTypeFlags();
}
