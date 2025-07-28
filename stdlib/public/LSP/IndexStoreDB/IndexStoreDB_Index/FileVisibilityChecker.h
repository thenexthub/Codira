//===--- FileVisibilityChecker.h --------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_LIB_INDEX_FILEVISIBILITYCHECKER_H
#define INDEXSTOREDB_LIB_INDEX_FILEVISIBILITYCHECKER_H

#include <IndexStoreDB_Database/IDCode.h>
#include <IndexStoreDB_Support/LLVM.h>
#include <IndexStoreDB_LLVMSupport/llvm_Support_Mutex.h>
#include <unordered_map>
#include <unordered_set>

namespace IndexStoreDB {
  class CanonicalPathCache;

namespace db {
  class IDCode;
  class ReadTransaction;
  class Database;
  typedef std::shared_ptr<Database> DatabaseRef;
  struct UnitInfo;
}

namespace index {

class FileVisibilityChecker {
  db::DatabaseRef DBase;
  std::shared_ptr<CanonicalPathCache> CanonPathCache;

  mutable llvm::sys::Mutex VisibleCacheMtx;
  std::unordered_set<db::IDCode> VisibleMainFiles;
  std::unordered_map<db::IDCode, unsigned> MainFilesRefCount;
  std::unordered_map<db::IDCode, bool> UnitVisibilityCache;

  std::unordered_set<db::IDCode> OutUnitFiles;
  bool UseExplicitOutputUnits;

public:
  FileVisibilityChecker(db::DatabaseRef dbase,
                        std::shared_ptr<CanonicalPathCache> canonPathCache,
                        bool useExplicitOutputUnits);

  void registerMainFiles(ArrayRef<StringRef> filePaths, StringRef productName);
  void unregisterMainFiles(ArrayRef<StringRef> filePaths, StringRef productName);

  void addUnitOutFilePaths(ArrayRef<StringRef> filePaths);
  void removeUnitOutFilePaths(ArrayRef<StringRef> filePaths);

  bool isUnitVisible(const db::UnitInfo &unitInfo, db::ReadTransaction &reader);
};

} // namespace index
} // namespace IndexStoreDB

#endif
