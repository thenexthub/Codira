//===--- AlignOf.h - Portable calculation of type alignment -----*- C++ -*-===//
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
// This file defines the AlignedCharArrayUnion class.
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SUPPORT_ALIGNOF_H
#define TOOLCHAIN_SUPPORT_ALIGNOF_H

#include <type_traits>

inline namespace __language { inline namespace __runtime {
namespace toolchain {

/// A suitably aligned and sized character array member which can hold elements
/// of any type.
///
/// This template is equivalent to std::aligned_union_t<1, ...>, but we cannot
/// use it due to a bug in the MSVC x86 compiler:
/// https://github.com/microsoft/STL/issues/1533
/// Using `alignas` here works around the bug.
template <typename T, typename... Ts> struct AlignedCharArrayUnion {
  using AlignedUnion = std::aligned_union_t<1, T, Ts...>;
  alignas(alignof(AlignedUnion)) char buffer[sizeof(AlignedUnion)];
};

} // end namespace toolchain
}} // namespace language::runtime

#endif // TOOLCHAIN_SUPPORT_ALIGNOF_H
