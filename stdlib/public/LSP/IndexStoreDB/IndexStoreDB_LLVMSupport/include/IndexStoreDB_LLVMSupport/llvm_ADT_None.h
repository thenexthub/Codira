//===-- None.h - Simple null value for implicit construction ------*- C++ -*-=//
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
//  This file provides None, an enumerator for use in implicit constructors
//  of various (usually templated) types to make such construction more
//  terse.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ADT_NONE_H
#define LLVM_ADT_NONE_H

#include <IndexStoreDB_LLVMSupport/toolchain_Config_indexstoredb-prefix.h>

namespace toolchain {
/// A simple null object to allow implicit construction of Optional<T>
/// and similar types without having to spell out the specialization's name.
// (constant value 1 in an attempt to workaround MSVC build issue... )
enum class NoneType { None = 1 };
const NoneType None = NoneType::None;
}

#endif
