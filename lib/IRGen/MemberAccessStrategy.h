//===--- MemberAccessStrategy.h - Abstract member access --------*- C++ -*-===//
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
//  This file defines an interface for summarizing how to access a
//  particular physical member.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_MEMBERACCESSSTRATEGY_H
#define LANGUAGE_IRGEN_MEMBERACCESSSTRATEGY_H

#include <string>
#include "IRGen.h"

namespace language {
namespace irgen {

/// A class which describes how to access a particular physical member.
class MemberAccessStrategy {
public:
  enum class OffsetKind : uint8_t {
    /// A word-size offset that represents a number of bytes.
    Bytes_Word,
  };

  enum class Kind : uint8_t {
    /// The member has complex or overlapping storage.  It cannot be
    /// accessed simply by projecting out to a particular offset.
    Complex,

    /// The member is at an offset which is statically known.
    DirectFixed,

    /// The member is at an offset which must be loaded from global memory.
    DirectGlobal,

    /// The member is at an offset which must be loaded from a
    /// statically-known offset within the type's metadata.
    IndirectFixed,

    /// The member is at an offset which must be loaded from an offset
    /// within the type's metadata, which itself must be loaded from
    /// a global symbol.
    IndirectGlobal,
  };

private:
  union StorageType {
    std::string GlobalSymbol;
    Size FixedOffset;

    StorageType() {}
    ~StorageType() {}
  } Storage;
  Kind TheKind;
  OffsetKind DirectOffsetKind;
  OffsetKind IndirectOffsetKind;

  explicit MemberAccessStrategy(Kind kind) : TheKind(kind) {}

public:
  // These are implementable, but there's no need.
  MemberAccessStrategy(const MemberAccessStrategy &other) = delete;
  MemberAccessStrategy &operator=(const MemberAccessStrategy &other) = delete;

  MemberAccessStrategy(MemberAccessStrategy &&other)
      : TheKind(other.TheKind) {
    switch (TheKind) {
    case Kind::Complex:
      return;
    case Kind::DirectFixed:
      Storage.FixedOffset = other.Storage.FixedOffset;
      return;
    case Kind::DirectGlobal:
      DirectOffsetKind = other.DirectOffsetKind;
      ::new (&Storage.GlobalSymbol)
          std::string(std::move(other.Storage.GlobalSymbol));
      return;
    case Kind::IndirectFixed:
      DirectOffsetKind = other.DirectOffsetKind;
      Storage.FixedOffset = other.Storage.FixedOffset;
      return;
    case Kind::IndirectGlobal:
      DirectOffsetKind = other.DirectOffsetKind;
      IndirectOffsetKind = other.IndirectOffsetKind;
      ::new (&Storage.GlobalSymbol)
          std::string(std::move(other.Storage.GlobalSymbol));
      return;
    }
    toolchain_unreachable("bad member access strategy kind");
  }

  MemberAccessStrategy &operator=(MemberAccessStrategy &&other) {
    this->~MemberAccessStrategy();
    ::new (this) MemberAccessStrategy(std::move(other));
    return *this;
  }

  ~MemberAccessStrategy() {
    switch (TheKind) {
    case Kind::Complex:
    case Kind::DirectFixed:
    case Kind::IndirectFixed:
      return;
    case Kind::DirectGlobal:
    case Kind::IndirectGlobal:
      Storage.GlobalSymbol.~basic_string();
      return;
    }
    toolchain_unreachable("bad member access strategy kind");
  }

  static MemberAccessStrategy
  getComplex() {
    return MemberAccessStrategy(Kind::Complex);
  }

  static MemberAccessStrategy
  getDirectFixed(Size directOffset) {
    MemberAccessStrategy result(Kind::DirectFixed);
    result.Storage.FixedOffset = directOffset;
    return result;
  }

  static MemberAccessStrategy
  getDirectGlobal(std::string &&directOffsetSymbol,
                  OffsetKind directOffsetKind) {
    MemberAccessStrategy result(Kind::DirectGlobal);
    ::new(&result.Storage.GlobalSymbol)
        std::string(std::move(directOffsetSymbol));
    result.DirectOffsetKind = directOffsetKind;
    return result;
  }

  static MemberAccessStrategy
  getIndirectFixed(Size indirectOffset, OffsetKind directOffsetKind) {
    MemberAccessStrategy result(Kind::IndirectFixed);
    result.Storage.FixedOffset = indirectOffset;
    result.DirectOffsetKind = directOffsetKind;
    return result;
  }

  static MemberAccessStrategy
  getIndirectGlobal(std::string &&indirectOffsetSymbol,
                    OffsetKind indirectOffsetKind,
                    OffsetKind directOffsetKind) {
    MemberAccessStrategy result(Kind::DirectGlobal);
    ::new(&result.Storage.GlobalSymbol)
        std::string(std::move(indirectOffsetSymbol));
    result.IndirectOffsetKind = indirectOffsetKind;
    result.DirectOffsetKind = directOffsetKind;
    return result;
  }

  Kind getKind() const { return TheKind; }
  OffsetKind getDirectOffsetKind() const {
    assert(TheKind == Kind::DirectGlobal ||
           TheKind == Kind::IndirectFixed ||
           TheKind == Kind::IndirectGlobal);
    return DirectOffsetKind;
  }
  OffsetKind getIndirectOffsetKind() const {
    assert(TheKind == Kind::IndirectGlobal);
    return IndirectOffsetKind;
  }

  Size getDirectOffset() const {
    assert(TheKind == Kind::DirectFixed);
    return Storage.FixedOffset;
  }

  Size getIndirectOffset() const {
    assert(TheKind == Kind::IndirectFixed);
    return Storage.FixedOffset;
  }

  const std::string &getDirectGlobalSymbol() const {
    assert(TheKind == Kind::DirectGlobal);
    return Storage.GlobalSymbol;
  }

  const std::string &getIndirectGlobalSymbol() const {
    assert(TheKind == Kind::IndirectGlobal);
    return Storage.GlobalSymbol;
  }
};

} // end namespace irgen
} // end namespace language

#endif
