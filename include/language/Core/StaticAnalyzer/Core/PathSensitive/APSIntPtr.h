//== APSIntPtr.h - Wrapper for APSInt objects owned separately -*- C++ -*--==//
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

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_APSIntPtr_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_APSIntPtr_H

#include "toolchain/ADT/APSInt.h"
#include "toolchain/Support/Compiler.h"

namespace language::Core::ento {

/// A safe wrapper around APSInt objects allocated and owned by
/// \c BasicValueFactory. This just wraps a common toolchain::APSInt.
class APSIntPtr {
  using APSInt = toolchain::APSInt;

public:
  APSIntPtr() = delete;
  APSIntPtr(const APSIntPtr &) = default;
  APSIntPtr &operator=(const APSIntPtr &) & = default;
  ~APSIntPtr() = default;

  /// You should not use this API.
  /// If do, ensure that the \p Ptr not going to dangle.
  /// Prefer using \c BasicValueFactory::getValue() to get an APSIntPtr object.
  static APSIntPtr unsafeConstructor(const APSInt *Ptr) {
    return APSIntPtr(Ptr);
  }

  LLVM_ATTRIBUTE_RETURNS_NONNULL
  const APSInt *get() const { return Ptr; }
  /*implicit*/ operator const APSInt &() const { return *get(); }

  APSInt operator-() const { return -*Ptr; }
  APSInt operator~() const { return ~*Ptr; }

#define DEFINE_OPERATOR(OP)                                                    \
  bool operator OP(APSIntPtr Other) const { return (*Ptr)OP(*Other.Ptr); }
  DEFINE_OPERATOR(>)
  DEFINE_OPERATOR(>=)
  DEFINE_OPERATOR(<)
  DEFINE_OPERATOR(<=)
  DEFINE_OPERATOR(==)
  DEFINE_OPERATOR(!=)
#undef DEFINE_OPERATOR

  const APSInt &operator*() const { return *Ptr; }
  const APSInt *operator->() const { return Ptr; }

private:
  explicit APSIntPtr(const APSInt *Ptr) : Ptr(Ptr) {}

  /// Owned by \c BasicValueFactory.
  const APSInt *Ptr;
};

} // namespace language::Core::ento

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_APSIntPtr_H
