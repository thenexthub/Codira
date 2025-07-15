//===--- TaskStatusRecord.h - Structures to track task status --*- C++ -*-===//
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
// Codira ABI describing "status records", the mechanism by which
// tasks track dynamic information about their child tasks, custom
// cancellation hooks, and other information which may need to be exposed
// asynchronously outside of the task.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_TASKSTATUS_BACKDEPLOY56_H
#define LANGUAGE_ABI_TASKSTATUS_BACKDEPLOY56_H

#include "language/ABI/MetadataValues.h"
#include "Task.h"

namespace language {

/// The abstract base class for all status records.
///
/// TaskStatusRecords are typically allocated on the stack (possibly
/// in the task context), partially initialized, and then atomically
/// added to the task with `language_task_addTaskStatusRecord`.  While
/// registered with the task, a status record should only be
/// modified in ways that respect the possibility of asynchronous
/// access by a cancelling thread.  In particular, the chain of
/// status records must not be disturbed.  When the task leaves
/// the scope that requires the status record, the record can
/// be unregistered from the task with `removeStatusRecord`,
/// at which point the memory can be returned to the system.
class TaskStatusRecord {
public:
  TaskStatusRecordFlags Flags;
  TaskStatusRecord *Parent;

  TaskStatusRecord(TaskStatusRecordKind kind,
                   TaskStatusRecord *parent = nullptr)
      : Flags(kind) {
    resetParent(parent);
  }

  TaskStatusRecord(const TaskStatusRecord &) = delete;
  TaskStatusRecord &operator=(const TaskStatusRecord &) = delete;

  TaskStatusRecordKind getKind() const { return Flags.getKind(); }

  TaskStatusRecord *getParent() const { return Parent; }

  /// Change the parent of this unregistered status record to the
  /// given record.
  ///
  /// This should be used when the record has been previously initialized
  /// without knowing what the true parent is.  If we decide to cache
  /// important information (e.g. the earliest timeout) in the innermost
  /// status record, this is the method that should fill that in
  /// from the parent.
  void resetParent(TaskStatusRecord *newParent) {
    Parent = newParent;
    // TODO: cache
  }

  /// Splice a record out of the status-record chain.
  ///
  /// Unlike resetParent, this assumes that it's just removing one or
  /// more records from the chain and that there's no need to do any
  /// extra cache manipulation.
  void spliceParent(TaskStatusRecord *newParent) { Parent = newParent; }
};

/// A status record which states that a task has one or
/// more active child tasks.
class ChildTaskStatusRecord : public TaskStatusRecord {
  AsyncTask *FirstChild;

public:
  ChildTaskStatusRecord(AsyncTask *child)
      : TaskStatusRecord(TaskStatusRecordKind::ChildTask), FirstChild(child) {}

  ChildTaskStatusRecord(AsyncTask *child, TaskStatusRecordKind kind)
      : TaskStatusRecord(kind), FirstChild(child) {
    assert(kind == TaskStatusRecordKind::ChildTask);
    assert(!child->hasGroupChildFragment() &&
           "Group child tasks must be tracked in their respective "
           "TaskGroupTaskStatusRecord, and not as independent "
           "ChildTaskStatusRecord "
           "records.");
  }

  /// Return the first child linked by this record.  This may be null;
  /// if not, it (and all of its successors) are guaranteed to satisfy
  /// `isChildTask()`.
  AsyncTask *getFirstChild() const { return FirstChild; }

  static AsyncTask *getNextChildTask(AsyncTask *task) {
    return task->childFragment()->getNextChild();
  }

  using child_iterator = LinkedListIterator<AsyncTask, getNextChildTask>;
  toolchain::iterator_range<child_iterator> children() const {
    return child_iterator::rangeBeginning(getFirstChild());
  }

  static bool classof(const TaskStatusRecord *record) {
    return record->getKind() == TaskStatusRecordKind::ChildTask;
  }
};

/// A cancellation record which states that a task has an arbitrary
/// function that needs to be called if the task is cancelled.
///
/// The end of any call to the function will be ordered before the
/// end of a call to unregister this record from the task.  That is,
/// code may call `removeStatusRecord` and freely
/// assume after it returns that this function will not be
/// subsequently used.
class CancellationNotificationStatusRecord : public TaskStatusRecord {
public:
  using FunctionType = LANGUAGE_CC(language) void(LANGUAGE_CONTEXT void *);

private:
  FunctionType *__ptrauth_language_cancellation_notification_function Function;
  void *Argument;

public:
  CancellationNotificationStatusRecord(FunctionType *fn, void *arg)
      : TaskStatusRecord(TaskStatusRecordKind::CancellationNotification),
        Function(fn), Argument(arg) {}

  void run() { Function(Argument); }

  static bool classof(const TaskStatusRecord *record) {
    return record->getKind() == TaskStatusRecordKind::CancellationNotification;
  }
};

/// Return the current thread's active task reference.
__attribute__((visibility("hidden")))
LANGUAGE_CC(language)
AsyncTask *language_task_getCurrent(void);
} // namespace language

#endif // LANGUAGE_ABI_TASKSTATUS_BACKDEPLOY56_H
