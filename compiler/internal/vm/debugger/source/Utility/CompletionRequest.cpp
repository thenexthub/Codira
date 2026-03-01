//===-- CompletionRequest.cpp ---------------------------------------------===//
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

#include "lldb/Utility/CompletionRequest.h"

using namespace lldb;
using namespace lldb_private;

CompletionRequest::CompletionRequest(llvm::StringRef command_line,
                                     unsigned raw_cursor_pos,
                                     CompletionResult &result)
    : m_command(command_line), m_raw_cursor_pos(raw_cursor_pos),
      m_result(result) {
  assert(raw_cursor_pos <= command_line.size() && "Out of bounds cursor?");

  // We parse the argument up to the cursor, so the last argument in
  // parsed_line is the one containing the cursor, and the cursor is after the
  // last character.
  llvm::StringRef partial_command(command_line.substr(0, raw_cursor_pos));
  m_parsed_line = Args(partial_command);

  if (GetParsedLine().GetArgumentCount() == 0) {
    m_cursor_index = 0;
    m_cursor_char_position = 0;
  } else {
    m_cursor_index = GetParsedLine().GetArgumentCount() - 1U;
    m_cursor_char_position =
        strlen(GetParsedLine().GetArgumentAtIndex(m_cursor_index));
  }

  // The cursor is after a space but the space is not part of the argument.
  // Let's add an empty fake argument to the end to make sure the completion
  // code. Note: The space could be part of the last argument when it's quoted.
  if (partial_command.ends_with(" ") &&
      !GetCursorArgumentPrefix().ends_with(" "))
    AppendEmptyArgument();
}

std::string CompletionResult::Completion::GetUniqueKey() const {

  // We build a unique key for this pair of completion:description. We
  // prefix the key with the length of the completion string. This prevents
  // that we could get any collisions from completions pairs such as these:
  // "foo:", "bar" would be "foo:bar", but will now be: "4foo:bar"
  // "foo", ":bar" would be "foo:bar", but will now be: "3foo:bar"

  std::string result;
  result.append(std::to_string(m_completion.size()));
  result.append(m_completion);
  result.append(std::to_string(static_cast<int>(m_mode)));
  result.append(":");
  result.append(m_descripton);
  return result;
}

void CompletionResult::AddResult(llvm::StringRef completion,
                                 llvm::StringRef description,
                                 CompletionMode mode) {
  Completion r(completion, description, mode);

  // Add the completion if we haven't seen the same value before.
  if (m_added_values.insert(r.GetUniqueKey()).second)
    m_results.push_back(r);
}

void CompletionResult::GetMatches(StringList &matches) const {
  matches.Clear();
  for (const Completion &completion : m_results)
    matches.AppendString(completion.GetCompletion());
}

void CompletionResult::GetDescriptions(StringList &descriptions) const {
  descriptions.Clear();
  for (const Completion &completion : m_results)
    descriptions.AppendString(completion.GetDescription());
}
