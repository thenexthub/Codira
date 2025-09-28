//===-- language/Compability-rt/runtime/array-constructor.h ------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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

// External APIs to create temporary storage for array constructors when their
// final extents or length parameters cannot be pre-computed.

#ifndef FLANG_RT_RUNTIME_ARRAY_CONSTRUCTOR_H_
#define FLANG_RT_RUNTIME_ARRAY_CONSTRUCTOR_H_

#include "descriptor.h"
#include "language/Compability/Runtime/array-constructor-consts.h"
#include "language/Compability/Runtime/entry-names.h"
#include <cstdint>

namespace language::Compability::runtime {

// Runtime data structure to hold information about the storage of
// an array constructor being constructed.
struct ArrayConstructorVector {
  RT_API_ATTRS ArrayConstructorVector(class Descriptor &to,
      SubscriptValue nextValuePosition, SubscriptValue actualAllocationSize,
      const char *sourceFile, int sourceLine, bool useValueLengthParameters)
      : to{to}, nextValuePosition{nextValuePosition},
        actualAllocationSize{actualAllocationSize}, sourceFile{sourceFile},
        sourceLine{sourceLine},
        useValueLengthParameters_{useValueLengthParameters} {}

  RT_API_ATTRS bool useValueLengthParameters() const {
    return useValueLengthParameters_;
  }

  class Descriptor &to;
  SubscriptValue nextValuePosition;
  SubscriptValue actualAllocationSize;
  const char *sourceFile;
  int sourceLine;

private:
  unsigned char useValueLengthParameters_ : 1;
};

static_assert(sizeof(language::Compability::runtime::ArrayConstructorVector) <=
        MaxArrayConstructorVectorSizeInBytes,
    "ABI requires sizeof(ArrayConstructorVector) to be smaller than "
    "MaxArrayConstructorVectorSizeInBytes");
static_assert(alignof(language::Compability::runtime::ArrayConstructorVector) <=
        MaxArrayConstructorVectorAlignInBytes,
    "ABI requires alignof(ArrayConstructorVector) to be smaller than "
    "MaxArrayConstructorVectorAlignInBytes");

} // namespace language::Compability::runtime
#endif // FLANG_RT_RUNTIME_ARRAY_CONSTRUCTOR_H_
