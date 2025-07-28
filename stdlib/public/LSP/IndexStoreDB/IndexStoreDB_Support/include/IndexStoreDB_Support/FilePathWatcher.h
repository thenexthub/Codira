//===--- FilePathWatcher.h --------------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_SUPPORT_FILEPATHWATCHER_H
#define INDEXSTOREDB_SUPPORT_FILEPATHWATCHER_H

#include <IndexStoreDB_Support/LLVM.h>
#include <IndexStoreDB_Support/Visibility.h>
#include <IndexStoreDB_LLVMSupport/llvm_ADT_ArrayRef.h>
#include <IndexStoreDB_LLVMSupport/llvm_ADT_StringRef.h>
#include <functional>

namespace IndexStoreDB {

class INDEXSTOREDB_EXPORT FilePathWatcher {
  struct Implementation;

public:
  typedef std::function<void(std::vector<std::string>)> FileEventsReceiverTy;
  explicit FilePathWatcher(FileEventsReceiverTy pathsReceiver);
  ~FilePathWatcher();

private:
  Implementation &Impl;
};

} // namespace IndexStoreDB

#endif
