//===--- Address.h - Address Representation ---------------------*- C++ -*-===//
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
// A structure for holding the address of an object.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_IRGEN_ADDRESS_H
#define LANGUAGE_IRGEN_ADDRESS_H

#include "IRGen.h"
#include "toolchain/ADT/ilist.h"
#include "toolchain/IR/Argument.h"
#include "toolchain/IR/DerivedTypes.h"
#include "toolchain/IR/Function.h"
#include "toolchain/IR/Instruction.h"
#include "toolchain/IR/Type.h"
#include "toolchain/IR/Value.h"

namespace language {
namespace irgen {

/// The address of an object in memory.
class Address {
  toolchain::Value *Addr;
  toolchain::Type *ElementType;
  Alignment Align;

public:
  Address() : Addr(nullptr) {}

  Address(toolchain::Value *addr, toolchain::Type *elementType, Alignment align)
      : Addr(addr), ElementType(elementType), Align(align) {
    if (addr == toolchain::DenseMapInfo<toolchain::Value *>::getEmptyKey() ||
        toolchain::DenseMapInfo<toolchain::Value *>::getTombstoneKey())
      return;
    assert(addr != nullptr && "building an invalid address");
  }

  toolchain::Value *operator->() const {
    assert(isValid());
    return getAddress();
  }

  bool isValid() const { return Addr != nullptr; }

  toolchain::Value *getAddress() const { return Addr; }

  Alignment getAlignment() const {
    return Align;
  }
  
  toolchain::PointerType *getType() const {
    return cast<toolchain::PointerType>(Addr->getType());
  }

  toolchain::Type *getElementType() const { return ElementType; }

  bool operator==(Address RHS) const {
    return Addr == RHS.Addr && ElementType == RHS.ElementType &&
           Align == RHS.Align;
  }
  bool operator!=(Address RHS) const { return !(*this == RHS); }
};

/// An address in memory together with the (possibly null) heap
/// allocation which owns it.
class OwnedAddress {
  Address Addr;
  toolchain::Value *Owner;

public:
  OwnedAddress() : Owner(nullptr) {}
  OwnedAddress(Address address, toolchain::Value *owner)
    : Addr(address), Owner(owner) {}

  toolchain::Value *getAddressPointer() const { return Addr.getAddress(); }
  Alignment getAlignment() const { return Addr.getAlignment(); }
  Address getAddress() const { return Addr; }
  toolchain::Value *getOwner() const { return Owner; }

  Address getUnownedAddress() const {
    assert(getOwner() == nullptr);
    return getAddress();
  }

  operator Address() const { return getAddress(); }

  bool isValid() const { return Addr.isValid(); }
};

/// An address in memory together with the local allocation which owns it.
class ContainedAddress {
  /// The address of an object of type T.
  Address Addr;

  /// The container of the address.
  Address Container;

public:
  ContainedAddress() {}
  ContainedAddress(Address container, Address address)
    : Addr(address), Container(container) {}

  toolchain::Value *getAddressPointer() const { return Addr.getAddress(); }
  Alignment getAlignment() const { return Addr.getAlignment(); }
  Address getAddress() const { return Addr; }
  Address getContainer() const { return Container; }

  bool isValid() const { return Addr.isValid(); }
};

/// An address on the stack together with an optional stack pointer reset
/// location.
class StackAddress {
  /// The address of an object of type T.
  Address Addr;

  /// In a normal function, the result of toolchain.stacksave or null.
  /// In a coroutine, the result of toolchain.coro.alloca.alloc.
  /// In an async function, the result of the taskAlloc call.
  toolchain::Value *ExtraInfo;

public:
  StackAddress() : ExtraInfo(nullptr) {}
  StackAddress(Address address, toolchain::Value *extraInfo = nullptr)
    : Addr(address), ExtraInfo(extraInfo) {}

  /// Return a StackAddress with the address changed in some superficial way.
  StackAddress withAddress(Address addr) const {
    return StackAddress(addr, ExtraInfo);
  }

  toolchain::Value *getAddressPointer() const { return Addr.getAddress(); }
  Alignment getAlignment() const { return Addr.getAlignment(); }
  Address getAddress() const { return Addr; }
  toolchain::Value *getExtraInfo() const { return ExtraInfo; }

  bool isValid() const { return Addr.isValid(); }

  bool operator==(StackAddress RHS) const {
    return Addr == RHS.Addr && ExtraInfo == RHS.ExtraInfo;
  }
  bool operator!=(StackAddress RHS) const { return !(*this == RHS); }
};

} // end namespace irgen
} // end namespace language

namespace toolchain {
template <>
struct DenseMapInfo<language::irgen::Address> {
  static language::irgen::Address getEmptyKey() {
    return language::irgen::Address(DenseMapInfo<toolchain::Value *>::getEmptyKey(),
                                 DenseMapInfo<toolchain::Type *>::getEmptyKey(),
                                 language::irgen::Alignment(8));
  }
  static language::irgen::Address getTombstoneKey() {
    return language::irgen::Address(DenseMapInfo<toolchain::Value *>::getTombstoneKey(),
                                 DenseMapInfo<toolchain::Type *>::getTombstoneKey(),
                                 language::irgen::Alignment(8));
  }
  static unsigned getHashValue(language::irgen::Address address) {
    return detail::combineHashValue(
        DenseMapInfo<toolchain::Value *>::getHashValue(address.getAddress()),
        detail::combineHashValue(
            DenseMapInfo<toolchain::Type *>::getHashValue(address.getElementType()),
            DenseMapInfo<language::irgen::Alignment::int_type>::getHashValue(
                address.getAlignment().getValue())));
  }
  static bool isEqual(language::irgen::Address LHS, language::irgen::Address RHS) {
    return LHS == RHS;
  }
};
template <>
struct DenseMapInfo<language::irgen::StackAddress> {
  static language::irgen::StackAddress getEmptyKey() {
    return language::irgen::StackAddress(
        DenseMapInfo<language::irgen::Address>::getEmptyKey(),
        DenseMapInfo<toolchain::Value *>::getEmptyKey());
  }
  static language::irgen::StackAddress getTombstoneKey() {
    return language::irgen::StackAddress(
        DenseMapInfo<language::irgen::Address>::getTombstoneKey(),
        DenseMapInfo<toolchain::Value *>::getTombstoneKey());
  }
  static unsigned getHashValue(language::irgen::StackAddress address) {
    return detail::combineHashValue(
        DenseMapInfo<language::irgen::Address>::getHashValue(address.getAddress()),
        DenseMapInfo<language::irgen::Alignment::int_type>::getHashValue(
            address.getAlignment().getValue()));
  }
  static bool isEqual(language::irgen::StackAddress LHS,
                      language::irgen::StackAddress RHS) {
    return LHS == RHS;
  }
};
} // end namespace toolchain

#endif
