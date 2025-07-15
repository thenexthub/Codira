//===--- Explosion.h - Exploded R-Value Representation ----------*- C++ -*-===//
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
// A storage structure for holding an exploded r-value.  An exploded
// r-value has been separated into individual components.  Only types
// with non-resilient structure may be exploded.
//
// The standard use for explosions is for argument-passing.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_EXPLOSION_H
#define LANGUAGE_IRGEN_EXPLOSION_H

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "IRGen.h"
#include "IRGenFunction.h"

namespace language {
namespace irgen {

/// The representation for an explosion is just a list of raw LLVM
/// values.  The meaning of these values is imposed externally by the
/// type infos, except that it is expected that they will be passed
/// as arguments in exactly this way.
class Explosion {
  unsigned NextValue;
  SmallVector<toolchain::Value*, 8> Values;

public:
  Explosion() : NextValue(0) {}

  // We want to be a move-only type.
  Explosion(const Explosion &) = delete;
  Explosion &operator=(const Explosion &) = delete;
  Explosion(Explosion &&other) : NextValue(0) {
    // Do an uninitialized copy of the non-consumed elements.
    Values.resize_for_overwrite(other.size());
    std::uninitialized_copy(other.begin(), other.end(), Values.begin());

    // Remove everything from the other explosion.
    other.reset();
  }

  Explosion &operator=(Explosion &&o) {
    assert(empty() && "explosion had values remaining when reassigned!");
    NextValue = o.NextValue;
    Values.swap(o.Values);
    o.reset();
    return *this;
  }

  Explosion(toolchain::Value *singleValue) : NextValue(0) {
    add(singleValue);
  }

  ~Explosion() {
    assert(empty() && "explosion had values remaining when destroyed!");
  }

  bool empty() const {
    return NextValue == Values.size();
  }

  size_t size() const {
    return Values.size() - NextValue;
  }

  using iterator = SmallVector<toolchain::Value *, 8>::iterator;
  iterator begin() { return Values.begin() + NextValue; }
  iterator end() { return Values.end(); }

  using const_iterator = SmallVector<toolchain::Value *, 8>::const_iterator;
  const_iterator begin() const { return Values.begin() + NextValue; }
  const_iterator end() const { return Values.end(); }

  /// Add a value to the end of this exploded r-value.
  void add(toolchain::Value *value) {
    assert(value && "adding null value to explosion");
    assert(NextValue == 0 && "adding to partially-claimed explosion?");
    Values.push_back(value);
  }

  void add(ArrayRef<toolchain::Value*> values) {
#ifndef NDEBUG
    for (auto value : values)
      assert(value && "adding null value to explosion");
#endif
    assert(NextValue == 0 && "adding to partially-claimed explosion?");
    Values.append(values.begin(), values.end());
  }

  void insert(unsigned index, toolchain::Value *value) {
    Values.insert(Values.begin() + index, value);
  }

  /// Return an array containing the given range of values.  The values
  /// are not claimed.
  ArrayRef<toolchain::Value*> getRange(unsigned from, unsigned to) const {
    assert(from <= to);
    assert(to <= Values.size());
    return toolchain::ArrayRef(begin() + from, to - from);
  }

  /// Return an array containing all of the remaining values.  The values
  /// are not claimed.
  ArrayRef<toolchain::Value *> getAll() {
    return toolchain::ArrayRef(begin(), Values.size() - NextValue);
  }

  /// Transfer ownership of the next N values to the given explosion.
  void transferInto(Explosion &other, unsigned n) {
    other.add(claim(n));
  }


  /// The next N values have been claimed in some indirect way (e.g.
  /// using getRange() and the like); just give up on them.
  void markClaimed(unsigned n) {
    assert(NextValue + n <= Values.size());
    NextValue += n;
  }

  /// Claim and return the next value in this explosion.
  toolchain::Value *claimNext() {
    assert(NextValue < Values.size());
    return Values[NextValue++];
  }

  toolchain::Constant *claimNextConstant() {
    return cast<toolchain::Constant>(claimNext());
  }

  /// Claim and return the next N values in this explosion.
  ArrayRef<toolchain::Value*> claim(unsigned n) {
    assert(NextValue + n <= Values.size());
    auto array = toolchain::ArrayRef(begin(), n);
    NextValue += n;
    return array;
  }

  /// Claim and return all the values in this explosion.
  ArrayRef<toolchain::Value*> claimAll() {
    return claim(size());
  }

  // These are all kindof questionable.

  /// Without changing any state, take the last claimed value,
  /// if there is one.
  toolchain::Value *getLastClaimed() {
    assert(NextValue > 0);
    return Values[NextValue-1];
  }

  /// Claim and remove the last item in the array.
  /// Unlike the normal 'claim' methods, the item is gone forever.
  toolchain::Value *takeLast() {
    assert(!empty());
    auto result = Values.back();
    Values.pop_back();
    return result;
  }

  /// Reset this explosion.
  void reset() {
    NextValue = 0;
    Values.clear();
  }

  /// Do an operation while borrowing the values from this explosion.
  template <class Fn>
  void borrowing(Fn &&fn) {
    auto savedNextValue = NextValue;
    fn(*this);
    assert(empty() &&
           "didn't claim all values from the explosion during the borrow");
    NextValue = savedNextValue;
  }

  void print(toolchain::raw_ostream &OS);
  void dump();
};

/// An explosion schema is essentially the type of an Explosion.
class ExplosionSchema {
public:

  /// The schema for one atom of the explosion.
  class Element {
    toolchain::Type *Type;
    Alignment::int_type Align;
    Element() = default;
  public:
    static Element forScalar(toolchain::Type *type) {
      Element e;
      e.Type = type;
      e.Align = 0;
      return e;
    }

    static Element forAggregate(toolchain::Type *type, Alignment align) {
      assert(align.getValue() != 0 && "alignment with zero value!");
      Element e;
      e.Type = type;
      e.Align = align.getValue();
      return e;
    }

    bool isScalar() const { return Align == 0; }
    toolchain::Type *getScalarType() const { assert(isScalar()); return Type; }

    bool isAggregate() const { return !isScalar(); }
    toolchain::Type *getAggregateType() const {
      assert(isAggregate());
      return Type;
    }
    Alignment getAggregateAlignment() const {
      assert(isAggregate());
      return Alignment(Align);
    }
  };
  
private:
  SmallVector<Element, 8> Elements;
  bool ContainsAggregate;

public:
  ExplosionSchema() : ContainsAggregate(false) {}

  /// Return the number of elements in this schema.
  unsigned size() const { return Elements.size(); }
  bool empty() const { return Elements.empty(); }

  /// Does this schema contain an aggregate element?
  bool containsAggregate() const { return ContainsAggregate; }

  /// Does this schema consist solely of one aggregate element?
  bool isSingleAggregate() const {
    return size() == 1 && containsAggregate();
  }

  const Element &operator[](size_t index) const {
    return Elements[index];
  }

  using iterator = SmallVectorImpl<Element>::iterator;
  using const_iterator = SmallVectorImpl<Element>::const_iterator;

  iterator begin() { return Elements.begin(); }
  iterator end() { return Elements.end(); }
  const_iterator begin() const { return Elements.begin(); }
  const_iterator end() const { return Elements.end(); }

  void add(Element e) {
    Elements.push_back(e);
    ContainsAggregate |= e.isAggregate();
  }

  /// Produce the correct type for a direct return of this schema,
  /// which is assumed to contain only scalars.  This is defined as:
  ///   - void, if the schema is empty;
  ///   - the element type, if the schema contains exactly one element;
  ///   - an anonymous struct type concatenating those types, otherwise.
  toolchain::Type *getScalarResultType(IRGenModule &IGM) const;
};

/// A peepholed explosion of an optional scalar value.
class OptionalExplosion {
public:
  enum Kind {
    /// The value is known statically to be `Optional.none`.
    /// The explosion is empty.
    None,

    /// The value is known statically to be `Optional.some(x)`.
    /// The explosion is the wrapped value `x`.
    Some,

    /// It is unknown statically what case the optional is in.
    /// The explosion is an optional value.
    Optional
  };

private:
  Kind kind;
  Explosion value;

  OptionalExplosion(Kind kind) : kind(kind) {}

public:
  static OptionalExplosion forNone() {
    return None;
  }

  template <class Fn>
  static OptionalExplosion forSome(Fn &&fn) {
    OptionalExplosion result(Some);
    std::forward<Fn>(fn)(result.value);
    return result;
  }

  template <class Fn>
  static OptionalExplosion forOptional(Fn &&fn) {
    OptionalExplosion result(Optional);
    std::forward<Fn>(fn)(result.value);
    return result;
  }

  Kind getKind() const { return kind; }
  bool isNone() const { return kind == None; }
  bool isSome() const { return kind == Some; }
  bool isOptional() const { return kind == Optional; }

  Explosion &getSomeExplosion() {
    assert(kind == Some);
    return value;
  }
  Explosion &getOptionalExplosion() {
    assert(kind == Optional);
    return value;
  }
};

} // end namespace irgen
} // end namespace language

#endif
