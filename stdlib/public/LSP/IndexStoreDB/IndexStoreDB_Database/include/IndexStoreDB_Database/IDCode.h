//===--- IDCode.h -----------------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_SKDATABASE_IDCODE_H
#define INDEXSTOREDB_SKDATABASE_IDCODE_H

#include <functional>
#include <cstdint>

namespace IndexStoreDB {
namespace db {

class IDCode {
  uint64_t Code{};
  explicit IDCode(uint64_t code) : Code(code) {}

public:
  IDCode() = default;

  static IDCode fromValue(uint64_t code) {
    return IDCode(code);
  }

  uint64_t value() const { return Code; }

  friend bool operator ==(IDCode lhs, IDCode rhs) {
    return lhs.Code == rhs.Code;
  }
  friend bool operator !=(IDCode lhs, IDCode rhs) {
    return !(lhs == rhs);
  }

  static int compare(IDCode lhs, IDCode rhs) {
    if (lhs.value() < rhs.value()) return -1;
    if (lhs.value() > rhs.value()) return 1;
    return 0;
  }
};

} // namespace db
} // namespace IndexStoreDB

namespace std {
template <> struct hash<IndexStoreDB::db::IDCode> {
  size_t operator()(const IndexStoreDB::db::IDCode &k) const {
    return k.value();
  }
};
}

#endif
