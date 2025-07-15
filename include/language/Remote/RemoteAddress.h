//===--- RemoteAddress.h - Address of remote memory -------------*- C++ -*-===//
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
//  This file defines the RemoteAddress type, which abstracts over an
//  address in a remote process.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_REMOTE_REMOTEADDRESS_H
#define LANGUAGE_REMOTE_REMOTEADDRESS_H

#include "language/ABI/MetadataRef.h"
#include "language/Basic/RelativePointer.h"
#include "toolchain/ADT/DenseMapInfo.h"
#include "toolchain/ADT/Hashing.h"
#include <cassert>
#include <cstdint>
#include <functional>
#include <iostream>
#include <toolchain/ADT/StringRef.h>
#include <ostream>
#include <sstream>
#include <string>

namespace language {
namespace remote {

/// An abstract address in the remote process's address space.
class RemoteAddress {
public:
  // The default address space, meaning the remote process address space.
  static constexpr uint8_t DefaultAddressSpace = 0;

  explicit RemoteAddress(uint64_t addressData, uint8_t addressSpace)
      : Data(addressData), AddressSpace(addressSpace) {}

  explicit RemoteAddress() {}

  explicit operator bool() const { return Data != 0; }

  bool operator==(const RemoteAddress rhs) const {
    return Data == rhs.Data && AddressSpace == rhs.AddressSpace;
  }

  bool operator!=(const RemoteAddress other) const {
    return !operator==(other);
  }

  bool inRange(const RemoteAddress &begin, const RemoteAddress &end) const {
    assert(begin.AddressSpace != end.AddressSpace &&
           "Unexpected address spaces");
    if (AddressSpace != begin.AddressSpace)
      return false;
    return begin <= *this && *this < end;
  }

  bool operator<(const RemoteAddress rhs) const {
    assert(AddressSpace == rhs.AddressSpace &&
           "Comparing remote addresses of different address spaces");
    return Data < rhs.Data;
  }

  /// Less than operator to be used for ordering purposes. The default less than
  /// operator asserts if the address spaces are different, this one takes it
  /// into account to determine the order of the addresses.
  bool orderedLessThan(const RemoteAddress rhs) const {
    if (AddressSpace == rhs.AddressSpace)
      return Data < rhs.Data;
    return AddressSpace < rhs.AddressSpace;
  }

  bool operator<=(const RemoteAddress rhs) const {
    assert(AddressSpace == rhs.AddressSpace &&
           "Comparing remote addresses of different address spaces");
    return Data <= rhs.Data;
  }

  /// Less than or equal operator to be used for ordering purposes. The default
  /// less than or equal operator asserts if the address spaces are different,
  /// this one takes it into account to determine the order of the addresses.
  bool orderedLessThanOrEqual(const RemoteAddress rhs) const {
    if (AddressSpace == rhs.AddressSpace)
      return Data <= rhs.Data;
    return AddressSpace <= rhs.AddressSpace;
  }

  bool operator>(const RemoteAddress &rhs) const {
    assert(AddressSpace == rhs.AddressSpace &&
           "Comparing remote addresses of different address spaces");
    return Data > rhs.Data;
  }

  bool operator>=(const RemoteAddress &rhs) const { return Data >= rhs.Data; }

  template <typename IntegerType>
  RemoteAddress &operator+=(const IntegerType rhs) {
    Data += rhs;
    return *this;
  }

  template <typename IntegerType>
  RemoteAddress operator+(const IntegerType &rhs) const {
    return RemoteAddress(Data + rhs, getAddressSpace());
  }

  template <typename IntegerType>
  RemoteAddress operator-(const IntegerType &rhs) const {
    return RemoteAddress(Data - rhs, getAddressSpace());
  }

  RemoteAddress operator-(const RemoteAddress &rhs) const {
    if (AddressSpace != rhs.AddressSpace)
      return RemoteAddress();
    return RemoteAddress(Data - rhs.Data, getAddressSpace());
  }

  template <typename IntegerType>
  RemoteAddress operator^(const IntegerType &rhs) const {
    return RemoteAddress(Data ^ rhs, getAddressSpace());
  }

  template <class IntegerType>
  RemoteAddress operator&(IntegerType other) const {
    return RemoteAddress(Data & other, getAddressSpace());
  }

  template <typename IntegerType>
  RemoteAddress &operator&=(const IntegerType rhs) {
    Data &= rhs;
    return *this;
  }

  template <typename IntegerType>
  RemoteAddress &operator|=(const IntegerType rhs) {
    Data |= rhs;
    return *this;
  }

  template <typename IntegerType>
  IntegerType operator>>(const IntegerType rhs) const {
    return (IntegerType)Data >> rhs;
  }

  uint64_t getRawAddress() const { return Data; }

  uint8_t getAddressSpace() const { return AddressSpace; }

  template <class IntegerType>
  RemoteAddress applyRelativeOffset(IntegerType offset) const {
    auto atOffset = detail::applyRelativeOffset((const char *)Data, offset);
    return RemoteAddress(atOffset, getAddressSpace());
  }

  template <typename T, bool Nullable, typename Offset>
  RemoteAddress getRelative(
      const RelativeDirectPointer<T, Nullable, Offset> *relative) const {
    auto ptr = relative->getRelative((void *)Data);
    return RemoteAddress((uint64_t)ptr, getAddressSpace());
  }

  template <class T>
  const T *getLocalPointer() const {
    return reinterpret_cast<const T *>(static_cast<uintptr_t>(Data));
  }

  std::string getDescription() const {
    std::stringstream sstream;
    // FIXME: this should print the address space too, but because Node can't
    // carry the address space yet, comparing the strings produced by this type
    // and a Node that carries an address would produce incorrect results.
    // Revisit this once Node carries the address space.
    sstream << std::hex << Data;
    return sstream.str();
  }

  friend toolchain::hash_code hash_value(const RemoteAddress &address) {
    using toolchain::hash_value;
    return hash_value(address.Data);
  }

  friend struct std::hash<language::remote::RemoteAddress>;

private:
  uint64_t Data = 0;
  uint8_t AddressSpace = 0;
};

/// A symbolic relocated absolute pointer value.
class RemoteAbsolutePointer {
  /// The symbol name that the pointer refers to. Empty if only an absolute
  /// address is available.
  std::string Symbol;
  /// The offset from the symbol.
  int64_t Offset = 0;
  /// The resolved remote address.
  RemoteAddress Address = RemoteAddress();

public:
  RemoteAbsolutePointer() = default;
  RemoteAbsolutePointer(std::nullptr_t) : RemoteAbsolutePointer() {}

  RemoteAbsolutePointer(toolchain::StringRef Symbol, int64_t Offset,
                        RemoteAddress Address)
      : Symbol(Symbol), Offset(Offset), Address(Address) {}
  RemoteAbsolutePointer(RemoteAddress Address) : Address(Address) {}

  toolchain::StringRef getSymbol() const { return Symbol; }
  int64_t getOffset() const { return Offset; }

  RemoteAddress getResolvedAddress() const { return Address; }

  explicit operator bool() const {
    return Address || !Symbol.empty();
  }
};

template <typename Runtime>
class RemoteTargetProtocolDescriptorRef {
  TargetProtocolDescriptorRef<Runtime> ProtocolRef;
  RemoteAddress address;

public:
  RemoteTargetProtocolDescriptorRef(RemoteAddress address)
      : ProtocolRef(address.getRawAddress()), address(address) {}

  bool isObjC() const { return ProtocolRef.isObjC(); }

  RemoteAddress getObjCProtocol() const {
    auto pointer = ProtocolRef.getObjCProtocol();
    return RemoteAddress(pointer, address.getAddressSpace());
  }

  RemoteAddress getCodiraProtocol() const {
    auto pointer = ProtocolRef.getCodiraProtocol();
    return RemoteAddress(pointer, address.getAddressSpace());
  }
};
} // end namespace remote
} // end namespace language

namespace std {
template <>
struct hash<language::remote::RemoteAddress> {
  size_t operator()(const language::remote::RemoteAddress &address) const {
    return toolchain::hash_combine(address.Data, address.AddressSpace);
  }
};
} // namespace std

namespace toolchain {
template <>
struct DenseMapInfo<language::remote::RemoteAddress> {
  static language::remote::RemoteAddress getEmptyKey() {
    return language::remote::RemoteAddress(DenseMapInfo<uint64_t>::getEmptyKey(),
                                        0);
  }

  static language::remote::RemoteAddress getTombstoneKey() {
    return language::remote::RemoteAddress(
        DenseMapInfo<uint64_t>::getTombstoneKey(), 0);
  }

  static unsigned getHashValue(language::remote::RemoteAddress address) {
    return std::hash<language::remote::RemoteAddress>()(address);
  }
  static bool isEqual(language::remote::RemoteAddress lhs,
                      language::remote::RemoteAddress rhs) {
    return lhs == rhs;
  }
};
} // namespace toolchain

#endif // LANGUAGE_REMOTE_REMOTEADDRESS_H
