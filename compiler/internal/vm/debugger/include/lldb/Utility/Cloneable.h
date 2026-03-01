//===-- Cloneable.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_CLONEABLE_H
#define LLDB_UTILITY_CLONEABLE_H

#include <memory>
#include <type_traits>

namespace lldb_private {

/// \class Cloneable Cloneable.h "lldb/Utility/Cloneable.h"
/// A class that implements CRTP-based "virtual constructor" idiom.
///
/// Example:
/// @code
/// class Base {
///   using TopmostBase = Base;
/// public:
///   virtual std::shared_ptr<Base> Clone() const = 0;
/// };
/// @endcode
///
/// To define a class derived from the Base with overridden Clone:
/// @code
/// class Intermediate : public Cloneable<Intermediate, Base> {};
/// @endcode
///
/// To define a class at the next level of inheritance with overridden Clone:
/// @code
/// class Derived : public Cloneable<Derived, Intermediate> {};
/// @endcode

template <typename Derived, typename Base>
class Cloneable : public Base {
public:
  using Base::Base;

  std::shared_ptr<typename Base::TopmostBase> Clone() const override {
    // std::is_base_of requires derived type to be complete, that's why class
    // scope static_assert cannot be used.
    static_assert(std::is_base_of<Cloneable, Derived>::value,
                  "Derived class must be derived from this.");

    return std::make_shared<Derived>(static_cast<const Derived &>(*this));
  }
};

} // namespace lldb_private

#endif // LLDB_UTILITY_CLONEABLE_H
