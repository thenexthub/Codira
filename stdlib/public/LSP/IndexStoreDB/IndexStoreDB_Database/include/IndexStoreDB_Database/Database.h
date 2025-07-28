//===--- Database.h ---------------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_SKDATABASE_DATABASE_H
#define INDEXSTOREDB_SKDATABASE_DATABASE_H

#include <IndexStoreDB_Database/IDCode.h>
#include <IndexStoreDB_Support/LLVM.h>
#include <IndexStoreDB_Support/Visibility.h>
#include <memory>
#include <string>

namespace IndexStoreDB {
namespace db {
  class Database;
  typedef std::shared_ptr<Database> DatabaseRef;

class INDEXSTOREDB_EXPORT Database {
public:
  static DatabaseRef create(StringRef dbPath, bool readonly, Optional<size_t> initialDBSize, std::string &error);
  ~Database();

  void increaseMapSize();

  void printStats(raw_ostream &OS);

  class Implementation;
private:
  std::shared_ptr<Implementation> Impl;

public:
  // This is public for easier access from underlying code.
  Implementation &impl() { return *Impl; }

  // This is public for testing.
  static const unsigned DATABASE_FORMAT_VERSION;
};

INDEXSTOREDB_EXPORT IDCode makeIDCodeFromString(StringRef name);

} // namespace db
} // namespace IndexStoreDB

#endif
