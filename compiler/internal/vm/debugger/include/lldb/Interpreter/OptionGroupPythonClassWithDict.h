//===-- OptionGroupPythonClassWithDict.h ------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONGROUPPYTHONCLASSWITHDICT_H
#define LLDB_INTERPRETER_OPTIONGROUPPYTHONCLASSWITHDICT_H

#include "lldb/Interpreter/Options.h"
#include "lldb/Utility/Flags.h"
#include "lldb/Utility/StructuredData.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

// Use this Option group if you have a python class that implements some
// Python extension point, and you pass a SBStructuredData to the class 
// __init__ method.  
// class_option specifies the class name
// the key and value options are read in in pairs, and a 
// StructuredData::Dictionary is constructed with those pairs.
class OptionGroupPythonClassWithDict : public OptionGroup {
public:
  enum OptionKind {
    eScriptClass    = 1 << 0,
    eDictKey        = 1 << 1,
    eDictValue      = 1 << 2,
    ePythonFunction = 1 << 3,
    eAllOptions     = (eScriptClass | eDictKey | eDictValue | ePythonFunction)
  };

  OptionGroupPythonClassWithDict(const char *class_use, bool is_class = true,
                                 int class_option = 'C', int key_option = 'k',
                                 int value_option = 'v',
                                 uint16_t required_options = eScriptClass |
                                                             ePythonFunction);

  ~OptionGroupPythonClassWithDict() override = default;

  llvm::ArrayRef<OptionDefinition> GetDefinitions() override {
    return llvm::ArrayRef<OptionDefinition>(m_option_definition);
  }

  Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_value,
                        ExecutionContext *execution_context) override;

  void OptionParsingStarting(ExecutionContext *execution_context) override;
  Status OptionParsingFinished(ExecutionContext *execution_context) override;
  
  const StructuredData::DictionarySP GetStructuredData() {
    return m_dict_sp;
  }
  const std::string &GetName() {
    return m_name;
  }

protected:
  std::string m_name;
  std::string m_current_key;
  StructuredData::DictionarySP m_dict_sp;
  std::string m_class_usage_text, m_key_usage_text, m_value_usage_text;
  bool m_is_class;
  OptionDefinition m_option_definition[4];
  Flags m_required_options;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONGROUPPYTHONCLASSWITHDICT_H
