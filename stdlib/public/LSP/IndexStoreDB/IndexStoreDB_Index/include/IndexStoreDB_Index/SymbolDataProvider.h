//===--- SymbolDataProvider.h -----------------------------------*- C++ -*-===//
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
#include <IndexStoreDB_LLVMSupport/toolchain_ADT_OptionSet.h>
#include <memory>
#include <vector>

namespace IndexStoreDB {
  class Symbol;
  class SymbolOccurrence;
  struct SymbolInfo;
  enum class SymbolProviderKind : uint8_t;
  enum class SymbolRole : uint64_t;
  typedef std::shared_ptr<Symbol> SymbolRef;
  typedef std::shared_ptr<SymbolOccurrence> SymbolOccurrenceRef;
  typedef toolchain::OptionSet<SymbolRole> SymbolRoleSet;

namespace db {
  class IDCode;
}

namespace index {

class SymbolDataProvider {
public:
  virtual ~SymbolDataProvider() {}

  virtual std::string getIdentifier() const = 0;

  virtual bool isSystem() const = 0;

  virtual bool foreachCoreSymbolData(function_ref<bool(StringRef USR,
                                                       StringRef Name,
                                                       SymbolInfo Info,
                                                       SymbolRoleSet Roles,
                                                       SymbolRoleSet RelatedRoles)> Receiver) = 0;

  virtual bool foreachSymbolOccurrence(function_ref<bool(SymbolOccurrenceRef Occur)> Receiver) = 0;

  virtual bool foreachSymbolOccurrenceByUSR(ArrayRef<db::IDCode> USRs,
                                            SymbolRoleSet RoleSet,
                        function_ref<bool(SymbolOccurrenceRef Occur)> Receiver) = 0;

  virtual bool foreachRelatedSymbolOccurrenceByUSR(ArrayRef<db::IDCode> USRs,
                                            SymbolRoleSet RoleSet,
                        function_ref<bool(SymbolOccurrenceRef Occur)> Receiver) = 0;

  virtual bool foreachUnitTestSymbolOccurrence(
                        function_ref<bool(SymbolOccurrenceRef Occur)> Receiver) = 0;

private:
  virtual void anchor();
};

typedef std::shared_ptr<SymbolDataProvider> SymbolDataProviderRef;

} // namespace index
} // namespace IndexStoreDB

#endif
