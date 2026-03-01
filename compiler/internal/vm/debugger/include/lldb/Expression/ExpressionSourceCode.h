//===-- ExpressionSourceCode.h ----------------------------------*- C++ -*-===//
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

#ifndef LLDB_EXPRESSION_EXPRESSIONSOURCECODE_H
#define LLDB_EXPRESSION_EXPRESSIONSOURCECODE_H

#include "lldb/lldb-enumerations.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

#include <string>

namespace lldb_private {

class ExpressionSourceCode {
protected:
  enum Wrapping : bool {
    Wrap = true,
    NoWrap = false,
  };

public:
  bool NeedsWrapping() const { return m_wrap == Wrap; }

  const char *GetName() const { return m_name.c_str(); }

protected:
  ExpressionSourceCode(llvm::StringRef name, llvm::StringRef prefix,
                       llvm::StringRef body, Wrapping wrap)
      : m_name(name.str()), m_prefix(prefix.str()), m_body(body.str()),
        m_wrap(wrap) {}

  std::string m_name;
  std::string m_prefix;
  std::string m_body;
  Wrapping m_wrap;
};

} // namespace lldb_private

#endif
