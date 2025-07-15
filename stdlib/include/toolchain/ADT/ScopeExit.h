//===- toolchain/ADT/ScopeExit.h - Execute code at scope exit --------*- C++ -*-===//
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
// This file defines the make_scope_exit function, which executes user-defined
// cleanup logic at scope exit.
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_ADT_SCOPEEXIT_H
#define TOOLCHAIN_ADT_SCOPEEXIT_H

#include "toolchain/Support/Compiler.h"

#include <type_traits>
#include <utility>

namespace toolchain {
namespace detail {

template <typename Callable> class scope_exit {
  Callable ExitFunction;
  bool Engaged = true; // False once moved-from or release()d.

public:
  template <typename Fp>
  explicit scope_exit(Fp &&F) : ExitFunction(std::forward<Fp>(F)) {}

  scope_exit(scope_exit &&Rhs)
      : ExitFunction(std::move(Rhs.ExitFunction)), Engaged(Rhs.Engaged) {
    Rhs.release();
  }
  scope_exit(const scope_exit &) = delete;
  scope_exit &operator=(scope_exit &&) = delete;
  scope_exit &operator=(const scope_exit &) = delete;

  void release() { Engaged = false; }

  ~scope_exit() {
    if (Engaged)
      ExitFunction();
  }
};

} // end namespace detail

// Keeps the callable object that is passed in, and execute it at the
// destruction of the returned object (usually at the scope exit where the
// returned object is kept).
//
// Interface is specified by p0052r2.
template <typename Callable>
[[nodiscard]] detail::scope_exit<typename std::decay<Callable>::type>
make_scope_exit(Callable &&F) {
  return detail::scope_exit<typename std::decay<Callable>::type>(
      std::forward<Callable>(F));
}

} // end namespace toolchain

#endif
