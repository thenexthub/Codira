//===--- IndexSystemDelegate.h ----------------------------------*- C++ -*-===//
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

#ifndef INDEXSTOREDB_INDEX_INDEXSYSTEMDELEGATE_H
#define INDEXSTOREDB_INDEX_INDEXSYSTEMDELEGATE_H

#include <IndexStoreDB_Index/StoreUnitInfo.h>
#include <IndexStoreDB_LLVMSupport/llvm_Support_Chrono.h>
#include <memory>
#include <string>

namespace IndexStoreDB {
namespace index {
struct StoreUnitInfo;
class OutOfDateFileTrigger;

typedef std::shared_ptr<OutOfDateFileTrigger> OutOfDateFileTriggerRef;

/// Records a known out-of-date file path for a unit, along with its
/// modification time. This is used to provide IndexDelegate with information
/// about the file that triggered the unit to become out-of-date.
class OutOfDateFileTrigger final {
  std::string FilePath;
  llvm::sys::TimePoint<> ModTime;

public:
  explicit OutOfDateFileTrigger(StringRef filePath,
                                llvm::sys::TimePoint<> modTime)
      : FilePath(filePath), ModTime(modTime) {}

  static OutOfDateFileTriggerRef create(StringRef filePath,
                                        llvm::sys::TimePoint<> modTime) {
    return std::make_shared<OutOfDateFileTrigger>(filePath, modTime);
  }

  llvm::sys::TimePoint<> getModTime() const { return ModTime; }

  /// Returns a reference to the stored file path. Note this has the same
  /// lifetime as the trigger.
  StringRef getPathRef() const { return FilePath; }

  std::string getPath() const { return FilePath; }
  std::string description() { return FilePath; }
};

class INDEXSTOREDB_EXPORT IndexSystemDelegate {
public:
  virtual ~IndexSystemDelegate() {}

  /// Called when the datastore gets initialized and receives the number of available units.
  virtual void initialPendingUnits(unsigned numUnits) {}

  virtual void processingAddedPending(unsigned NumActions) {}
  virtual void processingCompleted(unsigned NumActions) {}

  virtual void processedStoreUnit(StoreUnitInfo unitInfo) {}

  virtual void unitIsOutOfDate(StoreUnitInfo unitInfo,
                               OutOfDateFileTriggerRef trigger,
                               bool synchronous = false) {}

private:
  virtual void anchor();
};

} // namespace index
} // namespace IndexStoreDB

#endif
