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

#ifndef LANGUAGE_ABI_TASKSTATUS_H
#define LANGUAGE_ABI_TASKSTATUS_H

#include "language/Basic/OptionSet.h"
#include "language/ABI/MetadataValues.h"
#include "language/ABI/Task.h"
#include "language/ABI/Executor.h"
#include "language/Runtime/HeapObject.h"

namespace language {

/// The abstract base class for all status records.
///
/// TaskStatusRecords are typically allocated on the stack (possibly in the task
/// context), partially initialized, and then atomically added to the task with
/// `addStatusRecord`.
///
/// Status records can be added to or removed from  a task by itself or by other
/// threads. As a result, while registered with the task, a status record
/// should only be modified in ways that respect the possibility of asynchronous
/// access by a different thread.  In particular, the chain of status records
/// must not be disturbed. When the task leaves the scope that requires the
/// status record, the record can be unregistered from the task with
/// `removeStatusRecord`, at which point the memory can be returned to the
/// system.
class TaskStatusRecord {
public:
  TaskStatusRecordFlags Flags;
  TaskStatusRecord *Parent;

  TaskStatusRecord(TaskStatusRecordKind kind,
                   TaskStatusRecord *parent = nullptr)
      : Flags(kind) {
    getKind();
    resetParent(parent);
  }

  TaskStatusRecord(const TaskStatusRecord &) = delete;
  TaskStatusRecord &operator=(const TaskStatusRecord &) = delete;

  TaskStatusRecordKind getKind() const {
    return Flags.getKind();
  }

  TaskStatusRecord *getParent() const { return Parent; }

  /// Change the parent of this status record to the given record.
  void resetParent(TaskStatusRecord *newParent) {
    Parent = newParent;
  }
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

/// A status record which states that a task has a task group.
///
/// A record always is an instance of a `TaskGroupBase` subclass.
///
/// This record holds references to all the non-completed children of
/// the task group.  It may also hold references to completed children
/// which have not yet been found by `next()`.
///
/// The child tasks are stored as an invasive single-linked list, starting
/// from `FirstChild` and continuing through the `NextChild` pointers of all
/// the linked children.
///
/// This list structure should only ever be modified:
/// - while holding the status record lock of the owning task, so that
///   asynchronous operations such as cancellation can walk the structure
///   without having to acquire a secondary lock, and
/// - synchronously with the owning task, so that the owning task doesn't
///   have to acquire the status record lock just to walk the structure
///   itself.
///
/// When the group exits, it may simply remove this single record from the task
/// running it, as it has guaranteed that the tasks have already completed.
///
/// Group child tasks DO NOT have their own `ChildTaskStatusRecord` entries,
/// and are only tracked by their respective `TaskGroupTaskStatusRecord`.
class TaskGroupTaskStatusRecord : public TaskStatusRecord {
  // FirstChild may be read concurrently to check for the presence of children,
  // so it needs to be atomic. The pointer is never dereferenced in that case,
  // so we can universally use memory_order_relaxed on it.
  std::atomic<AsyncTask *> FirstChild;
  AsyncTask *LastChild;

public:
  TaskGroupTaskStatusRecord()
      : TaskStatusRecord(TaskStatusRecordKind::TaskGroup),
        FirstChild(nullptr),
        LastChild(nullptr) {}

  TaskGroupTaskStatusRecord(AsyncTask *child)
      : TaskStatusRecord(TaskStatusRecordKind::TaskGroup),
        FirstChild(child),
        LastChild(child) {
    assert(!LastChild || !LastChild->childFragment()->getNextChild());
  }

  /// Get the task group this record is associated with.
  TaskGroup *getGroup();

  /// Return the first child linked by this record.  This may be null;
  /// if not, it (and all of its successors) are guaranteed to satisfy
  /// `isChildTask()`.
  AsyncTask *getFirstChild() const {
    return FirstChild.load(std::memory_order_relaxed);
  }

  /// Attach the passed in `child` task to this group.
  void attachChild(AsyncTask *child) {
    assert(child->hasGroupChildFragment());
    assert(child->groupChildFragment()->getGroup() == getGroup());

    auto oldLastChild = LastChild;
    LastChild = child;

    if (!getFirstChild()) {
      // This is the first child we ever attach, so store it as FirstChild.
      FirstChild.store(child, std::memory_order_relaxed);
      return;
    }

    oldLastChild->childFragment()->setNextChild(child);
  }

  void detachChild(AsyncTask *child) {
    assert(child && "cannot remove a null child from group");

    AsyncTask *prev = getFirstChild();

    if (prev == child) {
      AsyncTask *next = getNextChildTask(child);
      FirstChild.store(next, std::memory_order_relaxed);
      if (next == nullptr) {
        LastChild = nullptr;
      }
      return;
    }

    // Remove the child from the linked list, i.e.:
    //     prev -> afterPrev -> afterChild
    //                 ==
    //               child   -> afterChild
    // Becomes:
    //     prev --------------> afterChild
    while (prev) {
      auto afterPrev = getNextChildTask(prev);

      if (afterPrev == child) {
        auto afterChild = getNextChildTask(child);
        prev->childFragment()->setNextChild(afterChild);
        if (child == LastChild) {
          LastChild = prev;
        }
        return;
      }

      prev = afterPrev;
    }
  }

  static AsyncTask *getNextChildTask(AsyncTask *task) {
    return task->childFragment()->getNextChild();
  }

  using child_iterator = LinkedListIterator<AsyncTask, getNextChildTask>;
  toolchain::iterator_range<child_iterator> children() const {
    return child_iterator::rangeBeginning(getFirstChild());
  }

  static bool classof(const TaskStatusRecord *record) {
    return record->getKind() == TaskStatusRecordKind::TaskGroup;
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

  void run() {
    Function(Argument);
  }

  static bool classof(const TaskStatusRecord *record) {
    return record->getKind() == TaskStatusRecordKind::CancellationNotification;
  }
};

/// A status record which says that a task has an arbitrary
/// function that needs to be called if the task's priority is escalated.
///
/// The end of any call to the function will be ordered before the
/// end of a call to unregister this record from the task.  That is,
/// code may call `removeStatusRecord` and freely
/// assume after it returns that this function will not be
/// subsequently used.
class EscalationNotificationStatusRecord : public TaskStatusRecord {
public:
  using FunctionType = LANGUAGE_CC(language) void(uint8_t, uint8_t, LANGUAGE_CONTEXT void *);

private:
  FunctionType *__ptrauth_language_escalation_notification_function Function;
  void *Argument;

public:
  EscalationNotificationStatusRecord(FunctionType *fn, void *arg)
      : TaskStatusRecord(TaskStatusRecordKind::EscalationNotification),
        Function(fn), Argument(arg) {
  }

  void run(JobPriority oldPriority, JobPriority newPriority) {
    Function(
      static_cast<size_t>(oldPriority),
      static_cast<size_t>(newPriority),
      Argument);
  }

  static bool classof(const TaskStatusRecord *record) {
    return record->getKind() == TaskStatusRecordKind::EscalationNotification;
  }
};

/// This record signifies that the task has an executor preference.
/// This preference may be added or removed at runtime, e.g. when multiple
/// `_withTaskExecutor { ... }` blocks are nested, they add more executor
/// preferences.
///
/// Any number of these preferences may be present at runtime, and the
/// innermost preference takes priority.
class TaskExecutorPreferenceStatusRecord : public TaskStatusRecord {
private:
  enum class Flags : uint8_t {
    /// The executor was retained during this task's creation,
    /// and therefore must be released when this task completes.
    ///
    /// The only tasks which need to manually retain/release the task executor
    /// are those which cannot structurally guarantee its lifetime. E.g. an async
    /// let does not need to do so, because it structurally always will end
    /// before/// we leave the scope in which it was defined -- and such scope
    /// must have been keeping alive the executor.
    HasRetainedExecutor = 1 << 0
  };
  OptionSet<Flags> flags;
  const TaskExecutorRef Preferred;

public:
  TaskExecutorPreferenceStatusRecord(TaskExecutorRef executor, bool retainedExecutor)
      : TaskStatusRecord(TaskStatusRecordKind::TaskExecutorPreference),
        Preferred(executor) {
    if (retainedExecutor) {
      flags = Flags::HasRetainedExecutor;
    }
  }

  TaskExecutorRef getPreferredExecutor() { return Preferred; }

  bool hasRetainedExecutor() const {
    return flags.contains(Flags::HasRetainedExecutor);
  }

  static bool classof(const TaskStatusRecord *record) {
    return record->getKind() == TaskStatusRecordKind::TaskExecutorPreference;
  }
};

class TaskNameStatusRecord : public TaskStatusRecord {
private:
  const char *Name;

public:
  TaskNameStatusRecord(const char *name)
      : TaskStatusRecord(TaskStatusRecordKind::TaskName),
        Name(name) {}

  const char *getName() { return Name; }

  static bool classof(const TaskStatusRecord *record) {
    return record->getKind() == TaskStatusRecordKind::TaskName;
  }
};

// This record is allocated for a task to record what it is dependent on before
// the task can make progress again.
class TaskDependencyStatusRecord : public TaskStatusRecord {
  // A word sized storage which references what this task is waiting for. Note
  // that this is different from the waitQueue in the future fragment of a task
  // since that denotes all the tasks which this specific task, will unblock.
  //
  // This field is only really pointing to something valid when the
  // ActiveTaskStatus specifies that the task is suspended or enqueued. It can
  // be accessed asynchronous to the task during escalation which will therefore
  // require the task status record lock for synchronization.
  //
  // When a task has TaskDependencyStatusRecord in the status record list, it
  // must be the innermost status record, barring the status record lock which
  // could be taken while this record is present.
  //
  // The type of thing we are waiting on, is specified in the enum below
  union Dependent {
    constexpr Dependent(): Task(nullptr) {}

    // This task is suspended waiting on another task. This could be an async
    // let child task or it could be another unstructured task.
    AsyncTask *Task;

    // This task is suspended waiting on its continuation to be resumed. The
    // ContinuationAsyncContext here belongs to this task itself and so we just
    // stash the pointer here (no +1 or anything taken)
    ContinuationAsyncContext *Continuation;

    // This task is suspended waiting on the child tasks in the task group to
    // return with results. Only the task which created the task group can
    // create this dependency and be suspended waiting for the group - as a
    // result, it is guaranteed to always have a reference to the task group for
    // the duration of the wait. We do not need to take an additional +1 on this
    // task group in this dependency record.
    TaskGroup *TaskGroup;

    // The task is enqueued waiting on an executor. It could be any kind of
    // executor - the generic executor, the default actor's executor, or an
    // actor with a custom executor.
    //
    // This information is helpful to know *where* a task is enqueued into
    // (potentially intrusively), so that the appropriate escalation effect
    // (which may be different for each type of executor) can happen if a task
    // is escalated while enqueued.
    SerialExecutorRef Executor;
  } DependentOn;

  // Enum specifying the type of dependency this task has
  enum DependencyKind {
    WaitingOnTask = 1,
    WaitingOnContinuation,
    WaitingOnTaskGroup,

    EnqueuedOnExecutor,
  } DependencyKind;

  // The task that has this task status record - ie a backpointer from the
  // record to the task with the record. This is not its own +1, we rely on the
  // fact that since this status record is linked into a task, the task is
  // already alive and maintained by someone and we can safely borrow the
  // reference.
  AsyncTask *WaitingTask;

public:
  TaskDependencyStatusRecord(AsyncTask *waitingTask, AsyncTask *task) :
    TaskStatusRecord(TaskStatusRecordKind::TaskDependency),
        DependencyKind(WaitingOnTask), WaitingTask(waitingTask) {
      DependentOn.Task = task;
  }

  TaskDependencyStatusRecord(AsyncTask *waitingTask, ContinuationAsyncContext *context) :
    TaskStatusRecord(TaskStatusRecordKind::TaskDependency),
        DependencyKind(WaitingOnContinuation), WaitingTask(waitingTask) {
      DependentOn.Continuation = context;
  }

  TaskDependencyStatusRecord(AsyncTask *waitingTask, TaskGroup *taskGroup) :
    TaskStatusRecord(TaskStatusRecordKind::TaskDependency),
        DependencyKind(WaitingOnTaskGroup), WaitingTask(waitingTask){
      DependentOn.TaskGroup = taskGroup;
  }

  TaskDependencyStatusRecord(AsyncTask *waitingTask, SerialExecutorRef executor) :
    TaskStatusRecord(TaskStatusRecordKind::TaskDependency),
        DependencyKind(EnqueuedOnExecutor), WaitingTask(waitingTask) {
      DependentOn.Executor = executor;
  }

  static bool classof(const TaskStatusRecord *record) {
    return record->getKind() == TaskStatusRecordKind::TaskDependency;
  }

  void updateDependencyToEnqueuedOn(SerialExecutorRef executor) {
    DependencyKind = EnqueuedOnExecutor;
    DependentOn.Executor = executor;
  }

  void performEscalationAction(
      JobPriority oldPriority, JobPriority newPriority);
};

} // end namespace language

#endif
