//===- COFFReader.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_OBJCOPY_COFF_COFFREADER_H
#define LLVM_LIB_OBJCOPY_COFF_COFFREADER_H

#include "vm/core/BinaryFormat/COFF.h"
#include "vm/core/Object/COFF.h"
#include "vm/core/Support/Error.h"

namespace vm::core {
namespace objcopy {
namespace coff {

struct Object;

using object::COFFObjectFile;

class COFFReader {
  const COFFObjectFile &COFFObj;

  Error readExecutableHeaders(Object &Obj) const;
  Error readSections(Object &Obj) const;
  Error readSymbols(Object &Obj, bool IsBigObj) const;
  Error setSymbolTargets(Object &Obj) const;

public:
  explicit COFFReader(const COFFObjectFile &O) : COFFObj(O) {}
  Expected<std::unique_ptr<Object>> create() const;
};

} // end namespace coff
} // end namespace objcopy
} // end namespace vm::core

#endif // LLVM_LIB_OBJCOPY_COFF_COFFREADER_H
