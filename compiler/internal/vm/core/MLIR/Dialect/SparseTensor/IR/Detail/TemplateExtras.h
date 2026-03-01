//===- TemplateExtras.h -----------------------------------------*- C++ -*-===//
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

#ifndef MLIR_DIALECT_SPARSETENSOR_IR_DETAIL_TEMPLATEEXTRAS_H
#define MLIR_DIALECT_SPARSETENSOR_IR_DETAIL_TEMPLATEEXTRAS_H

#include <utility>

#include "vm/core/ADT/STLExtras.h"
#include "vm/core/Support/raw_ostream.h"

namespace mlir {
namespace sparse_tensor {
namespace ir_detail {

//===----------------------------------------------------------------------===//
template <typename T>
using has_print_method =
    decltype(std::declval<T>().print(std::declval<toolchain::raw_ostream &>()));
template <typename T>
using detect_has_print_method = toolchain::is_detected<has_print_method, T>;
template <typename T, typename R = void>
using enable_if_has_print_method =
    std::enable_if_t<detect_has_print_method<T>::value, R>;

/// Generic template for defining `operator<<` overloads which delegate
/// to `T::print(raw_ostream&) const`.
template <typename T>
inline enable_if_has_print_method<T, toolchain::raw_ostream &>
operator<<(toolchain::raw_ostream &os, T const &t) {
  t.print(os);
  return os;
}

//===----------------------------------------------------------------------===//
template <typename T>
static constexpr bool IsZeroCostAbstraction =
    // These two predicates license the compiler to make optimizations.
    std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T> &&
    // This helps ensure ABI compatibility (e.g., padding and alignment).
    std::is_standard_layout_v<T> &&
    // These two are what SmallVector uses to determine whether it can
    // use memcpy.
    std::is_trivially_copy_constructible<T>::value &&
    std::is_trivially_move_constructible<T>::value;

//===----------------------------------------------------------------------===//

} // namespace ir_detail
} // namespace sparse_tensor
} // namespace mlir

#endif // MLIR_DIALECT_SPARSETENSOR_IR_DETAIL_TEMPLATEEXTRAS_H
