//===--- ImportTransactionImpl.h --------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_SKDATABASE_LIB_IMPORTTRANSACTIONIMPL_H
#define INDEXSTOREDB_SKDATABASE_LIB_IMPORTTRANSACTIONIMPL_H

#include <IndexStoreDB_Database/ImportTransaction.h>
#include <IndexStoreDB_Database/UnitInfo.h>
#include "lmdb/lmdb++.h"

namespace IndexStoreDB {
namespace db {

class ImportTransaction::Implementation {
public:
  DatabaseRef DBase;
  lmdb::txn Txn{nullptr};

  explicit Implementation(DatabaseRef dbase);

  IDCode getUnitCode(StringRef unitName);
  IDCode addProviderName(StringRef name, bool *wasInserted);
  // Marks a provider as containing test symbols.
  void setProviderContainsTestSymbols(IDCode provider);
  bool providerContainsTestSymbols(IDCode provider);
  /// \returns a IDCode of the USR.
  IDCode addSymbolInfo(IDCode provider, StringRef USR, StringRef symbolName, SymbolInfo symInfo,
                       SymbolRoleSet roles, SymbolRoleSet relatedRoles);
  IDCode addFilePath(CanonicalFilePathRef canonFilePath);
  IDCode addDirectory(CanonicalFilePathRef directory);
  IDCode addUnitFileIdentifier(StringRef unitFile);
  IDCode addTargetName(StringRef target);
  IDCode addModuleName(StringRef moduleName);
  /// If file is already associated, its timestamp is updated if \c modTime is more recent.
  void addFileAssociationForProvider(IDCode provider, IDCode file, IDCode unit, toolchain::sys::TimePoint<> modTime, IDCode module, bool isSystem);
  /// \returns true if there is no remaining file reference, false otherwise.
  bool removeFileAssociationFromProvider(IDCode provider, IDCode file, IDCode unit);

  /// UnitInfo.UnitName will be empty if \c unit was not found. UnitInfo.UnitCode is always filled out.
  UnitInfo getUnitInfo(IDCode unitCode);

  void addUnitInfo(const UnitInfo &unitInfo);
  /// \returns the IDCode for the file path.
  IDCode addUnitFileDependency(IDCode unitCode, CanonicalFilePathRef filePathDep);
  /// \returns the IDCode for the unit name.
  IDCode addUnitUnitDependency(IDCode unitCode, StringRef unitNameDep);

  void removeUnitFileDependency(IDCode unitCode, IDCode pathCode);
  void removeUnitUnitDependency(IDCode unitCode, IDCode unitDepCode);
  void removeUnitData(IDCode unitCode);
  void removeUnitData(StringRef unitName);

  void commit();

private:
  IDCode addFilePath(StringRef filePath);
};

} // namespace db
} // namespace IndexStoreDB

#endif
