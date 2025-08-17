//===-- Optimizer/Support/Matcher.h -----------------------------*- C++ -*-===//
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_MATCHER_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_MATCHER_H

#include "language/Compability/Common/idioms.h"
#include <variant>

// Boilerplate CRTP class for a simplified type-casing syntactic sugar. This
// lets one write pattern matchers using a more compact syntax.
namespace fir::details {
// clang-format off
template<class... Ts> struct matches : Ts... { using Ts::operator()...; };
template<class... Ts> matches(Ts...) -> matches<Ts...>;
template<typename N> struct matcher {
  template<typename... Ts> auto match(Ts... ts) {
    return language::Compability::common::visit(matches{ts...}, static_cast<N*>(this)->matchee());
  }
  template<typename... Ts> auto match(Ts... ts) const {
    return language::Compability::common::visit(matches{ts...}, static_cast<N const*>(this)->matchee());
  }
};
// clang-format on
} // namespace fir::details

#endif // FORTRAN_OPTIMIZER_SUPPORT_MATCHER_H
