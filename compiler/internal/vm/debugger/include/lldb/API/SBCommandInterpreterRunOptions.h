//===-- SBCommandInterpreterRunOptions.h ------------------------*- C++ -*-===//
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

#ifndef LLDB_API_SBCOMMANDINTERPRETERRUNOPTIONS_H
#define LLDB_API_SBCOMMANDINTERPRETERRUNOPTIONS_H

#include <memory>

#include "lldb/API/SBDefines.h"

namespace lldb_private {
class CommandInterpreterRunOptions;
class CommandInterpreterRunResult;
} // namespace lldb_private

namespace lldb {

class LLDB_API SBCommandInterpreterRunOptions {
  friend class SBDebugger;
  friend class SBCommandInterpreter;

public:
  SBCommandInterpreterRunOptions();
  SBCommandInterpreterRunOptions(const SBCommandInterpreterRunOptions &rhs);
  ~SBCommandInterpreterRunOptions();

  SBCommandInterpreterRunOptions &
  operator=(const SBCommandInterpreterRunOptions &rhs);

  bool GetStopOnContinue() const;

  void SetStopOnContinue(bool);

  bool GetStopOnError() const;

  void SetStopOnError(bool);

  bool GetStopOnCrash() const;

  void SetStopOnCrash(bool);

  bool GetEchoCommands() const;

  void SetEchoCommands(bool);

  bool GetEchoCommentCommands() const;

  void SetEchoCommentCommands(bool echo);

  bool GetPrintResults() const;

  void SetPrintResults(bool);

  bool GetPrintErrors() const;

  void SetPrintErrors(bool);

  bool GetAddToHistory() const;

  void SetAddToHistory(bool);

  bool GetAutoHandleEvents() const;

  void SetAutoHandleEvents(bool);

  bool GetSpawnThread() const;

  void SetSpawnThread(bool);

  bool GetAllowRepeats() const;

  /// By default, RunCommandInterpreter will discard repeats if the
  /// IOHandler being used is not interactive.  Setting AllowRepeats to true
  /// will override this behavior and always process empty lines in the input
  /// as a repeat command.
  void SetAllowRepeats(bool);

private:
  lldb_private::CommandInterpreterRunOptions *get() const;

  lldb_private::CommandInterpreterRunOptions &ref() const;

  // This is set in the constructor and will always be valid.
  mutable std::unique_ptr<lldb_private::CommandInterpreterRunOptions>
      m_opaque_up;
};

#ifndef SWIG
class LLDB_API SBCommandInterpreterRunResult {
  friend class SBDebugger;
  friend class SBCommandInterpreter;

public:
  SBCommandInterpreterRunResult();
  SBCommandInterpreterRunResult(const SBCommandInterpreterRunResult &rhs);
  ~SBCommandInterpreterRunResult();

  SBCommandInterpreterRunResult &
  operator=(const SBCommandInterpreterRunResult &rhs);

  int GetNumberOfErrors() const;
  lldb::CommandInterpreterResult GetResult() const;

private:
  SBCommandInterpreterRunResult(
      const lldb_private::CommandInterpreterRunResult &rhs);

  // This is set in the constructor and will always be valid.
  std::unique_ptr<lldb_private::CommandInterpreterRunResult> m_opaque_up;
};
#endif

} // namespace lldb

#endif // LLDB_API_SBCOMMANDINTERPRETERRUNOPTIONS_H
