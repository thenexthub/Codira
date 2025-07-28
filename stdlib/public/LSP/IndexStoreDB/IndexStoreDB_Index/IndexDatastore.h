//===--- IndexDatastore.h ---------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_LIB_INDEX_INDEXDATASTORE_H
#define INDEXSTOREDB_LIB_INDEX_INDEXDATASTORE_H

#include <IndexStoreDB_Core/Symbol.h>
#include <IndexStoreDB_LLVMSupport/llvm_ADT_OptionSet.h>
#include <memory>
#include <string>
#include <vector>

namespace indexstore {
  class IndexStore;
  typedef std::shared_ptr<IndexStore> IndexStoreRef;
}

namespace IndexStoreDB {
  class CanonicalPathCache;

namespace index {
  class IndexSystemDelegate;
  class SymbolIndex;
  struct CreationOptions;
  typedef std::shared_ptr<SymbolIndex> SymbolIndexRef;

class IndexDatastore {
public:
  ~IndexDatastore();

  static std::unique_ptr<IndexDatastore> create(indexstore::IndexStoreRef idxStore,
                                                SymbolIndexRef SymIndex,
                                                std::shared_ptr<IndexSystemDelegate> Delegate,
                                                std::shared_ptr<CanonicalPathCache> CanonPathCache,
                                                const CreationOptions &Options,
                                                std::string &Error);

  bool isUnitOutOfDate(StringRef unitOutputPath, ArrayRef<StringRef> dirtyFiles);
  bool isUnitOutOfDate(StringRef unitOutputPath, llvm::sys::TimePoint<> outOfDateModTime);
  llvm::Optional<llvm::sys::TimePoint<>> timestampOfUnitForOutputPath(StringRef unitOutputPath);

  /// Check whether any unit(s) containing \p file are out of date and if so,
  /// *synchronously* notify the delegate.
  void checkUnitContainingFileIsOutOfDate(StringRef file);

  void addUnitOutFilePaths(ArrayRef<StringRef> filePaths, bool waitForProcessing);
  void removeUnitOutFilePaths(ArrayRef<StringRef> filePaths, bool waitForProcessing);

  void purgeStaleData();

  /// *For Testing* Poll for any changes to units and wait until they have been registered.
  void pollForUnitChangesAndWait(bool isInitialScan);

  /// Import the units for the given output paths into indexstore-db. Returns after the import has finished.
  void processUnitsForOutputPathsAndWait(ArrayRef<StringRef> outputPaths);

private:
  IndexDatastore(void *Impl) : Impl(Impl) {}

  void *Impl; // An IndexDatastoreImpl.
};

} // namespace index
} // namespace IndexStoreDB

#endif
