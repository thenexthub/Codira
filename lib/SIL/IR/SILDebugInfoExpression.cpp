//===-------- SILDebugInfoExpression.cpp - DIExpression for SIL -----------===//
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
///
/// \file
/// This file contains the table used by SILDIExprInfo to map from
/// SILDIExprOperator to supplement information like the operator string.
///
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/SIL/SILDebugInfoExpression.h"
#include <unordered_map>

using namespace language;

namespace std {
// This is, unfortunately, a workaround for an ancient bug in libstdc++:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60970
// Which prevented std::hash<T> users from having drop-in support for
// enum class types.
template <> struct hash<SILDIExprOperator> {
  size_t operator()(SILDIExprOperator V) const noexcept {
    return std::hash<unsigned>{}(static_cast<unsigned>(V));
  }
};
} // end namespace std

const SILDIExprInfo *SILDIExprInfo::get(SILDIExprOperator Op) {
  static const std::unordered_map<SILDIExprOperator, SILDIExprInfo> Infos = {
      {SILDIExprOperator::Fragment,
       {"op_fragment", {SILDIExprElement::DeclKind}}},
      {SILDIExprOperator::TupleFragment,
       {"op_tuple_fragment",
         {SILDIExprElement::TypeKind, SILDIExprElement::ConstIntKind}}},
      {SILDIExprOperator::Dereference, {"op_deref", {}}},
      {SILDIExprOperator::Plus, {"op_plus", {}}},
      {SILDIExprOperator::Minus, {"op_minus", {}}},
      {SILDIExprOperator::ConstUInt,
       {"op_constu", {SILDIExprElement::ConstIntKind}}},
      {SILDIExprOperator::ConstSInt,
       {"op_consts", {SILDIExprElement::ConstIntKind}}}};

  return Infos.count(Op) ? &Infos.at(Op) : nullptr;
}

void SILDebugInfoExpression::op_iterator::increment() {
  if (Remain.empty()) {
    // Effectively making this an end iterator
    Current = {};
    return;
  }

  const SILDIExprElement &NextHead = Remain[0];
  SILDIExprOperator Op = NextHead.getAsOperator();
  if (const auto *ExprInfo = SILDIExprInfo::get(Op)) {
    auto NewSlice = Remain.take_front(ExprInfo->OperandKinds.size() + 1);
    Current = SILDIExprOperand(NewSlice.data(), NewSlice.size());
    if (Remain.size() >= Current.size())
      Remain = Remain.drop_front(Current.size());
  }
}

SILDebugInfoExpression SILDebugInfoExpression::createFragment(VarDecl *Field) {
  assert(Field);
  return SILDebugInfoExpression(
      {SILDIExprElement::createOperator(SILDIExprOperator::Fragment),
       SILDIExprElement::createDecl(Field)});
}

SILDebugInfoExpression
SILDebugInfoExpression::createTupleFragment(TupleType *TypePtr, unsigned Idx) {
  assert(TypePtr);
  return SILDebugInfoExpression(
      {SILDIExprElement::createOperator(SILDIExprOperator::TupleFragment),
       SILDIExprElement::createType(TypePtr),
       SILDIExprElement::createConstInt(Idx)});
}

