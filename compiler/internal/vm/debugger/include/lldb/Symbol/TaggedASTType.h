//===-- TaggedASTType.h -----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SYMBOL_TAGGEDASTTYPE_H
#define LLDB_SYMBOL_TAGGEDASTTYPE_H

#include "lldb/Symbol/CompilerType.h"

namespace lldb_private {

// For cases in which there are multiple classes of types that are not
// interchangeable, to allow static type checking.
template <unsigned int C> class TaggedASTType : public CompilerType {
public:
  TaggedASTType(const CompilerType &compiler_type)
      : CompilerType(compiler_type) {}

  TaggedASTType(lldb::opaque_compiler_type_t type,
                lldb::TypeSystemWP type_system)
      : CompilerType(type_system, type) {}

  TaggedASTType(const TaggedASTType<C> &tw) : CompilerType(tw) {}

  TaggedASTType() : CompilerType() {}

  virtual ~TaggedASTType() = default;

  TaggedASTType<C> &operator=(const TaggedASTType<C> &tw) {
    CompilerType::operator=(tw);
    return *this;
  }
};

// Commonly-used tagged types, so code using them is interoperable
typedef TaggedASTType<0> TypeFromParser;
typedef TaggedASTType<1> TypeFromUser;
}

#endif
