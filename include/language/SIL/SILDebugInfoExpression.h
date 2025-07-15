//===--- SILDebugInfoExpression.h - DIExpression for SIL --------*- C++ -*-===//
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
/// This file contains types that model debug info expressions in SIL. Including
/// (debug info) operator and operand.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_DEBUGINFOEXPRESSION_H
#define LANGUAGE_SIL_DEBUGINFOEXPRESSION_H

#include "language/AST/Decl.h"
#include "toolchain/ADT/APInt.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/iterator_range.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>

namespace language {

class TailAllocatedDebugVariable;

/// Operator in a debug info expression
enum class SILDIExprOperator : unsigned {
  INVALID = 0,
  /// Dereferences the SSA value
  Dereference,
  /// Specifies that the SSA value is a fragment (sub-field) of the
  /// associated source variable. This operator takes a single
  /// VarDecl operand pointing to the field declaration.
  /// Note that this directive can only appear at the end of an
  /// expression, along with `TupleFragment`.
  Fragment,
  /// Perform arithmetic addition on the top two elements of the
  /// expression stack and push the result back to the stack.
  Plus,
  /// Subtract the top element in expression stack by the second
  /// element. Then push the result back to the stack.
  Minus,
  /// Push an unsigned integer constant onto the stack.
  ConstUInt,
  /// Push a signed integer constant onto the stack.
  ConstSInt,
  /// Specifies that the SSA value is an element of the
  /// associated tuple. This operator takes a TupleType
  /// operand pointing to the tuple type, and the index of the element.
  /// Note that this directive can only appear at the end of an
  /// expression, along with `Fragment`.
  TupleFragment
};

/// Represents a single component in a debug info expression.
/// Including operator and operand.
struct SILDIExprElement {
  enum Kind {
    /// A di-expression operator.
    OperatorKind,
    /// An operand that has declaration type.
    DeclKind,
    /// An integer constant value. Note that
    /// we don't specify its signedness here.
    ConstIntKind,
    /// An operand that has `Type` type.
    TypeKind
  };

private:
  Kind OpKind;

  union {
    SILDIExprOperator Operator;
    Decl *Declaration;
    uint64_t ConstantInt;
    Type TypePtr;
  };

  explicit SILDIExprElement(Kind OpK) : OpKind(OpK) {}

public:
  Kind getKind() const { return OpKind; }

  SILDIExprOperator getAsOperator() const {
    return OpKind == OperatorKind ? Operator : SILDIExprOperator::INVALID;
  }

  Decl *getAsDecl() const { return OpKind == DeclKind ? Declaration : nullptr; }

  std::optional<uint64_t> getAsConstInt() const {
    if (OpKind == ConstIntKind)
      return ConstantInt;
    else
      return {};
  }

  Type getAsType() const { return OpKind == TypeKind ? TypePtr : nullptr; }

  static SILDIExprElement createOperator(SILDIExprOperator Op) {
    SILDIExprElement DIOp(OperatorKind);
    DIOp.Operator = Op;
    return DIOp;
  }

  static SILDIExprElement createDecl(Decl *D) {
    SILDIExprElement DIOp(DeclKind);
    DIOp.Declaration = D;
    return DIOp;
  }

  static SILDIExprElement createConstInt(uint64_t V) {
    SILDIExprElement DIOp(ConstIntKind);
    DIOp.ConstantInt = V;
    return DIOp;
  }

  static SILDIExprElement createType(Type T) {
    SILDIExprElement DIOp(TypeKind);
    DIOp.TypePtr = T;
    return DIOp;
  }
};

/// Returns the hashcode for the di expr element.
inline toolchain::hash_code hash_value(const SILDIExprElement &elt) {
  return toolchain::hash_combine(elt.getKind(), elt.getAsDecl(), elt.getAsDecl(),
                            elt.getAsConstInt());
}

/// For a given SILDIExprOperator, provides information
/// like its textual name and operand types.
struct SILDIExprInfo {
  StringRef OpText;
  SmallVector<SILDIExprElement::Kind, 2> OperandKinds;

  static const SILDIExprInfo *get(SILDIExprOperator Op);
};

/// A DIExpr operand is consisting of a SILDIExprOperator and
/// SILDIExprElement arguments following after.
struct SILDIExprOperand : public toolchain::ArrayRef<SILDIExprElement> {
  // Reuse all the ctors
  using toolchain::ArrayRef<SILDIExprElement>::ArrayRef;

  SILDIExprOperator getOperator() const {
    assert(size() && "empty DIExpr operand");
    const SILDIExprElement &First = front();
    return First.getAsOperator();
  }

  size_t getNumArg() const {
    assert(size() && "empty DIExpr operand");
    return size() - 1;
  }

  toolchain::ArrayRef<SILDIExprElement> args() const {
    return drop_front();
  }
};

/// Represents a debug info expression in SIL
class SILDebugInfoExpression {
  friend class TailAllocatedDebugVariable;
  toolchain::SmallVector<SILDIExprElement, 2> Elements;

public:
  SILDebugInfoExpression() = default;

  explicit SILDebugInfoExpression(toolchain::ArrayRef<SILDIExprElement> EL)
      : Elements(EL.begin(), EL.end()) {}

  void clear() { Elements.clear(); }

  size_t getNumElements() const { return Elements.size(); }

  using iterator = typename decltype(Elements)::iterator;
  using const_iterator = typename decltype(Elements)::const_iterator;

  iterator element_begin() { return Elements.begin(); }
  iterator element_end() { return Elements.end(); }

  const_iterator element_begin() const { return Elements.begin(); }
  const_iterator element_end() const { return Elements.end(); }

  toolchain::iterator_range<iterator> elements() {
    return toolchain::make_range(element_begin(), element_end());
  }

  toolchain::iterator_range<const_iterator> elements() const {
    return toolchain::make_range(element_begin(), element_end());
  }

  const SILDIExprElement &getElement(size_t index) const {
    assert(index < Elements.size());
    return Elements[index];
  }

  void push_back(const SILDIExprElement &Element) {
    Elements.push_back(Element);
  }

  void appendElements(toolchain::ArrayRef<SILDIExprElement> NewElements) {
    if (NewElements.size())
      Elements.append(NewElements.begin(), NewElements.end());
  }

  void append(const SILDebugInfoExpression &Tail) {
    appendElements(Tail.Elements);
  }

  void prependElements(toolchain::ArrayRef<SILDIExprElement> NewElements) {
    Elements.insert(Elements.begin(),
                    NewElements.begin(), NewElements.end());
  }

  void eraseElement(const_iterator It) {
    Elements.erase(It);
  }

  /// The iterator for SILDIExprOperand
  class op_iterator {
    friend class SILDebugInfoExpression;

    SILDIExprOperand Current;
    toolchain::ArrayRef<SILDIExprElement> Remain;

    void increment();

    explicit
    op_iterator(toolchain::ArrayRef<SILDIExprElement> Remain): Remain(Remain) {
      increment();
    }

  public:
    op_iterator() = default;
    op_iterator(const op_iterator &) = default;

    const SILDIExprOperand &operator*() const { return Current; }
    const SILDIExprOperand *operator->() const { return &Current; }

    // Pre increment
    op_iterator &operator++() {
      increment();
      return *this;
    }

    // Post increment
    op_iterator operator++(int) {
      op_iterator This(*this);
      increment();
      return This;
    }

    bool operator==(const op_iterator &Other) const {
      return (Current.empty() && Other.Current.empty()) ||
             (Current.data() == Other.Current.data() &&
              Current.size() == Other.Current.size());
    }
    bool operator!=(const op_iterator &Other) const {
      return !(Other == *this);
    }
  };

  op_iterator operand_begin() const {
    return op_iterator(Elements);
  }
  op_iterator operand_end() const {
    return op_iterator(toolchain::ArrayRef<SILDIExprElement>{});
  }

  toolchain::iterator_range<op_iterator> operands() const {
    return toolchain::make_range(operand_begin(), operand_end());
  }

  /// Return true if this expression is not empty
  inline operator bool() const { return Elements.size(); }

  /// Create a `op_fragment` expression
  static SILDebugInfoExpression createFragment(VarDecl *Field);

  /// Create a `op_tuple_fragment` expression
  static
  SILDebugInfoExpression createTupleFragment(TupleType *TypePtr, unsigned Idx);

  /// Return true if this DIExpression starts with op_deref
  bool startsWithDeref() const {
    return Elements.size() &&
           Elements[0].getAsOperator() == SILDIExprOperator::Dereference;
  }

  /// Return the part of this SILDebugInfoExpression corresponding to fragments
  SILDebugInfoExpression getFragmentPart() const {
    for (auto it = element_begin(), end = element_end(); it != end; ++it) {
      if (it->getAsOperator() == SILDIExprOperator::Fragment
          || it->getAsOperator() == SILDIExprOperator::TupleFragment)
        return SILDebugInfoExpression(ArrayRef(it, element_end()));
    }
    return {};
  }
};

/// Returns the hashcode for the di expr element.
inline toolchain::hash_code hash_value(const SILDebugInfoExpression &elt) {
  return toolchain::hash_combine_range(elt.element_begin(), elt.element_end());
}

} // end namespace language

#endif
