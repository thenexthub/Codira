//===-- OptionValueArgs.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_INTERPRETER_OPTIONVALUEARGS_H
#define LLDB_INTERPRETER_OPTIONVALUEARGS_H

#include "lldb/Interpreter/OptionValueArray.h"

namespace lldb_private {

class OptionValueArgs : public Cloneable<OptionValueArgs, OptionValueArray> {
public:
  OptionValueArgs()
      : Cloneable(OptionValue::ConvertTypeToMask(OptionValue::eTypeString)) {}

  ~OptionValueArgs() override = default;

  size_t GetArgs(Args &args) const;

  Type GetType() const override { return eTypeArgs; }
};

} // namespace lldb_private

#endif // LLDB_INTERPRETER_OPTIONVALUEARGS_H
