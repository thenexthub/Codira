//===--- Edit.cpp - Misc edit utilities -----------------------------------===//
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
#include "language/Basic/Edit.h"
#include "language/Basic/SourceManager.h"
#include <algorithm>

using namespace language;

void SourceEdits::addEdit(SourceManager &SM, CharSourceRange Range, 
                          StringRef Text) {
  SourceLoc Loc = Range.getStart();
  unsigned BufID = SM.findBufferContainingLoc(Loc);
  unsigned Offset = SM.getLocOffsetInBuffer(Loc, BufID);
  unsigned Length = Range.getByteLength();
  StringRef Path = SM.getIdentifierForBuffer(BufID);

  // NOTE: We cannot store SourceManager here since this logic is used by a
  // DiagnosticConsumer where the SourceManager may not remain valid. This is
  // the case when e.g build language interfaces, we create a fresh
  // CompilerInstance for a limited scope, but diagnostics are passed outside of
  // it.
  Edits.push_back({Path.str(), Text.str(), Offset, Length});
}

void language::
writeEditsInJson(const SourceEdits &AllEdits, toolchain::raw_ostream &OS) {
  // Sort the edits so they occur from the last to the first within a given
  // source file. That's the order in which applying non-overlapping edits
  // will succeed.
  std::vector<SourceEdits::Edit> allEdits(AllEdits.getEdits().begin(),
                                          AllEdits.getEdits().end());
  std::sort(allEdits.begin(), allEdits.end(),
            [&](const SourceEdits::Edit &lhs, const SourceEdits::Edit &rhs) {
    // Sort first based on the path. This keeps the edits for a given
    // file together.
    if (lhs.Path < rhs.Path)
      return true;
    else if (lhs.Path > rhs.Path)
      return false;

    // Then sort based on offset, with larger offsets coming earlier.
    return lhs.Offset > rhs.Offset;
  });

  // Remove duplicate edits.
  allEdits.erase(
      std::unique(allEdits.begin(), allEdits.end(),
      [&](const SourceEdits::Edit &lhs, const SourceEdits::Edit &rhs) {
        return lhs.Path == rhs.Path && lhs.Text == rhs.Text &&
          lhs.Offset == rhs.Offset && lhs.Length == rhs.Length;
      }),
      allEdits.end());

  OS << "[";
  bool first = true;
  for (auto &Edit : allEdits) {
    if (first) {
      first = false;
    } else {
      OS << ",";
    }
    OS << "\n";
    OS << " {\n";
    OS << "  \"file\": \"";
    OS.write_escaped(Edit.Path) << "\",\n";
    OS << "  \"offset\": " << Edit.Offset << ",\n";
    if (Edit.Length != 0)
      OS << "  \"remove\": " << Edit.Length << ",\n";
    if (!Edit.Text.empty()) {
      OS << "  \"text\": \"";
      OS.write_escaped(Edit.Text) << "\"\n";
    }
    OS << " }";
  }
  OS << "\n]\n";
}
