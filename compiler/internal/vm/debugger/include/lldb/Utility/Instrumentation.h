//===-- Instrumentation.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_UTILITY_INSTRUMENTATION_H
#define LLDB_UTILITY_INSTRUMENTATION_H

#include "lldb/Utility/FileSpec.h"
#include "lldb/Utility/Log.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ErrorHandling.h"

#include <map>
#include <thread>
#include <type_traits>

namespace lldb_private {
namespace instrumentation {

template <typename T, std::enable_if_t<std::is_fundamental<T>::value, int> = 0>
inline void stringify_append(llvm::raw_string_ostream &ss, const T &t) {
  ss << t;
}

template <typename T, std::enable_if_t<!std::is_fundamental<T>::value, int> = 0>
inline void stringify_append(llvm::raw_string_ostream &ss, const T &t) {
  ss << &t;
}

template <typename T>
inline void stringify_append(llvm::raw_string_ostream &ss, T *t) {
  ss << reinterpret_cast<void *>(t);
}

template <typename T>
inline void stringify_append(llvm::raw_string_ostream &ss, const T *t) {
  ss << reinterpret_cast<const void *>(t);
}

template <>
inline void stringify_append<char>(llvm::raw_string_ostream &ss,
                                   const char *t) {
  ss << '\"' << t << '\"';
}

template <>
inline void stringify_append<std::nullptr_t>(llvm::raw_string_ostream &ss,
                                             const std::nullptr_t &t) {
  ss << "\"nullptr\"";
}

template <typename Head>
inline void stringify_helper(llvm::raw_string_ostream &ss, const Head &head) {
  stringify_append(ss, head);
}

template <typename Head, typename... Tail>
inline void stringify_helper(llvm::raw_string_ostream &ss, const Head &head,
                             const Tail &...tail) {
  stringify_append(ss, head);
  ss << ", ";
  stringify_helper(ss, tail...);
}

template <typename... Ts> inline std::string stringify_args(const Ts &...ts) {
  std::string buffer;
  llvm::raw_string_ostream ss(buffer);
  stringify_helper(ss, ts...);
  return buffer;
}

/// RAII object for instrumenting LLDB API functions.
class Instrumenter {
public:
  Instrumenter(llvm::StringRef pretty_func, std::string &&pretty_args = {});
  ~Instrumenter();

private:
  void UpdateBoundary();

  llvm::StringRef m_pretty_func;

  /// Whether this function call was the one crossing the API boundary.
  bool m_local_boundary = false;
};
} // namespace instrumentation
} // namespace lldb_private

#define LLDB_INSTRUMENT()                                                      \
  lldb_private::instrumentation::Instrumenter _instr(LLVM_PRETTY_FUNCTION);

#define LLDB_INSTRUMENT_VA(...)                                                \
  lldb_private::instrumentation::Instrumenter _instr(                          \
      LLVM_PRETTY_FUNCTION,                                                    \
      lldb_private::instrumentation::stringify_args(__VA_ARGS__));

#endif // LLDB_UTILITY_INSTRUMENTATION_H
