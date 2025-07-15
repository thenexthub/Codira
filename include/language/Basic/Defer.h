//===--- Defer.h - 'defer' helper macro -------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// This file defines a 'LANGUAGE_DEFER' macro for performing a cleanup on any exit
// out of a scope.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BASIC_DEFER_H
#define LANGUAGE_BASIC_DEFER_H

#include "toolchain/ADT/ScopeExit.h"

namespace language {
  namespace detail {
    struct DeferTask {};
    template<typename F>
    auto operator+(DeferTask, F &&fn) ->
        decltype(toolchain::make_scope_exit(std::forward<F>(fn))) {
      return toolchain::make_scope_exit(std::forward<F>(fn));
    }
  }
} // end namespace language


#define DEFER_CONCAT_IMPL(x, y) x##y
#define DEFER_MACRO_CONCAT(x, y) DEFER_CONCAT_IMPL(x, y)

/// This macro is used to register a function / lambda to be run on exit from a
/// scope.  Its typical use looks like:
///
///   LANGUAGE_DEFER {
///     stuff
///   };
///
#define LANGUAGE_DEFER                                                            \
  auto DEFER_MACRO_CONCAT(defer_func, __COUNTER__) =                           \
      ::language::detail::DeferTask() + [&]()

#endif // LANGUAGE_BASIC_DEFER_H
