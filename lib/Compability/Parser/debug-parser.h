//===-- lib/Parser/debug-parser.h -------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_PARSER_DEBUG_PARSER_H_
#define LANGUAGE_COMPABILITY_PARSER_DEBUG_PARSER_H_

// Implements the parser with syntax "(YOUR MESSAGE HERE)"_debug for use
// in temporary modifications to the grammar intended for tracing the
// flow of the parsers.  Not to be used in production.

#include "basic-parsers.h"
#include "language/Compability/Parser/parse-state.h"
#include <cstddef>
#include <optional>

namespace language::Compability::parser {

class DebugParser {
public:
  using resultType = Success;
  constexpr DebugParser(const DebugParser &) = default;
  constexpr DebugParser(const char *str, std::size_t n)
      : str_{str}, length_{n} {}
  std::optional<Success> Parse(ParseState &) const;

private:
  const char *const str_;
  const std::size_t length_;
};

constexpr DebugParser operator""_debug(const char str[], std::size_t n) {
  return DebugParser{str, n};
}
} // namespace language::Compability::parser
#endif // FORTRAN_PARSER_DEBUG_PARSER_H_
