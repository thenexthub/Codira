//===--- IndexStoreLibraryProvider.h ----------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_INDEX_SYMBOLDATAPROVIDER_H
#define INDEXSTOREDB_INDEX_SYMBOLDATAPROVIDER_H

#include <IndexStoreDB_Support/LLVM.h>
#include <IndexStoreDB_Support/Visibility.h>
#include <IndexStoreDB_LLVMSupport/llvm_ADT_StringRef.h>
#include <memory>

namespace indexstore {
  class IndexStoreLibrary;
  typedef std::shared_ptr<IndexStoreLibrary> IndexStoreLibraryRef;
}

namespace IndexStoreDB {

namespace index {

using IndexStoreLibrary = ::indexstore::IndexStoreLibrary;
using IndexStoreLibraryRef = ::indexstore::IndexStoreLibraryRef;

class INDEXSTOREDB_EXPORT IndexStoreLibraryProvider {
public:
  virtual ~IndexStoreLibraryProvider() {}

  /// Returns an indexstore compatible with the data format in given store path.
  virtual IndexStoreLibraryRef getLibraryForStorePath(StringRef storePath) = 0;

private:
  virtual void anchor();
};

/// A simple library provider that can be used if libIndexStore is linked to your binary.
class INDEXSTOREDB_EXPORT GlobalIndexStoreLibraryProvider: public IndexStoreLibraryProvider {
public:
  IndexStoreLibraryRef getLibraryForStorePath(StringRef storePath) override;
};

INDEXSTOREDB_EXPORT IndexStoreLibraryRef loadIndexStoreLibrary(std::string dylibPath,
                                                       std::string &error);

} // namespace index
} // namespace IndexStoreDB

#endif
