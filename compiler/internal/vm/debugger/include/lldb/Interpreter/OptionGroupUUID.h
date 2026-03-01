//===-- OptionGroupUUID.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONGROUPUUID_H
#define LLDB_INTERPRETER_OPTIONGROUPUUID_H

#include "lldb/Interpreter/OptionValueUUID.h"
#include "lldb/Interpreter/Options.h"

namespace lldb_private {

// OptionGroupUUID

class OptionGroupUUID : public OptionGroup {
public:
  OptionGroupUUID() = default;

  ~OptionGroupUUID() override = default;

  llvm::ArrayRef<OptionDefinition> GetDefinitions() override;

  Status SetOptionValue(uint32_t option_idx, llvm::StringRef option_value,
                        ExecutionContext *execution_context) override;

  void OptionParsingStarting(ExecutionContext *execution_context) override;

  const OptionValueUUID &GetOptionValue() const { return m_uuid; }

protected:
  OptionValueUUID m_uuid;
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONGROUPUUID_H
