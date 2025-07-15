//===--- Task.inc - ---------------------------------------------*- C++ -*-===//
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
/// This file contains the default implementation of the class Task
///
//===----------------------------------------------------------------------===//

#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/Program.h"
#include "toolchain/Support/Signals.h"
#include <optional>

namespace language {
namespace sys {

// Platform-independent implementation of Task,
// a particular platform can provide its own more efficient version.
class Task {
public:
  /// The path to the executable which this Task will execute.
  const char *ExecPath;

  /// Any arguments which should be passed during execution.
  toolchain::ArrayRef<const char *> Args;

  /// The environment which should be used during execution. If empty,
  /// the current process's environment will be used instead.
  toolchain::ArrayRef<const char *> Env;

  /// Context associated with this Task.
  void *Context;

  /// True if the errors of the Task should be stored in Errors instead of
  /// Output.
  bool SeparateErrors;

  toolchain::SmallString<64> StdoutPath;

  toolchain::SmallString<64> StderrPath;

  toolchain::sys::ProcessInfo PI;

  Task(const char *ExecPath, toolchain::ArrayRef<const char *> Args,
       toolchain::ArrayRef<const char *> Env = std::nullopt, void *Context = nullptr,
       bool SeparateErrors = false)
      : ExecPath(ExecPath), Args(Args), Env(Env), Context(Context),
        SeparateErrors(SeparateErrors) {}

  /// Begins execution of this Task.
  /// \returns true on error.
  bool execute();
};

bool Task::execute() {
  toolchain::SmallVector<const char *, 128> Argv;
  Argv.push_back(ExecPath);
  Argv.append(Args.begin(), Args.end());
  Argv.push_back(nullptr);

  std::optional<toolchain::ArrayRef<toolchain::StringRef>> Envp =
      Env.empty() ? decltype(Envp)(std::nullopt)
                  : decltype(Envp)(toolchain::toStringRefArray(Env.data()));

  if (toolchain::sys::fs::createTemporaryFile("stdout", "tmp", StdoutPath))
    return true;
  toolchain::sys::RemoveFileOnSignal(StdoutPath);

  if (SeparateErrors) {
    if (toolchain::sys::fs::createTemporaryFile("stderr", "tmp", StderrPath))
      return true;
    toolchain::sys::RemoveFileOnSignal(StderrPath);
  }

  std::optional<toolchain::StringRef> Redirects[] = {
      std::nullopt,
      toolchain::StringRef{StdoutPath},
      toolchain::StringRef{SeparateErrors ? StderrPath : StdoutPath},
  };

  bool ExecutionFailed = false;
  PI = toolchain::sys::ExecuteNoWait(ExecPath, toolchain::toStringRefArray(Argv.data()),
                                Envp, Redirects, /*memoryLimit*/ 0,
                                /*ErrMsg*/ nullptr, &ExecutionFailed);
  return ExecutionFailed;
}

} // namespace sys
} // namespace language
