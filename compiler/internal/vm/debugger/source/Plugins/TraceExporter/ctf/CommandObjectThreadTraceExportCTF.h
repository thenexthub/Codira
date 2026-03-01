//===-- CommandObjectThreadTraceExportCTF.h -------------------*- C++ //-*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_COMMANDOBJECTTHREADTRACEEXPORTCTF_H
#define LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_COMMANDOBJECTTHREADTRACEEXPORTCTF_H

#include "TraceExporterCTF.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Interpreter/CommandReturnObject.h"
#include <optional>

namespace lldb_private {
namespace ctf {

class CommandObjectThreadTraceExportCTF : public CommandObjectParsed {
public:
  class CommandOptions : public Options {
  public:
    CommandOptions() : Options() { OptionParsingStarting(nullptr); }

    Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_arg,
                          ExecutionContext *execution_context) override;

    void OptionParsingStarting(ExecutionContext *execution_context) override;

    llvm::ArrayRef<OptionDefinition> GetDefinitions() override;

    std::optional<size_t> m_thread_index;
    std::string m_file;
  };

  CommandObjectThreadTraceExportCTF(CommandInterpreter &interpreter)
      : CommandObjectParsed(
            interpreter, "thread trace export ctf",
            "Export a given thread's trace to Chrome Trace Format",
            "thread trace export ctf [<ctf-options>]",
            lldb::eCommandRequiresProcess | lldb::eCommandTryTargetAPILock |
                lldb::eCommandProcessMustBeLaunched |
                lldb::eCommandProcessMustBePaused |
                lldb::eCommandProcessMustBeTraced),
        m_options() {}

  Options *GetOptions() override { return &m_options; }

protected:
  void DoExecute(Args &args, CommandReturnObject &result) override;

  CommandOptions m_options;
};

} // namespace ctf
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_COMMANDOBJECTTHREADTRACEEXPORTCTF_H
