//===--- HeaderInclude.h - Header Include -----------------------*- C++ -*-===//
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
///
/// \file
/// Defines enums used when emitting included header information.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_HEADERINCLUDEFORMATKIND_H
#define LANGUAGE_CORE_BASIC_HEADERINCLUDEFORMATKIND_H
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/ErrorHandling.h"
#include <utility>

namespace language::Core {
/// The format in which header information is emitted.
enum HeaderIncludeFormatKind { HIFMT_None, HIFMT_Textual, HIFMT_JSON };

/// Whether header information is filtered or not. If HIFIL_Only_Direct_System
/// is used, only information on system headers directly included from
/// non-system files is emitted. The HIFIL_Direct_Per_File filtering shows the
/// direct imports and includes for each non-system source and header file
/// separately.
enum HeaderIncludeFilteringKind {
  HIFIL_None,
  HIFIL_Only_Direct_System,
  HIFIL_Direct_Per_File
};

inline HeaderIncludeFormatKind
stringToHeaderIncludeFormatKind(const char *Str) {
  return toolchain::StringSwitch<HeaderIncludeFormatKind>(Str)
      .Case("textual", HIFMT_Textual)
      .Case("json", HIFMT_JSON)
      .Default(HIFMT_None);
}

inline bool stringToHeaderIncludeFiltering(const char *Str,
                                           HeaderIncludeFilteringKind &Kind) {
  std::pair<bool, HeaderIncludeFilteringKind> P =
      toolchain::StringSwitch<std::pair<bool, HeaderIncludeFilteringKind>>(Str)
          .Case("none", {true, HIFIL_None})
          .Case("only-direct-system", {true, HIFIL_Only_Direct_System})
          .Case("direct-per-file", {true, HIFIL_Direct_Per_File})
          .Default({false, HIFIL_None});
  Kind = P.second;
  return P.first;
}

inline const char *headerIncludeFormatKindToString(HeaderIncludeFormatKind K) {
  switch (K) {
  case HIFMT_None:
    toolchain_unreachable("unexpected format kind");
  case HIFMT_Textual:
    return "textual";
  case HIFMT_JSON:
    return "json";
  }
  toolchain_unreachable("Unknown HeaderIncludeFormatKind enum");
}

inline const char *
headerIncludeFilteringKindToString(HeaderIncludeFilteringKind K) {
  switch (K) {
  case HIFIL_None:
    return "none";
  case HIFIL_Only_Direct_System:
    return "only-direct-system";
  case HIFIL_Direct_Per_File:
    return "direct-per-file";
  }
  toolchain_unreachable("Unknown HeaderIncludeFilteringKind enum");
}

} // end namespace language::Core

#endif // LANGUAGE_CORE_BASIC_HEADERINCLUDEFORMATKIND_H
