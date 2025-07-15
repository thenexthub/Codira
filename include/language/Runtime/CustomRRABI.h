//===--- CustomRRABI.h - Custom retain/release ABI support ----------------===//
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
// Utilities for creating register-specific retain/release entrypoints.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_CUSTOMRRABI_H
#define LANGUAGE_RUNTIME_CUSTOMRRABI_H

namespace language {

#if __arm64__ || defined(_M_ARM64)

// Invoke the macro X on the number of each register we support for a custom ABI
// entrypoint, along with a custom parameter. We don't support all 31 registers:
// - x0 is already covered by the standard entrypoints.
// - x16/x17 are scratch registers that can be used by procedure call glue.
// - x18 is reserved.
// - x29 is the frame pointer.
// - x30 is the link register and gets overwritten when making a call.
#define CUSTOM_RR_ENTRYPOINTS_FOREACH_REG(X, param)                            \
  X(1, param)                                                                  \
  X(2, param)                                                                  \
  X(3, param)                                                                  \
  X(4, param)                                                                  \
  X(5, param)                                                                  \
  X(6, param)                                                                  \
  X(7, param)                                                                  \
  X(8, param)                                                                  \
  X(9, param)                                                                  \
  X(10, param)                                                                 \
  X(11, param)                                                                 \
  X(12, param)                                                                 \
  X(13, param)                                                                 \
  X(14, param)                                                                 \
  X(15, param)                                                                 \
  X(19, param)                                                                 \
  X(20, param)                                                                 \
  X(21, param)                                                                 \
  X(22, param)                                                                 \
  X(23, param)                                                                 \
  X(24, param)                                                                 \
  X(25, param)                                                                 \
  X(26, param)                                                                 \
  X(27, param)                                                                 \
  X(28, param)

// Helper template for deducing the parameter type of a one-parameter function.
template <typename Ret, typename Param>
Param returnTypeHelper(Ret (*)(Param)) {}

#if defined(__LP64__) || defined(_LP64)
#define REGISTER_SUBSTITUTION_PREFIX ""
#define REGISTER_PREFIX "x"
#else
#define REGISTER_SUBSTITUTION_PREFIX "w"
#define REGISTER_PREFIX "w"
#endif

// Helper macro that defines one entrypoint that takes the parameter in reg and
// calls through to function.
#define CUSTOM_RR_ENTRYPOINTS_DEFINE_ONE_ENTRYPOINT(reg, function)             \
  LANGUAGE_RUNTIME_EXPORT decltype(function(nullptr)) function##_x##reg() {       \
    decltype(returnTypeHelper(function)) ptr;                                  \
    asm(".ifnc %" REGISTER_SUBSTITUTION_PREFIX "0, " REGISTER_PREFIX #reg "\n" \
        "mov %" REGISTER_SUBSTITUTION_PREFIX "0, " REGISTER_PREFIX #reg "\n"   \
        ".endif"                                                               \
        : "=r"(ptr));                                                          \
    return function(ptr);                                                      \
  }

// A macro that defines all register-specific entrypoints for the given
// retain/release function.
#define CUSTOM_RR_ENTRYPOINTS_DEFINE_ENTRYPOINTS(function)                     \
  CUSTOM_RR_ENTRYPOINTS_FOREACH_REG(                                           \
      CUSTOM_RR_ENTRYPOINTS_DEFINE_ONE_ENTRYPOINT, function)

#else

// No custom entrypoints on other architectures.
#define CUSTOM_RR_ENTRYPOINTS_DEFINE_ENTRYPOINTS(function)

#endif

} // namespace language

#endif // LANGUAGE_RUNTIME_CUSTOMRRABI_H
