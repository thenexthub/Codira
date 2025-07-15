//===--- QuotedString.cpp - Printing a string as a quoted string ----------===//
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

#include "toolchain/Support/raw_ostream.h"
#include "language/Basic/QuotedString.h"

using namespace language;

void language::printAsQuotedString(toolchain::raw_ostream &out, toolchain::StringRef text) {
  out << '"';
  for (auto C : text) {
    switch (C) {
    case '\\': out << "\\\\"; break;
    case '\t': out << "\\t"; break;
    case '\n': out << "\\n"; break;
    case '\r': out << "\\r"; break;
    case '"': out << "\\\""; break;
    case '\'': out << '\''; break; // no need to escape these
    case '\0': out << "\\0"; break;
    default:
      auto c = (unsigned char)C;
      // Other ASCII control characters should get escaped.
      if (c < 0x20 || c == 0x7F) {
        static const char hexdigit[] = {
          '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
          'A', 'B', 'C', 'D', 'E', 'F'
        };
        out << "\\u{" << hexdigit[c >> 4] << hexdigit[c & 0xF] << '}';
      } else {
        out << (char)c;
      }
      break;
    }
  }
  out << '"';
}
