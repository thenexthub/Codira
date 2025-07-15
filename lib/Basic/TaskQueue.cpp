//===--- TaskQueue.cpp - Task Execution Work Queue ------------------------===//
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
///
/// \file
/// This file includes the appropriate platform-specific TaskQueue
/// implementation (or the default serial fallback if one is not available),
/// as well as any platform-agnostic TaskQueue functionality.
///
//===----------------------------------------------------------------------===//

#include "language/Basic/TaskQueue.h"

using namespace language;
using namespace language::sys;

// Include the correct TaskQueue implementation.
#if TOOLCHAIN_ON_UNIX && !defined(__CYGWIN__) && !defined(__HAIKU__)
#include "Unix/TaskQueue.inc"
#elif defined(_WIN32)
#include "Windows/TaskQueue.inc"
#else
#include "Default/TaskQueue.inc"
#endif

namespace language {
namespace sys {
void TaskProcessInformation::ResourceUsage::provideMapping(json::Output &out) {
  out.mapRequired("utime", Utime);
  out.mapRequired("stime", Stime);
  out.mapRequired("maxrss", Maxrss);
}

void TaskProcessInformation::provideMapping(json::Output &out) {
  out.mapRequired("real_pid", OSPid);
  if (ProcessUsage.has_value())
    out.mapRequired("usage", ProcessUsage.value());
}
}
}

TaskQueue::TaskQueue(unsigned NumberOfParallelTasks,
                     UnifiedStatsReporter *USR)
  : NumberOfParallelTasks(NumberOfParallelTasks),
    Stats(USR){}

TaskQueue::~TaskQueue() = default;

// DummyTaskQueue implementation

DummyTaskQueue::DummyTaskQueue(unsigned NumberOfParallelTasks)
  : TaskQueue(NumberOfParallelTasks) {}

DummyTaskQueue::~DummyTaskQueue() = default;

void DummyTaskQueue::addTask(const char *ExecPath, ArrayRef<const char *> Args,
                             ArrayRef<const char *> Env, void *Context,
                             bool SeparateErrors) {
  QueuedTasks.emplace(std::unique_ptr<DummyTask>(
      new DummyTask(ExecPath, Args, Env, Context, SeparateErrors)));
}

bool DummyTaskQueue::execute(TaskQueue::TaskBeganCallback Began,
                             TaskQueue::TaskFinishedCallback Finished,
                             TaskQueue::TaskSignalledCallback Signalled) {
  using PidTaskPair = std::pair<ProcessId, std::unique_ptr<DummyTask>>;
  std::queue<PidTaskPair> ExecutingTasks;

  bool SubtaskFailed = false;

  static ProcessId Pid = 0;

  unsigned MaxNumberOfParallelTasks = getNumberOfParallelTasks();

  while ((!QueuedTasks.empty() && !SubtaskFailed) ||
         !ExecutingTasks.empty()) {
    // Enqueue additional tasks if we have additional tasks, we aren't already
    // at the parallel limit, and no earlier subtasks have failed.
    while (!SubtaskFailed && !QueuedTasks.empty() &&
           ExecutingTasks.size() < MaxNumberOfParallelTasks) {
      std::unique_ptr<DummyTask> T(QueuedTasks.front().release());
      QueuedTasks.pop();

      if (Began)
        Began(++Pid, T->Context);

      ExecutingTasks.push(PidTaskPair(Pid, std::move(T)));
    }

    // Finish the first scheduled task.
    PidTaskPair P = std::move(ExecutingTasks.front());
    ExecutingTasks.pop();

    if (Finished) {
      std::string Output = "Output placeholder\n";
      std::string Errors =
          P.second->SeparateErrors ? "Error placeholder\n" : "";
      if (Finished(P.first, 0, Output, Errors, TaskProcessInformation(Pid),
                   P.second->Context) == TaskFinishedResponse::StopExecution)
        SubtaskFailed = true;
    }
  }

  return false;
}
