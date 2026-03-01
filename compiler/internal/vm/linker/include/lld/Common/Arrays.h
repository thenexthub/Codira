//===- Arrays.h ------------------------------------------------*- C++ -*-===//
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

#ifndef LLD_ARRAYS_H
#define LLD_ARRAYS_H

#include "llvm/ADT/ArrayRef.h"

#include <vector>

namespace lld {
// Split one uint8 array into small pieces of uint8 arrays.
inline std::vector<llvm::ArrayRef<uint8_t>> split(llvm::ArrayRef<uint8_t> arr,
                                                  size_t chunkSize) {
  std::vector<llvm::ArrayRef<uint8_t>> ret;
  while (arr.size() > chunkSize) {
    ret.push_back(arr.take_front(chunkSize));
    arr = arr.drop_front(chunkSize);
  }
  if (!arr.empty())
    ret.push_back(arr);
  return ret;
}

} // namespace lld

#endif
