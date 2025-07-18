//===--- TaskQueue.inc ------------------------------------------*- C++ -*-===//
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
/// This file contains an implementation of TaskQueue for Windows platform.
///
//===----------------------------------------------------------------------===//

#include "language/Basic/Toolchain.h"
#include "language/Basic/TaskQueue.h"

#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Signals.h"

#define NOMINMAX
#include <Windows.h>
#include <psapi.h>

#include "Default/Task.inc"

using namespace toolchain::sys;

namespace {

class TaskMonitor {
  std::queue<std::unique_ptr<Task>> &TasksToBeExecuted;
  toolchain::SmallVector<std::unique_ptr<Task>, 32> TasksBeingExecuted;
  const unsigned MaxNumberOfParallelTasks;

public:
  struct Callbacks {
    const TaskQueue::TaskBeganCallback TaskBegan;
    const TaskQueue::TaskFinishedCallback TaskFinished;
    const TaskQueue::TaskSignalledCallback TaskSignalled;
  };

private:
  Callbacks Cbs;

  bool startUpSomeTasks() {
    while (!TasksToBeExecuted.empty() &&
           TasksBeingExecuted.size() < MaxNumberOfParallelTasks) {
      std::unique_ptr<Task> T(std::move(TasksToBeExecuted.front()));
      TasksToBeExecuted.pop();
      if (T->execute())
        return true;
      if (Cbs.TaskBegan)
        Cbs.TaskBegan(T->PI.Pid, T->Context);
      TasksBeingExecuted.push_back(std::move(T));
    }
    return false;
  }

  bool waitForFinishedTask() {
    if (TasksBeingExecuted.empty())
      return false;
    toolchain::SmallVector<HANDLE, 32> Handles;
    for (auto &TBE : TasksBeingExecuted)
      Handles.push_back((HANDLE)TBE->PI.Process);

    DWORD Finished =
        WaitForMultipleObjects(Handles.size(), Handles.data(), FALSE, INFINITE);
    if (Finished < WAIT_OBJECT_0 || Finished >= WAIT_OBJECT_0 + Handles.size())
      return true;
    size_t Index = Finished - WAIT_OBJECT_0;
    if (Index + 1 != TasksBeingExecuted.size())
      std::swap(TasksBeingExecuted[Index], TasksBeingExecuted.back());

    std::unique_ptr<Task> T(std::move(TasksBeingExecuted.back()));
    TasksBeingExecuted.pop_back();

    DWORD Status = 0;
    BOOL RC = GetExitCodeProcess(T->PI.Process, &Status);
    DWORD Err = GetLastError();
    if (Err != ERROR_INVALID_HANDLE)
      CloseHandle(T->PI.Process);
    if (!RC) {
      SetLastError(Err);
      // -2 indicates a crash or timeout as opposed to failure to execute.
      // See the comments on toolchain::sys::Wait for more details.
      T->PI.ReturnCode = -2;
    }
    // Convert the return code in the same way as toolchain::sys::Wait does.
    if (Status != 0) {
      // Pass 10(Warning) and 11(Error) to the callee as negative value.
      // For more information on status codes, see
      // https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/87fba13e-bf06-450e-83b1-9241dc81e781
      if ((Status & 0xBFFF0000U) == 0x80000000U)
        T->PI.ReturnCode = static_cast<int>(Status);
      else if (Status & 0xFF)
        T->PI.ReturnCode = Status & 0x7FFFFFFF;
      else
        T->PI.ReturnCode = 1;
    }

    auto StdoutBuffer = toolchain::MemoryBuffer::getFile(T->StdoutPath);
    StringRef StdoutContents = StdoutBuffer.get()->getBuffer();

    StringRef StderrContents;
    if (T->SeparateErrors) {
      auto StderrBuffer = toolchain::MemoryBuffer::getFile(T->StderrPath);
      StderrContents = StderrBuffer.get()->getBuffer();
    }

    bool Crashed = T->PI.ReturnCode & 0xC0000000;
    FILETIME CreationTime;
    FILETIME ExitTime;
    FILETIME UtimeTicks;
    FILETIME StimeTicks;
    PROCESS_MEMORY_COUNTERS Counters = {};
    GetProcessTimes(T->PI.Process, &CreationTime, &ExitTime, &StimeTicks,
                    &UtimeTicks);
    // Each tick is 100ns
    uint64_t UTime =
        ((uint64_t)UtimeTicks.dwHighDateTime << 32 | UtimeTicks.dwLowDateTime) /
        10;
    uint64_t STime =
        ((uint64_t)StimeTicks.dwHighDateTime << 32 | StimeTicks.dwLowDateTime) /
        10;
    GetProcessMemoryInfo(T->PI.Process, &Counters, sizeof(Counters));

    TaskProcessInformation TPI(T->PI.Pid, UTime, STime,
                               Counters.PeakWorkingSetSize);
    bool ErrorOccurred = false;
    if (Crashed) {
      if (Cbs.TaskSignalled) {
        TaskFinishedResponse Response =
            Cbs.TaskSignalled(T->PI.Pid, "", StdoutContents, StderrContents,
                              T->Context, T->PI.ReturnCode, TPI);
        ErrorOccurred = (Response == TaskFinishedResponse::StopExecution);
      } else {
        // If we don't have a Signalled callback, unconditionally stop.
        ErrorOccurred = true;
      }
    } else {
      // Wait() returned a normal return code, so just indicate that the task
      // finished.
      if (Cbs.TaskFinished) {
        TaskFinishedResponse Response =
            Cbs.TaskFinished(T->PI.Pid, T->PI.ReturnCode, StdoutContents,
                             StderrContents, TPI, T->Context);
        ErrorOccurred = (Response == TaskFinishedResponse::StopExecution);
      } else if (Crashed) {
        ErrorOccurred = true;
      }
    }
    toolchain::sys::fs::remove(T->StdoutPath);
    if (T->SeparateErrors)
      toolchain::sys::fs::remove(T->StderrPath);
    return ErrorOccurred;
  }

public:
  TaskMonitor(std::queue<std::unique_ptr<Task>> &TasksToBeExecuted,
              unsigned NumberOfParallelTasks, Callbacks Cbs)
      : TasksToBeExecuted(TasksToBeExecuted),
        MaxNumberOfParallelTasks(std::max(NumberOfParallelTasks, 1u)),
        Cbs(std::move(Cbs)) {}

  /// Run the tasks to be executed.
  /// \return true on error.
  bool executeTasks() {
    bool ErrorOccurred = false;
    while ((!ErrorOccurred && !TasksToBeExecuted.empty()) ||
           !TasksBeingExecuted.empty()) {
      if (!ErrorOccurred)
        ErrorOccurred |= startUpSomeTasks();
      ErrorOccurred |= waitForFinishedTask();
    }
    return ErrorOccurred;
  }
};

} // end namespace

bool TaskQueue::supportsBufferingOutput() { return true; }

bool TaskQueue::supportsParallelExecution() { return true; }

unsigned TaskQueue::getNumberOfParallelTasks() const {
  return NumberOfParallelTasks;
}

void TaskQueue::addTask(const char *ExecPath, ArrayRef<const char *> Args,
                        ArrayRef<const char *> Env, void *Context,
                        bool SeparateErrors) {
  auto T = std::make_unique<Task>(ExecPath, Args, Env, Context, SeparateErrors);
  QueuedTasks.push(std::move(T));
}

bool TaskQueue::execute(TaskBeganCallback Began, TaskFinishedCallback Finished,
                        TaskSignalledCallback Signalled) {
  TaskMonitor::Callbacks Cbs{std::move(Began), std::move(Finished),
                             std::move(Signalled)};
  TaskMonitor TM(QueuedTasks, NumberOfParallelTasks, std::move(Cbs));
  return TM.executeTasks();
}
