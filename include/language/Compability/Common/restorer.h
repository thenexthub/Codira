//===-- language/Compability/Common/restorer.h -------------------------*- C++ -*-===//
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

// Utility: before overwriting a variable, capture its value and
// ensure that it will be restored when the Restorer goes out of scope.
//
// int x{3};
// {
//   auto save{common::ScopedSet(x, 4)};
//   // x is now 4
// }
// // x is back to 3

#ifndef LANGUAGE_COMPABILITY_COMMON_RESTORER_H_
#define LANGUAGE_COMPABILITY_COMMON_RESTORER_H_
#include "api-attrs.h"
#include "idioms.h"
namespace language::Compability::common {
template <typename A> class Restorer {
public:
  explicit RT_API_ATTRS Restorer(A &p, A original)
      : p_{p}, original_{std::move(original)} {}
  RT_API_ATTRS ~Restorer() { p_ = std::move(original_); }

  // Inhibit any recreation of this restorer that would result in two restorers
  // trying to restore the same reference.
  Restorer(const Restorer &) = delete;
  Restorer(Restorer &&that) = delete;
  const Restorer &operator=(const Restorer &) = delete;
  const Restorer &operator=(Restorer &&that) = delete;

private:
  A &p_;
  A original_;
};

template <typename A, typename B>
RT_API_ATTRS common::IfNoLvalue<Restorer<A>, B> ScopedSet(A &to, B &&from) {
  A original{std::move(to)};
  to = std::move(from);
  return Restorer<A>{to, std::move(original)};
}
template <typename A, typename B>
RT_API_ATTRS common::IfNoLvalue<Restorer<A>, B> ScopedSet(
    A &to, const B &from) {
  A original{std::move(to)};
  to = from;
  return Restorer<A>{to, std::move(original)};
}
} // namespace language::Compability::common
#endif // FORTRAN_COMMON_RESTORER_H_
