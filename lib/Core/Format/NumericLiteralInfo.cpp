//===--- NumericLiteralInfo.cpp ---------------------------------*- C++ -*-===//
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
/// This file implements the functionality of getting information about a
/// numeric literal string, including 0-based positions of the base letter, the
/// decimal/hexadecimal point, the exponent letter, and the suffix, or npos if
/// absent.
///
//===----------------------------------------------------------------------===//

#include "NumericLiteralInfo.h"
#include "toolchain/ADT/StringExtras.h"

namespace language::Core {
namespace format {

using namespace toolchain;

NumericLiteralInfo::NumericLiteralInfo(StringRef Text, char Separator) {
  if (Text.size() < 2)
    return;

  bool IsHex = false;
  if (Text[0] == '0') {
    switch (Text[1]) {
    case 'x':
    case 'X':
      IsHex = true;
      [[fallthrough]];
    case 'b':
    case 'B':
    case 'o': // JavaScript octal.
    case 'O':
      BaseLetterPos = 1; // e.g. 0xF
      break;
    }
  }

  DotPos = Text.find('.', BaseLetterPos + 1); // e.g. 0x.1 or .1

  // e.g. 1.e2 or 0xFp2
  const auto Pos = DotPos != StringRef::npos ? DotPos + 1 : BaseLetterPos + 2;

  ExponentLetterPos =
      // Trim C++ user-defined suffix as in `1_Pa`.
      (Separator == '\'' ? Text.take_front(Text.find('_')) : Text)
          .find_insensitive(IsHex ? 'p' : 'e', Pos);

  const bool HasExponent = ExponentLetterPos != StringRef::npos;
  SuffixPos = Text.find_if_not(
      [&](char C) {
        return (HasExponent || !IsHex ? isDigit : isHexDigit)(C) ||
               C == Separator;
      },
      HasExponent ? ExponentLetterPos + 2 : Pos); // e.g. 1e-2f
}

} // namespace format
} // namespace language::Core
