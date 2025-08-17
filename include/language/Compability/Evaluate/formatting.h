//===-- language/Compability/Evaluate/formatting.h ---------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_EVALUATE_FORMATTING_H_
#define LANGUAGE_COMPABILITY_EVALUATE_FORMATTING_H_

// It is inconvenient in C++ to have toolchain::raw_ostream::operator<<() as a direct
// friend function of a class template with many instantiations, so the
// various representational class templates in lib/Evaluate format themselves
// via AsFortran(toolchain::raw_ostream &) member functions, which the operator<<()
// overload below will call.  Others have AsFortran() member functions that
// return strings.
//
// This header is meant to be included by the headers that define the several
// representational class templates that need it, not by external clients.

#include "language/Compability/Common/indirection.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>
#include <type_traits>

namespace language::Compability::evaluate {

template <typename A>
auto operator<<(toolchain::raw_ostream &o, const A &x) -> decltype(x.AsFortran(o)) {
  return x.AsFortran(o);
}

template <typename A>
auto operator<<(toolchain::raw_ostream &o, const A &x)
    -> decltype(o << x.AsFortran()) {
  return o << x.AsFortran();
}

template <typename A, bool COPYABLE>
auto operator<<(
    toolchain::raw_ostream &o, const language::Compability::common::Indirection<A, COPYABLE> &x)
    -> decltype(o << x.value()) {
  return o << x.value();
}

template <typename A>
auto operator<<(toolchain::raw_ostream &o, const std::optional<A> &x)
    -> decltype(o << *x) {
  if (x) {
    o << *x;
  } else {
    o << "(nullopt)";
  }
  return o;
}
} // namespace language::Compability::evaluate
#endif // FORTRAN_EVALUATE_FORMATTING_H_
