//===--- UnitInfo.h ---------------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_SKDATABASE_UNITINFO_H
#define INDEXSTOREDB_SKDATABASE_UNITINFO_H

#include <IndexStoreDB_Database/IDCode.h>
#include <IndexStoreDB_Support/LLVM.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_ArrayRef.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_Hashing.h>
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringRef.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Chrono.h>

namespace IndexStoreDB {
namespace db {

struct UnitInfo {
  struct Provider {
    IDCode ProviderCode;
    IDCode FileCode;

    friend bool operator ==(const Provider &lhs, const Provider &rhs) {
      return lhs.ProviderCode == rhs.ProviderCode && lhs.FileCode == rhs.FileCode;
    }
    friend bool operator !=(const Provider &lhs, const Provider &rhs) {
      return !(lhs == rhs);
    }
  };

  StringRef UnitName;
  IDCode UnitCode;
  toolchain::sys::TimePoint<> ModTime;
  IDCode OutFileCode;
  IDCode MainFileCode;
  IDCode SysrootCode;
  IDCode TargetCode;
  bool HasMainFile;
  bool HasSysroot;
  bool IsSystem;
  bool HasTestSymbols;
  SymbolProviderKind SymProviderKind;
  ArrayRef<IDCode> FileDepends;
  ArrayRef<IDCode> UnitDepends;
  ArrayRef<Provider> ProviderDepends;

  bool isInvalid() const { return UnitName.empty(); }
  bool isValid() const { return !isInvalid(); }
};

} // namespace db
} // namespace IndexStoreDB

namespace std {
template <> struct hash<IndexStoreDB::db::UnitInfo::Provider> {
  size_t operator()(const IndexStoreDB::db::UnitInfo::Provider &k) const {
    return toolchain::hash_combine(k.FileCode.value(), k.ProviderCode.value());
  }
};
}

#endif
