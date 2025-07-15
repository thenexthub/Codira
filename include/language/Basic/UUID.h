//===--- UUID.h - UUID generation -------------------------------*- C++ -*-===//
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
// This is an interface over the standard OSF uuid library that gives UUIDs
// sound value semantics and operators.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_UUID_H
#define LANGUAGE_BASIC_UUID_H

#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/raw_ostream.h"
#include <array>
#include <optional>

namespace language {
  
class UUID {
public:
  enum {
    /// The number of bytes in a UUID's binary representation.
    Size = 16,
    
    /// The number of characters in a UUID's string representation.
    StringSize = 36,
    
    /// The number of bytes necessary to store a null-terminated UUID's string
    /// representation.
    StringBufferSize = StringSize + 1,
  };
  
  unsigned char Value[Size];

private:
  enum FromRandom_t { FromRandom };
  enum FromTime_t { FromTime };

  UUID(FromRandom_t);
  
  UUID(FromTime_t);
  
public:
  /// Default constructor.
  UUID();
  
  UUID(std::array<unsigned char, Size> bytes) {
    memcpy(Value, &bytes, Size);
  }
  
  /// Create a new random UUID from entropy (/dev/random).
  static UUID fromRandom() { return UUID(FromRandom); }
  
  /// Create a new pseudorandom UUID using the time, MAC address, and pid.
  static UUID fromTime() { return UUID(FromTime); }
  
  /// Parse a UUID from a C string.
  static std::optional<UUID> fromString(const char *s);

  /// Convert a UUID to its string representation.
  void toString(toolchain::SmallVectorImpl<char> &out) const;
  
  int compare(UUID y) const;
  
#define COMPARE_UUID(op) \
  bool operator op(UUID y) { return compare(y) op 0; }
  
  COMPARE_UUID(==)
  COMPARE_UUID(!=)
  COMPARE_UUID(<)
  COMPARE_UUID(<=)
  COMPARE_UUID(>)
  COMPARE_UUID(>=)
#undef COMPARE_UUID
};
  
toolchain::raw_ostream &operator<<(toolchain::raw_ostream &os, UUID uuid);
  
}

namespace toolchain {
  template<>
  struct DenseMapInfo<language::UUID> {
    static inline language::UUID getEmptyKey() {
      return {{{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}}};
    }
    static inline language::UUID getTombstoneKey() {
      return {{{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
                0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE}}};
    }
    
    static unsigned getHashValue(language::UUID uuid) {
      union {
        language::UUID uu;
        unsigned words[4];
      } reinterpret = {uuid};
      return reinterpret.words[0] ^ reinterpret.words[1]
           ^ reinterpret.words[2] ^ reinterpret.words[3];
    }
    
    static bool isEqual(language::UUID a, language::UUID b) {
      return a == b;
    }
  };
} // end namespace language

#endif // LANGUAGE_BASIC_UUID_H
