//===--- ReadTransaction.h --------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_SKDATABASE_READTRANSACTION_H
#define INDEXSTOREDB_SKDATABASE_READTRANSACTION_H

#include <IndexStoreDB_Core/Symbol.h>
#include <IndexStoreDB_Database/UnitInfo.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_STLExtras.h>
#include <memory>

namespace IndexStoreDB {
  class CanonicalFilePath;
  class CanonicalFilePathRef;

namespace db {
  class Database;
  typedef std::shared_ptr<Database> DatabaseRef;

class INDEXSTOREDB_EXPORT ReadTransaction {
public:
  explicit ReadTransaction(DatabaseRef dbase);
  ~ReadTransaction();

  /// Returns providers containing the USR with any of the roles.
  /// If both \c roles and \c relatedRoles are given then both any roles and any related roles should be satisfied.
  /// If both \c roles and \c relatedRoles are empty then all providers are returned.
  bool lookupProvidersForUSR(StringRef USR, SymbolRoleSet roles, SymbolRoleSet relatedRoles,
                             toolchain::function_ref<bool(IDCode provider, SymbolRoleSet roles, SymbolRoleSet relatedRoles)> receiver);
  bool lookupProvidersForUSR(IDCode usrCode, SymbolRoleSet roles, SymbolRoleSet relatedRoles,
                             toolchain::function_ref<bool(IDCode provider, SymbolRoleSet roles, SymbolRoleSet relatedRoles)> receiver);

  StringRef getProviderName(IDCode provider);
  StringRef getTargetName(IDCode target);
  StringRef getModuleName(IDCode moduleName);
  bool getProviderFileReferences(IDCode provider,
                                 toolchain::function_ref<bool(TimestampedPath path)> receiver);
  /// `unitFilter` returns `true` if the unit should be included, `false` if it should be ignored.
  bool getProviderFileCodeReferences(IDCode provider,
    function_ref<bool(IDCode unitCode)> unitFilter,
    function_ref<bool(IDCode pathCode, IDCode unitCode, toolchain::sys::TimePoint<> modTime, IDCode moduleNameCode, bool isSystem)> receiver);
  /// Returns all provider-file associations. Intended for debugging purposes.
  /// `unitFilter` returns `true` if the unit should be included, `false` if it should be ignored.
  bool foreachProviderAndFileCodeReference(function_ref<bool(IDCode unitCode)> unitFilter,
    function_ref<bool(IDCode provider, IDCode pathCode, IDCode unitCode, toolchain::sys::TimePoint<> modTime, IDCode moduleNameCode, bool isSystem)> receiver);

  bool foreachProviderContainingTestSymbols(function_ref<bool(IDCode provider)> receiver);

  /// Returns USR codes in batches.
  bool foreachUSROfGlobalSymbolKind(SymbolKind symKind, toolchain::function_ref<bool(ArrayRef<IDCode> usrCodes)> receiver);

  /// Returns USR codes in batches.
  bool foreachUSROfGlobalUnitTestSymbol(toolchain::function_ref<bool(ArrayRef<IDCode> usrCodes)> receiver);

  /// Returns USR codes in batches.
  bool findUSRsWithNameContaining(StringRef pattern,
                                  bool anchorStart, bool anchorEnd,
                                  bool subsequence, bool ignoreCase,
                                  toolchain::function_ref<bool(ArrayRef<IDCode> usrCodes)> receiver);
  bool foreachUSRBySymbolName(StringRef name, toolchain::function_ref<bool(ArrayRef<IDCode> usrCodes)> receiver);

  /// The memory that \c filePath points to may not live beyond the receiver function invocation.
  bool findFilenamesContaining(StringRef pattern,
                               bool anchorStart, bool anchorEnd,
                               bool subsequence, bool ignoreCase,
                               toolchain::function_ref<bool(CanonicalFilePathRef filePath)> receiver);

  /// Returns all the recorded symbol names along with their associated USRs.
  bool foreachSymbolName(function_ref<bool(StringRef name)> receiver);

  /// Returns true if it was found, false otherwise.
  bool getFullFilePathFromCode(IDCode filePathCode, raw_ostream &OS);
  CanonicalFilePath getFullFilePathFromCode(IDCode filePathCode);
  CanonicalFilePathRef getDirectoryFromCode(IDCode dirCode);
  /// Returns empty path if it was not found. This should only be used for the unit path since it is not treated as
  /// a canonicalized path.
  std::string getUnitFileIdentifierFromCode(IDCode fileCode);

  bool foreachDirPath(toolchain::function_ref<bool(CanonicalFilePathRef dirPath)> receiver);

  /// The memory that \c filePath points to may not live beyond the receiver function invocation.
  bool findFilePathsWithParentPaths(ArrayRef<CanonicalFilePathRef> parentPaths,
                                    toolchain::function_ref<bool(IDCode pathCode, CanonicalFilePathRef filePath)> receiver);

  IDCode getFilePathCode(CanonicalFilePathRef filePath);
  IDCode getUnitFileIdentifierCode(StringRef filePath);

  /// UnitInfo.UnitName will be empty if \c unit was not found. UnitInfo.UnitCode is always filled out.
  UnitInfo getUnitInfo(IDCode unitCode);
  /// UnitInfo.UnitName will be empty if \c unit was not found. UnitInfo.UnitCode is always filled out.
  UnitInfo getUnitInfo(StringRef unitName);

  bool foreachUnitContainingFile(IDCode filePathCode,
                                 toolchain::function_ref<bool(ArrayRef<IDCode> unitCodes)> receiver);
  bool foreachUnitContainingUnit(IDCode unitCode,
                                 toolchain::function_ref<bool(ArrayRef<IDCode> unitCodes)> receiver);

  bool foreachRootUnitOfFile(IDCode filePathCode,
                             function_ref<bool(const UnitInfo &unitInfo)> receiver);
  bool foreachRootUnitOfUnit(IDCode unitCode,
                             function_ref<bool(const UnitInfo &unitInfo)> receiver);

  void getDirectDependentUnits(IDCode unitCode, SmallVectorImpl<IDCode> &units);

  class Implementation;
private:
  std::unique_ptr<Implementation> Impl;
};

} // namespace db
} // namespace IndexStoreDB

#endif
