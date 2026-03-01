//===-- OptionGroupVariable.cpp -------------------------------------------===//
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

#include "lldb/Interpreter/OptionGroupVariable.h"

#include "lldb/DataFormatters/DataVisualization.h"
#include "lldb/Host/OptionParser.h"
#include "lldb/Interpreter/CommandInterpreter.h"
#include "lldb/Target/Target.h"
#include "lldb/Utility/Status.h"

using namespace lldb;
using namespace lldb_private;

// if you add any options here, remember to update the counters in
// OptionGroupVariable::GetNumDefinitions()
static constexpr OptionDefinition g_variable_options[] = {
    {LLDB_OPT_SET_1 | LLDB_OPT_SET_2, false, "no-args", 'a',
     OptionParser::eNoArgument, nullptr, {}, 0, eArgTypeNone,
     "Omit function arguments."},
    {LLDB_OPT_SET_1 | LLDB_OPT_SET_2, false, "no-recognized-args", 't',
     OptionParser::eNoArgument, nullptr, {}, 0, eArgTypeNone,
     "Omit recognized function arguments."},
    {LLDB_OPT_SET_1 | LLDB_OPT_SET_2, false, "no-locals", 'l',
     OptionParser::eNoArgument, nullptr, {}, 0, eArgTypeNone,
     "Omit local variables."},
    {LLDB_OPT_SET_1 | LLDB_OPT_SET_2, false, "show-globals", 'g',
     OptionParser::eNoArgument, nullptr, {}, 0, eArgTypeNone,
     "Show the current frame source file global and static variables."},
    {LLDB_OPT_SET_1 | LLDB_OPT_SET_2, false, "show-declaration", 'c',
     OptionParser::eNoArgument, nullptr, {}, 0, eArgTypeNone,
     "Show variable declaration information (source file and line where the "
     "variable was declared)."},
    {LLDB_OPT_SET_1 | LLDB_OPT_SET_2, false, "regex", 'r',
     OptionParser::eNoArgument, nullptr, {}, 0, eArgTypeRegularExpression,
     "The <variable-name> argument for name lookups are regular expressions."},
    {LLDB_OPT_SET_1 | LLDB_OPT_SET_2, false, "scope", 's',
     OptionParser::eNoArgument, nullptr, {}, 0, eArgTypeNone,
     "Show variable scope (argument, local, global, static)."},
    {LLDB_OPT_SET_1, false, "summary", 'y', OptionParser::eRequiredArgument,
     nullptr, {}, 0, eArgTypeName,
     "Specify the summary that the variable output should use."},
    {LLDB_OPT_SET_2, false, "summary-string", 'z',
     OptionParser::eRequiredArgument, nullptr, {}, 0, eArgTypeName,
     "Specify a summary string to use to format the variable output."},
};

static constexpr auto g_num_frame_options = 4;
static const auto g_variable_options_noframe =
    llvm::ArrayRef<OptionDefinition>(g_variable_options)
        .drop_front(g_num_frame_options);

static Status ValidateNamedSummary(const char *str, void *) {
  if (!str || !str[0])
    return Status::FromErrorStringWithFormat(
        "must specify a valid named summary");
  TypeSummaryImplSP summary_sp;
  if (!DataVisualization::NamedSummaryFormats::GetSummaryFormat(
          ConstString(str), summary_sp))
    return Status::FromErrorStringWithFormat(
        "must specify a valid named summary");
  return Status();
}

static Status ValidateSummaryString(const char *str, void *) {
  if (!str || !str[0])
    return Status::FromErrorStringWithFormat(
        "must specify a non-empty summary string");
  return Status();
}

OptionGroupVariable::OptionGroupVariable(bool show_frame_options)
    : include_frame_options(show_frame_options), show_args(false),
      show_recognized_args(false), show_locals(false), show_globals(false),
      use_regex(false), show_scope(false), show_decl(false),
      summary(ValidateNamedSummary), summary_string(ValidateSummaryString) {}

Status
OptionGroupVariable::SetOptionValue(uint32_t option_idx,
                                    llvm::StringRef option_arg,
                                    ExecutionContext *execution_context) {
  Status error;
  llvm::ArrayRef<OptionDefinition> variable_options =
      include_frame_options ? g_variable_options : g_variable_options_noframe;
  const int short_option = variable_options[option_idx].short_option;
  switch (short_option) {
  case 'r':
    use_regex = true;
    break;
  case 'a':
    show_args = false;
    break;
  case 'l':
    show_locals = false;
    break;
  case 'g':
    show_globals = true;
    break;
  case 'c':
    show_decl = true;
    break;
  case 's':
    show_scope = true;
    break;
  case 't':
    show_recognized_args = false;
    break;
  case 'y':
    error = summary.SetCurrentValue(option_arg);
    break;
  case 'z':
    error = summary_string.SetCurrentValue(option_arg);
    break;
  default:
    llvm_unreachable("Unimplemented option");
  }

  return error;
}

void OptionGroupVariable::OptionParsingStarting(
    ExecutionContext *execution_context) {
  show_args = true;     // Frame option only
  show_recognized_args = true; // Frame option only
  show_locals = true;   // Frame option only
  show_globals = false; // Frame option only
  show_decl = false;
  use_regex = false;
  show_scope = false;
  summary.Clear();
  summary_string.Clear();
}

llvm::ArrayRef<OptionDefinition> OptionGroupVariable::GetDefinitions() {
  // Show the "--no-args", "--no-recognized-args", "--no-locals" and
  // "--show-globals" options if we are showing frame specific options
  return include_frame_options ? g_variable_options
                               : g_variable_options_noframe;
}
