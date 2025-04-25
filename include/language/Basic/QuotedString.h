//===--- QuotedString.h - Print a string in double-quotes -------*- C++ -*-===//
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
/// \file Declares QuotedString, a convenient type for printing a
/// string as a string literal.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_BASIC_QUOTEDSTRING_H
#define SWIFT_BASIC_QUOTEDSTRING_H

#include "llvm/ADT/StringRef.h"

namespace llvm {
  class raw_ostream;
}

namespace language {
  /// Print the given string as if it were a quoted string.
  void printAsQuotedString(llvm::raw_ostream &out, llvm::StringRef text);

  /// A class designed to make it easy to write a string to a stream
  /// as a quoted string.
  class QuotedString {
    llvm::StringRef Text;
  public:
    explicit QuotedString(llvm::StringRef text) : Text(text) {}

    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &out,
                                         QuotedString string) {
      printAsQuotedString(out, string.Text);
      return out;
    }
  };
} // end namespace language

#endif // SWIFT_BASIC_QUOTEDSTRING_H
