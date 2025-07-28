//===--- StoreUnitInfo.h ----------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_INDEX_STOREUNITINFO_H
#define INDEXSTOREDB_INDEX_STOREUNITINFO_H

#include <IndexStoreDB_Support/Path.h>
#include <IndexStoreDB_LLVMSupport/llvm_Support_Chrono.h>
#include <string>

namespace IndexStoreDB {
namespace index {

struct StoreUnitInfo {
  std::string UnitName;
  CanonicalFilePath MainFilePath;
  std::string OutFileIdentifier;
  bool HasTestSymbols = false;
  llvm::sys::TimePoint<> ModTime;

  StoreUnitInfo() = default;
  StoreUnitInfo(std::string unitName, CanonicalFilePath mainFilePath,
                StringRef outFileIdentifier, bool hasTestSymbols,
                llvm::sys::TimePoint<> modTime)
      : UnitName(unitName),
        MainFilePath(mainFilePath),
        OutFileIdentifier(outFileIdentifier),
        HasTestSymbols(hasTestSymbols),
        ModTime(modTime) {}
};

} // namespace index
} // namespace IndexStoreDB

#endif
