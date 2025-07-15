//===--- TaskGroup.h - ABI structures for task groups -00--------*- C++ -*-===//
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
//
// Codira ABI describing task groups.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_TASK_GROUP_H
#define LANGUAGE_ABI_TASK_GROUP_H

#include "language/ABI/Task.h"
#include "language/ABI/TaskStatus.h"
#include "language/ABI/HeapObject.h"
#include "language/Runtime/Concurrency.h"
#include "language/Runtime/Config.h"
#include "language/Basic/RelativePointer.h"
#include "language/Basic/STLExtras.h"

namespace language {

/// The task group is responsible for maintaining dynamically created child tasks.
class alignas(Alignment_TaskGroup) TaskGroup {
public:
  // These constructors do not initialize the group instance, and the
  // destructor does not destroy the group instance; you must call
  // language_taskGroup_{initialize,destroy} yourself.
  constexpr TaskGroup()
    : PrivateData{} {}

  void *PrivateData[NumWords_TaskGroup];

  /// Upon a future task's completion, offer it to the task group it belongs to.
  void offer(AsyncTask *completed, AsyncContext *context);

  /// Checks the cancellation status of the group.
  bool isCancelled();

  /// Only mark the task group as cancelled, without performing the follow-up
  /// work of cancelling all the child tasks.
  ///
  /// Returns true if the group was already cancelled before this call.
  bool statusCancel();

  // Add a child task to the task group. Always called while holding the
  // status record lock of the task group's owning task.
  void addChildTask(AsyncTask *task);

  // Remove a child task from the task group. Always called while holding
  // the status record lock of the task group's owning task.
  void removeChildTask(AsyncTask *task);

  // Provide accessor for task group's status record
  TaskGroupTaskStatusRecord *getTaskRecord();

  /// The group is a `TaskGroup` that accumulates results.
  bool isAccumulatingResults() {
    return !isDiscardingResults();
  }

  /// The group is a `DiscardingTaskGroup` that discards results.
  bool isDiscardingResults();
};

} // end namespace language

#endif // LANGUAGE_ABI_TASK_GROUP_H
