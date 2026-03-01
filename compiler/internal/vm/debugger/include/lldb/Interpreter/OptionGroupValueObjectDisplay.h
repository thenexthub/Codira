//===-- OptionGroupValueObjectDisplay.h -------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONGROUPVALUEOBJECTDISPLAY_H
#define LLDB_INTERPRETER_OPTIONGROUPVALUEOBJECTDISPLAY_H

#include "lldb/Interpreter/Options.h"
#include "lldb/ValueObject/ValueObject.h"

namespace lldb_private {

// OptionGroupValueObjectDisplay

class OptionGroupValueObjectDisplay : public OptionGroup {
public:
  OptionGroupValueObjectDisplay() = default;

  ~OptionGroupValueObjectDisplay() override = default;

  llvm::ArrayRef<OptionDefinition> GetDefinitions() override;

  Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_value,
                        ExecutionContext *execution_context) override;

  void OptionParsingStarting(ExecutionContext *execution_context) override;

  bool AnyOptionWasSet() const {
    return show_types || no_summary_depth != 0 || show_location ||
           flat_output || use_object_desc || max_depth != UINT32_MAX ||
           ptr_depth != 0 || !use_synth || be_raw || ignore_cap ||
           run_validator;
  }

  DumpValueObjectOptions GetAsDumpOptions(
      LanguageRuntimeDescriptionDisplayVerbosity lang_descr_verbosity =
          eLanguageRuntimeDescriptionDisplayVerbosityFull,
      lldb::Format format = lldb::eFormatDefault,
      lldb::TypeSummaryImplSP summary_sp = lldb::TypeSummaryImplSP());

  bool show_types : 1, show_location : 1, flat_output : 1, use_object_desc : 1,
      use_synth : 1, be_raw : 1, ignore_cap : 1, run_validator : 1,
      max_depth_is_default : 1;

  uint32_t no_summary_depth;
  uint32_t max_depth;
  uint32_t ptr_depth;
  uint32_t elem_count;
  lldb::DynamicValueType use_dynamic;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONGROUPVALUEOBJECTDISPLAY_H
