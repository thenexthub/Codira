//===-- Lower/OpenMP/ClauseFinder.h --------------------------*- C++ -*-===//
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
#ifndef LANGUAGE_COMPABILITY_LOWER_CLAUSEFINDER_H
#define LANGUAGE_COMPABILITY_LOWER_CLAUSEFINDER_H

#include "language/Compability/Lower/OpenMP/Clauses.h"

namespace language::Compability {
namespace lower {
namespace omp {

class ClauseFinder {
  using ClauseIterator = List<Clause>::const_iterator;

public:
  /// Utility to find a clause within a range in the clause list.
  template <typename T>
  static ClauseIterator findClause(ClauseIterator begin, ClauseIterator end) {
    for (ClauseIterator it = begin; it != end; ++it) {
      if (std::get_if<T>(&it->u))
        return it;
    }

    return end;
  }

  /// Return the first instance of the given clause found in the clause list or
  /// `nullptr` if not present. If more than one instance is expected, use
  /// `findRepeatableClause` instead.
  template <typename T>
  static const T *findUniqueClause(const List<Clause> &clauses,
                                   const parser::CharBlock **source = nullptr) {
    ClauseIterator it = findClause<T>(clauses.begin(), clauses.end());
    if (it != clauses.end()) {
      if (source)
        *source = &it->source;
      return &std::get<T>(it->u);
    }
    return nullptr;
  }

  /// Call `callbackFn` for each occurrence of the given clause. Return `true`
  /// if at least one instance was found.
  template <typename T>
  static bool findRepeatableClause(
      const List<Clause> &clauses,
      std::function<void(const T &, const parser::CharBlock &source)>
          callbackFn) {
    bool found = false;
    ClauseIterator nextIt, endIt = clauses.end();
    for (ClauseIterator it = clauses.begin(); it != endIt; it = nextIt) {
      nextIt = findClause<T>(it, endIt);

      if (nextIt != endIt) {
        callbackFn(std::get<T>(nextIt->u), nextIt->source);
        found = true;
        ++nextIt;
      }
    }
    return found;
  }
};
} // namespace omp
} // namespace lower
} // namespace language::Compability

#endif // FORTRAN_LOWER_CLAUSEFINDER_H
