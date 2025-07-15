//===--- Edit.h - Misc edit utilities ---------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_EDIT_H
#define LANGUAGE_BASIC_EDIT_H

#include "toolchain/ADT/ArrayRef.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/SourceLoc.h"

namespace language {
  class SourceManager;
  class CharSourceRange;

  /// A set of edits made to a source file.
  class SourceEdits final {
  public:
    struct Edit {
      std::string Path;
      std::string Text;
      unsigned Offset;
      unsigned Length;
    };

  private:
    std::vector<Edit> Edits;

  public:
    ArrayRef<Edit> getEdits() const { return Edits; }

    void addEdit(SourceManager &SM, CharSourceRange Range, StringRef Text);
  };

  void writeEditsInJson(const SourceEdits &Edits, toolchain::raw_ostream &OS);
}

#endif // LANGUAGE_BASIC_EDIT_H
