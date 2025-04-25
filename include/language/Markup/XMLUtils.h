//===--- XMLUtils.h - Various XML utility routines --------------*- C++ -*-===//
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

#ifndef SWIFT_MARKUP_XML_UTILS_H
#define SWIFT_MARKUP_XML_UTILS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

namespace language {
namespace markup {

// FIXME: copied from Clang's
// CommentASTToXMLConverter::appendToResultWithXMLEscaping
static inline void appendWithXMLEscaping(raw_ostream &OS, StringRef S) {
  auto Start = S.begin(), Cursor = Start, End = S.end();
  for (; Cursor != End; ++Cursor) {
    switch (*Cursor) {
    case '&':
      OS.write(Start, Cursor - Start);
      OS << "&amp;";
      break;
    case '<':
      OS.write(Start, Cursor - Start);
      OS << "&lt;";
      break;
    case '>':
      OS.write(Start, Cursor - Start);
      OS << "&gt;";
      break;
    case '"':
      OS.write(Start, Cursor - Start);
      OS << "&quot;";
      break;
    case '\'':
      OS.write(Start, Cursor - Start);
      OS << "&apos;";
      break;
    default:
      continue;
    }
    Start = Cursor + 1;
  }
  OS.write(Start, Cursor - Start);
}

// FIXME: copied from Clang's
// CommentASTToXMLConverter::appendToResultWithCDATAEscaping
static inline void appendWithCDATAEscaping(raw_ostream &OS, StringRef S) {
  if (S.empty())
    return;

  OS << "<![CDATA[";
  while (!S.empty()) {
    size_t Pos = S.find("]]>");
    if (Pos == 0) {
      OS << "]]]]><![CDATA[>";
      S = S.drop_front(3);
      continue;
    }
    if (Pos == StringRef::npos)
      Pos = S.size();

    OS << S.substr(0, Pos);

    S = S.drop_front(Pos);
  }
  OS << "]]>";
}

} // namespace markup
} // namespace language

#endif // SWIFT_MARKUP_XML_UTILS_H

