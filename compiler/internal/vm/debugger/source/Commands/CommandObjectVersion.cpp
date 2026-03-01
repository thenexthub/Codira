//===-- CommandObjectVersion.cpp ------------------------------------------===//
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

#include "CommandObjectVersion.h"

#include "lldb/Core/Debugger.h"
#include "lldb/Interpreter/CommandReturnObject.h"
#include "lldb/Version/Version.h"
#include "llvm/ADT/StringExtras.h"

using namespace lldb;
using namespace lldb_private;

#define LLDB_OPTIONS_version
#include "CommandOptions.inc"

llvm::ArrayRef<OptionDefinition>
CommandObjectVersion::CommandOptions::GetDefinitions() {
  return llvm::ArrayRef(g_version_options);
}

CommandObjectVersion::CommandObjectVersion(CommandInterpreter &interpreter)
    : CommandObjectParsed(interpreter, "version",
                          "Show the LLDB debugger version.", "version") {}

CommandObjectVersion::~CommandObjectVersion() = default;

// Dump the array values on a single line.
static void dump(const StructuredData::Array &array, Stream &s) {
  std::vector<std::string> values;
  array.ForEach([&](StructuredData::Object *object) -> bool {
    values.emplace_back(object->GetStringValue().str());
    return true;
  });

  s << '[' << llvm::join(values, ", ") << ']';
}

// The default dump output is too verbose.
static void dump(const StructuredData::Dictionary &config, Stream &s) {
  config.ForEach(
      [&](llvm::StringRef key, StructuredData::Object *object) -> bool {
        assert(object);

        StructuredData::Dictionary *value_dict = object->GetAsDictionary();
        assert(value_dict);

        StructuredData::ObjectSP value_sp = value_dict->GetValueForKey("value");
        assert(value_sp);

        s << "  " << key << ": ";
        if (StructuredData::Boolean *boolean = value_sp->GetAsBoolean())
          s << (boolean->GetValue() ? "yes" : "no");
        else if (StructuredData::Array *array = value_sp->GetAsArray())
          dump(*array, s);
        s << '\n';

        return true;
      });
}

void CommandObjectVersion::DoExecute(Args &args, CommandReturnObject &result) {
  result.AppendMessageWithFormat("%s\n", lldb_private::GetVersion());

  if (m_options.verbose)
    dump(*Debugger::GetBuildConfiguration(), result.GetOutputStream());

  result.SetStatus(eReturnStatusSuccessFinishResult);
}
