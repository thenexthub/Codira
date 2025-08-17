//===-- Value.h -------------------------------------------------*- C++ -*-===//
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
//
// This file defines classes for values computed by abstract interpretation
// during dataflow analysis.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_VALUE_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_VALUE_H

#include "language/Core/AST/Decl.h"
#include "language/Core/Analysis/FlowSensitive/Formula.h"
#include "language/Core/Analysis/FlowSensitive/StorageLocation.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/StringMap.h"
#include "toolchain/ADT/StringRef.h"
#include <cassert>
#include <utility>

namespace language::Core {
namespace dataflow {

/// Base class for all values computed by abstract interpretation.
///
/// Don't use `Value` instances by value. All `Value` instances are allocated
/// and owned by `DataflowAnalysisContext`.
class Value {
public:
  enum class Kind {
    Integer,
    Pointer,

    // TODO: Top values should not be need to be type-specific.
    TopBool,
    AtomicBool,
    FormulaBool,
  };

  explicit Value(Kind ValKind) : ValKind(ValKind) {}

  // Non-copyable because addresses of values are used as their identities
  // throughout framework and user code. The framework is responsible for
  // construction and destruction of values.
  Value(const Value &) = delete;
  Value &operator=(const Value &) = delete;

  virtual ~Value() = default;

  Kind getKind() const { return ValKind; }

  /// Returns the value of the synthetic property with the given `Name` or null
  /// if the property isn't assigned a value.
  Value *getProperty(toolchain::StringRef Name) const {
    return Properties.lookup(Name);
  }

  /// Assigns `Val` as the value of the synthetic property with the given
  /// `Name`.
  ///
  /// Properties may not be set on `RecordValue`s; use synthetic fields instead
  /// (for details, see documentation for `RecordStorageLocation`).
  void setProperty(toolchain::StringRef Name, Value &Val) {
    Properties.insert_or_assign(Name, &Val);
  }

  toolchain::iterator_range<toolchain::StringMap<Value *>::const_iterator>
  properties() const {
    return {Properties.begin(), Properties.end()};
  }

private:
  Kind ValKind;
  toolchain::StringMap<Value *> Properties;
};

/// An equivalence relation for values. It obeys reflexivity, symmetry and
/// transitivity. It does *not* include comparison of `Properties`.
///
/// Computes equivalence for these subclasses:
/// * PointerValue -- pointee locations are equal. Does not compute deep
///   equality of `Value` at said location.
/// * TopBoolValue -- both are `TopBoolValue`s.
///
/// Otherwise, falls back to pointer equality.
bool areEquivalentValues(const Value &Val1, const Value &Val2);

/// Models a boolean.
class BoolValue : public Value {
  const Formula *F;

public:
  explicit BoolValue(Kind ValueKind, const Formula &F)
      : Value(ValueKind), F(&F) {}

  static bool classof(const Value *Val) {
    return Val->getKind() == Kind::TopBool ||
           Val->getKind() == Kind::AtomicBool ||
           Val->getKind() == Kind::FormulaBool;
  }

  const Formula &formula() const { return *F; }
};

/// A TopBoolValue represents a boolean that is explicitly unconstrained.
///
/// This is equivalent to an AtomicBoolValue that does not appear anywhere
/// else in a system of formula.
/// Knowing the value is unconstrained is useful when e.g. reasoning about
/// convergence.
class TopBoolValue final : public BoolValue {
public:
  TopBoolValue(const Formula &F) : BoolValue(Kind::TopBool, F) {
    assert(F.kind() == Formula::AtomRef);
  }

  static bool classof(const Value *Val) {
    return Val->getKind() == Kind::TopBool;
  }

  Atom getAtom() const { return formula().getAtom(); }
};

/// Models an atomic boolean.
///
/// FIXME: Merge this class into FormulaBoolValue.
///        When we want to specify atom identity, use Atom.
class AtomicBoolValue final : public BoolValue {
public:
  explicit AtomicBoolValue(const Formula &F) : BoolValue(Kind::AtomicBool, F) {
    assert(F.kind() == Formula::AtomRef);
  }

  static bool classof(const Value *Val) {
    return Val->getKind() == Kind::AtomicBool;
  }

  Atom getAtom() const { return formula().getAtom(); }
};

/// Models a compound boolean formula.
class FormulaBoolValue final : public BoolValue {
public:
  explicit FormulaBoolValue(const Formula &F)
      : BoolValue(Kind::FormulaBool, F) {
    assert(F.kind() != Formula::AtomRef && "For now, use AtomicBoolValue");
  }

  static bool classof(const Value *Val) {
    return Val->getKind() == Kind::FormulaBool;
  }
};

/// Models an integer.
class IntegerValue : public Value {
public:
  explicit IntegerValue() : Value(Kind::Integer) {}

  static bool classof(const Value *Val) {
    return Val->getKind() == Kind::Integer;
  }
};

/// Models a symbolic pointer. Specifically, any value of type `T*`.
class PointerValue final : public Value {
public:
  explicit PointerValue(StorageLocation &PointeeLoc)
      : Value(Kind::Pointer), PointeeLoc(PointeeLoc) {}

  static bool classof(const Value *Val) {
    return Val->getKind() == Kind::Pointer;
  }

  StorageLocation &getPointeeLoc() const { return PointeeLoc; }

private:
  StorageLocation &PointeeLoc;
};

raw_ostream &operator<<(raw_ostream &OS, const Value &Val);

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_VALUE_H
