//===--- FilePathIndex.h ----------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_INDEX_FILEPATHINDEX_H
#define INDEXSTOREDB_INDEX_FILEPATHINDEX_H

#include <IndexStoreDB_Support/LLVM.h>
#include <IndexStoreDB_LLVMSupport/llvm_ADT_StringRef.h>
#include <memory>

namespace indexstore {
  class IndexStore;
  typedef std::shared_ptr<IndexStore> IndexStoreRef;
}

namespace IndexStoreDB {
  class CanonicalFilePath;
  class CanonicalFilePathRef;
  class CanonicalPathCache;

namespace db {
  class Database;
  typedef std::shared_ptr<Database> DatabaseRef;
}

namespace index {
  class FileVisibilityChecker;
  struct StoreUnitInfo;

class FilePathIndex {
public:
  FilePathIndex(db::DatabaseRef dbase, indexstore::IndexStoreRef idxStore,
                std::shared_ptr<FileVisibilityChecker> visibilityChecker,
                std::shared_ptr<CanonicalPathCache> canonPathCache);
  ~FilePathIndex();

  CanonicalFilePath getCanonicalPath(StringRef Path,
                                     StringRef WorkingDir = StringRef());

  //===--------------------------------------------------------------------===//
  // Queries
  //===--------------------------------------------------------------------===//

  bool foreachMainUnitContainingFile(CanonicalFilePathRef filePath,
                                 function_ref<bool(const StoreUnitInfo &unitInfo)> Receiver);

  bool isKnownFile(CanonicalFilePathRef filePath);

  bool foreachFileOfUnit(StringRef unitName,
                         bool followDependencies,
                         function_ref<bool(CanonicalFilePathRef filePath)> receiver);

  bool foreachFilenameContainingPattern(StringRef Pattern,
                                        bool AnchorStart,
                                        bool AnchorEnd,
                                        bool Subsequence,
                                        bool IgnoreCase,
                               function_ref<bool(CanonicalFilePathRef FilePath)> Receiver);

  bool foreachFileIncludingFile(CanonicalFilePathRef targetPath,
                            function_ref<bool(CanonicalFilePathRef SourcePath, unsigned Line)> Receiver);

  bool foreachFileIncludedByFile(CanonicalFilePathRef sourcePath,
                            function_ref<bool(CanonicalFilePathRef TargetPath, unsigned Line)> Receiver);

  bool foreachIncludeOfUnit(StringRef unitName,
                            function_ref<bool(CanonicalFilePathRef sourcePath, CanonicalFilePathRef targetPath, unsigned line)> receiver);

private:
  void *Impl; // A FileIndexImpl.
};

typedef std::shared_ptr<FilePathIndex> FilePathIndexRef;

} // namespace index
} // namespace IndexStoreDB

#endif
