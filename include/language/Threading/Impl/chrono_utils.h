//===--- chrono_utils.h - Utility functions for duration ------ -*- C++ -*-===//
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
// Specifically, we want ceil() for these types, but that's only available in
// C++17, and we need to build with C++14, so... include a version of the
// necesary code here.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_THREADING_IMPL_CHRONO_UTILS_H
#define LANGUAGE_THREADING_IMPL_CHRONO_UTILS_H

#include <chrono>
#include <type_traits>

namespace language {
namespace threading_impl {
namespace chrono_utils {

#if __cplusplus >= 201703L
using std::chrono::ceil;
#else

namespace detail {
template <class _Tp>
struct is_duration : std::false_type {};

template <class _Rep, class _Period>
struct is_duration<std::chrono::duration<_Rep, _Period> >
  : std::true_type  {};

template <class _Rep, class _Period>
struct is_duration<const std::chrono::duration<_Rep, _Period> >
  : std::true_type  {};

template <class _Rep, class _Period>
struct is_duration<volatile std::chrono::duration<_Rep, _Period> >
  : std::true_type  {};

template <class _Rep, class _Period>
struct is_duration<const volatile std::chrono::duration<_Rep, _Period> >
  : std::true_type  {};
}

template <class To, class Rep, class Period,
          class = std::enable_if_t<detail::is_duration<To>::value>>
constexpr To
ceil(const std::chrono::duration<Rep, Period>& d)
{
  To t = std::chrono::duration_cast<To>(d);
  if (t < d)
    t = t + To{1};
  return t;
}

#endif

} // namespace chrono_utils
} // namespace threading_impl
} // namespace language

#endif // LANGUAGE_THREADING_IMPL_CHRONO_UTILS_H
