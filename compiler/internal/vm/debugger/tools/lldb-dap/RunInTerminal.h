//===-- RunInTerminal.h ----------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_LLDB_DAP_RUNINTERMINAL_H
#define LLDB_TOOLS_LLDB_DAP_RUNINTERMINAL_H

#include "FifoFiles.h"
#include "lldb/API/SBError.h"

#include <future>
#include <memory>
#include <string>

namespace lldb_dap {

enum RunInTerminalMessageKind {
  eRunInTerminalMessageKindPID = 0,
  eRunInTerminalMessageKindError,
  eRunInTerminalMessageKindDidAttach,
};

struct RunInTerminalMessage;
struct RunInTerminalMessagePid;
struct RunInTerminalMessageError;
struct RunInTerminalMessageDidAttach;

struct RunInTerminalMessage {
  RunInTerminalMessage(RunInTerminalMessageKind kind);

  virtual ~RunInTerminalMessage() = default;

  /// Serialize this object to JSON
  virtual llvm::json::Value ToJSON() const = 0;

  const RunInTerminalMessagePid *GetAsPidMessage() const;

  const RunInTerminalMessageError *GetAsErrorMessage() const;

  RunInTerminalMessageKind kind;
};

using RunInTerminalMessageUP = std::unique_ptr<RunInTerminalMessage>;

struct RunInTerminalMessagePid : RunInTerminalMessage {
  RunInTerminalMessagePid(lldb::pid_t pid);

  llvm::json::Value ToJSON() const override;

  lldb::pid_t pid;
};

struct RunInTerminalMessageError : RunInTerminalMessage {
  RunInTerminalMessageError(llvm::StringRef error);

  llvm::json::Value ToJSON() const override;

  std::string error;
};

struct RunInTerminalMessageDidAttach : RunInTerminalMessage {
  RunInTerminalMessageDidAttach();

  llvm::json::Value ToJSON() const override;
};

class RunInTerminalLauncherCommChannel {
public:
  RunInTerminalLauncherCommChannel(llvm::StringRef comm_file);

  /// Wait until the debug adapter attaches.
  ///
  /// \param[in] timeout
  ///     How long to wait to be attached.
  //
  /// \return
  ///     An \a llvm::Error object in case of errors or if this operation times
  ///     out.
  llvm::Error WaitUntilDebugAdapterAttaches(std::chrono::milliseconds timeout);

  /// Notify the debug adapter this process' pid.
  ///
  /// \return
  ///     An \a llvm::Error object in case of errors or if this operation times
  ///     out.
  llvm::Error NotifyPid();

  /// Notify the debug adapter that there's been an error.
  void NotifyError(llvm::StringRef error);

private:
  FifoFileIO m_io;
};

class RunInTerminalDebugAdapterCommChannel {
public:
  RunInTerminalDebugAdapterCommChannel(llvm::StringRef comm_file);

  /// Notify the runInTerminal launcher that it was attached.
  ///
  /// \return
  ///     A future indicated whether the runInTerminal launcher received the
  ///     message correctly or not.
  std::future<lldb::SBError> NotifyDidAttach();

  /// Fetch the pid of the runInTerminal launcher.
  ///
  /// \return
  ///     An \a llvm::Error object in case of errors or if this operation times
  ///     out.
  llvm::Expected<lldb::pid_t> GetLauncherPid();

  /// Fetch any errors emitted by the runInTerminal launcher or return a
  /// default error message if a certain timeout if reached.
  std::string GetLauncherError();

private:
  FifoFileIO m_io;
};

/// Create a fifo file used to communicate the debug adapter with
/// the runInTerminal launcher.
llvm::Expected<std::shared_ptr<FifoFile>> CreateRunInTerminalCommFile();

} // namespace lldb_dap

#endif // LLDB_TOOLS_LLDB_DAP_RUNINTERMINAL_H
