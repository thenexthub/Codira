//===- toolchain/Support/CBindingWrapping.h - C Interface Wrapping ---*- C++ -*-===//
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
// This file declares the wrapping macros for the C interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_CBINDINGWRAPPING_H
#define LLVM_SUPPORT_CBINDINGWRAPPING_H

#include <IndexStoreDB_LLVMSupport/toolchain-c_Types.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_Casting.h>

#define DEFINE_SIMPLE_CONVERSION_FUNCTIONS(ty, ref)     \
  inline ty *unwrap(ref P) {                            \
    return reinterpret_cast<ty*>(P);                    \
  }                                                     \
                                                        \
  inline ref wrap(const ty *P) {                        \
    return reinterpret_cast<ref>(const_cast<ty*>(P));   \
  }

#define DEFINE_ISA_CONVERSION_FUNCTIONS(ty, ref)        \
  DEFINE_SIMPLE_CONVERSION_FUNCTIONS(ty, ref)           \
                                                        \
  template<typename T>                                  \
  inline T *unwrap(ref P) {                             \
    return cast<T>(unwrap(P));                          \
  }

#define DEFINE_STDCXX_CONVERSION_FUNCTIONS(ty, ref)     \
  DEFINE_SIMPLE_CONVERSION_FUNCTIONS(ty, ref)           \
                                                        \
  template<typename T>                                  \
  inline T *unwrap(ref P) {                             \
    T *Q = (T*)unwrap(P);                               \
    assert(Q && "Invalid cast!");                       \
    return Q;                                           \
  }

#endif
