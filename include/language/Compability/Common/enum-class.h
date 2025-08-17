//===-- language/Compability/Common/enum-class.h -----------------------*- C++ -*-===//
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

// The macro
//   ENUM_CLASS(className, enum1, enum2, ..., enumN)
// defines
//   enum class className { enum1, enum2, ... , enumN };
// as well as the introspective utilities
//   static constexpr std::size_t className_enumSize{N};
//   static inline std::string_view EnumToString(className);

#ifndef LANGUAGE_COMPABILITY_COMMON_ENUM_CLASS_H_
#define LANGUAGE_COMPABILITY_COMMON_ENUM_CLASS_H_

#include <array>
#include <functional>
#include <string_view>
namespace language::Compability::common {

constexpr std::size_t CountEnumNames(const char *p) {
  std::size_t n{0};
  std::size_t any{0};
  for (; *p; ++p) {
    if (*p == ',') {
      n += any;
      any = 0;
    } else if (*p != ' ') {
      any = 1;
    }
  }
  return n + any;
}

template <std::size_t ITEMS>
constexpr std::array<std::string_view, ITEMS> EnumNames(const char *p) {
  std::array<std::string_view, ITEMS> result{""};
  std::size_t at{0};
  const char *start{nullptr};
  for (; *p; ++p) {
    if (*p == ',' || *p == ' ') {
      if (start) {
        result[at++] =
            std::string_view{start, static_cast<std::size_t>(p - start)};
        start = nullptr;
      }
    } else if (!start) {
      start = p;
    }
  }
  if (start) {
    result[at] = std::string_view{start, static_cast<std::size_t>(p - start)};
  }
  return result;
}

#define ENUM_CLASS(NAME, ...) \
  enum class NAME { __VA_ARGS__ }; \
  [[maybe_unused]] static constexpr std::size_t NAME##_enumSize{ \
      ::language::Compability::common::CountEnumNames(#__VA_ARGS__)}; \
  [[maybe_unused]] static inline std::size_t EnumToInt(NAME e) { \
    return static_cast<std::size_t>(e); \
  } \
  [[maybe_unused]] static inline std::string_view EnumToString(NAME e) { \
    static const constexpr auto names{ \
        ::language::Compability::common::EnumNames<NAME##_enumSize>(#__VA_ARGS__)}; \
    return names[static_cast<std::size_t>(e)]; \
  } \
  [[maybe_unused]] inline void ForEach##NAME(std::function<void(NAME)> f) { \
    for (std::size_t i{0}; i < NAME##_enumSize; ++i) { \
      f(static_cast<NAME>(i)); \
    } \
  }
} // namespace language::Compability::common
#endif // FORTRAN_COMMON_ENUM_CLASS_H_
