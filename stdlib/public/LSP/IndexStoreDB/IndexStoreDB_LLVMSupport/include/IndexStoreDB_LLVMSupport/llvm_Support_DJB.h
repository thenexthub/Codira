//===-- toolchain/Support/DJB.h ---DJB Hash --------------------------*- C++ -*-===//
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
//
// This file contains support for the DJ Bernstein hash function.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_DJB_H
#define LLVM_SUPPORT_DJB_H

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_StringRef.h>

namespace toolchain {

/// The Bernstein hash function used by the DWARF accelerator tables.
inline uint32_t djbHash(StringRef Buffer, uint32_t H = 5381) {
  for (unsigned char C : Buffer.bytes())
    H = (H << 5) + H + C;
  return H;
}

/// Computes the Bernstein hash after folding the input according to the Dwarf 5
/// standard case folding rules.
uint32_t caseFoldingDjbHash(StringRef Buffer, uint32_t H = 5381);
} // namespace toolchain

#endif // LLVM_SUPPORT_DJB_H
