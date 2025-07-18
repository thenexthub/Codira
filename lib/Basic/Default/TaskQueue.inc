//===--- TaskQueue.inc - Default serial TaskQueue ---------------*- C++ -*-===//
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
/// This file contains a platform-agnostic implementation of TaskQueue
/// using the functions from toolchain/Support/Program.h.
///
/// \note The default implementation of TaskQueue does not support parallel
/// execution, nor does it support output buffering. As a result,
/// platform-specific implementations should be preferred.
///
//===----------------------------------------------------------------------===//

#include "language/Basic/TaskQueue.h"
#include "language/Basic/Toolchain.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Signals.h"

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#include <psapi.h>
#endif

#include "Task.inc"

using namespace toolchain::sys;

bool TaskQueue::supportsBufferingOutput() {
  // The default implementation supports buffering output.
  return true;
}

bool TaskQueue::supportsParallelExecution() {
  // The default implementation does not support parallel execution.
  return false;
}

unsigned TaskQueue::getNumberOfParallelTasks() const {
  // The default implementation does not support parallel execution.
  return 1;
}

void TaskQueue::addTask(const char *ExecPath, ArrayRef<const char *> Args,
                        ArrayRef<const char *> Env, void *Context,
                        bool SeparateErrors) {
  auto T = make_unique<Task>(ExecPath, Args, Env, Context, SeparateErrors);
  QueuedTasks.push(std::move(T));
}

bool TaskQueue::execute(TaskBeganCallback Began, TaskFinishedCallback Finished,
                        TaskSignalledCallback Signalled) {
  bool ContinueExecution = true;

  // This implementation of TaskQueue doesn't support parallel execution.
  // We need to reference NumberOfParallelTasks to avoid warnings, though.
  (void)NumberOfParallelTasks;

  while (!QueuedTasks.empty() && ContinueExecution) {
    std::unique_ptr<Task> T(QueuedTasks.front().release());
    QueuedTasks.pop();
    bool ExecutionFailed = T->execute();

    if (ExecutionFailed) {
      return true;
    }

    if (Began) {
      Began(T->PI.Pid, T->Context);
    }

    std::string ErrMsg;
    T->PI = Wait(T->PI, 0, true, &ErrMsg);
    int ReturnCode = T->PI.ReturnCode;

    auto StdoutBuffer = toolchain::MemoryBuffer::getFile(T->StdoutPath);
    StringRef StdoutContents = StdoutBuffer.get()->getBuffer();

    StringRef StderrContents;
    if (T->SeparateErrors) {
      auto StderrBuffer = toolchain::MemoryBuffer::getFile(T->StderrPath);
      StderrContents = StderrBuffer.get()->getBuffer();
    }

#if defined(_WIN32)
    // Wait() sets the upper two bits of the return code to indicate warnings
    // (10) and errors (11).
    //
    // This isn't a true signal on Windows, but we'll treat it as such so that
    // we clean up after it properly
    bool Crashed = ReturnCode & 0xC0000000;

    FILETIME CreationTime;
    FILETIME ExitTime;
    FILETIME UtimeTicks;
    FILETIME StimeTicks;
    PROCESS_MEMORY_COUNTERS Counters = {};
    GetProcessTimes(T->PI.Process, &CreationTime, &ExitTime, &StimeTicks,
                    &UtimeTicks);
    // Each tick is 100ns
    uint64_t Utime =
        ((uint64_t)UtimeTicks.dwHighDateTime << 32 | UtimeTicks.dwLowDateTime) /
        10;
    uint64_t Stime =
        ((uint64_t)StimeTicks.dwHighDateTime << 32 | StimeTicks.dwLowDateTime) /
        10;
    GetProcessMemoryInfo(T->PI.Process, &Counters, sizeof(Counters));

    TaskProcessInformation TPI(T->PI.Pid, Utime, Stime,
                               Counters.PeakWorkingSetSize);
#else
    // Wait() returning a return code of -2 indicates the process received
    // a signal during execution.
    bool Crashed = ReturnCode == -2;
    TaskProcessInformation TPI(T->PI.Pid);
#endif
    if (Crashed) {
      if (Signalled) {
        TaskFinishedResponse Response =
            Signalled(T->PI.Pid, ErrMsg, StdoutContents, StderrContents,
                      T->Context, ReturnCode, TPI);
        ContinueExecution = Response != TaskFinishedResponse::StopExecution;
      } else {
        // If we don't have a Signalled callback, unconditionally stop.
        ContinueExecution = false;
      }
    } else {
      // Wait() returned a normal return code, so just indicate that the task
      // finished.
      if (Finished) {
        TaskFinishedResponse Response =
            Finished(T->PI.Pid, T->PI.ReturnCode, StdoutContents,
                     StderrContents, TPI, T->Context);
        ContinueExecution = Response != TaskFinishedResponse::StopExecution;
      } else if (T->PI.ReturnCode != 0) {
        ContinueExecution = false;
      }
    }
    toolchain::sys::fs::remove(T->StdoutPath);
    if (T->SeparateErrors)
      toolchain::sys::fs::remove(T->StderrPath);
  }

  return !ContinueExecution;
}
