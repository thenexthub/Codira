//===-------------- lib/Support/Hashing.cpp -------------------------------===//
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
// This file provides implementation bits for the LLVM common hashing
// infrastructure. Documentation and most of the other information is in the
// header file.
//
//===----------------------------------------------------------------------===//

#include <IndexStoreDB_LLVMSupport/toolchain_ADT_Hashing.h>

using namespace toolchain;

// Provide a definition and static initializer for the fixed seed. This
// initializer should always be zero to ensure its value can never appear to be
// non-zero, even during dynamic initialization.
uint64_t toolchain::hashing::detail::fixed_seed_override = 0;

// Implement the function for forced setting of the fixed seed.
// FIXME: Use atomic operations here so that there is no data race.
void toolchain::set_fixed_execution_hash_seed(uint64_t fixed_value) {
  hashing::detail::fixed_seed_override = fixed_value;
}
