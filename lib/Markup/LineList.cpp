//===--- LineList.cpp - Data structures for Markup parsing ----------------===//
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

#include "language/AST/RawComment.h"
#include "language/Basic/PrimitiveParsing.h"
#include "language/Markup/LineList.h"
#include "language/Markup/Markup.h"

using namespace language;
using namespace markup;

std::string LineList::str() const {
  std::string Result;
  toolchain::raw_string_ostream Stream(Result);
  if (Lines.empty())
    return "";

  Line *FirstNonEmptyLine = Lines.begin();
  while (FirstNonEmptyLine != Lines.end() && FirstNonEmptyLine->Text.empty())
    ++FirstNonEmptyLine;

  if (FirstNonEmptyLine == Lines.end())
    return "";

  auto InitialIndentation = measureIndentation(FirstNonEmptyLine->Text);

  Stream << FirstNonEmptyLine->Text.drop_front(InitialIndentation);
  for (auto Line = FirstNonEmptyLine + 1; Line != Lines.end(); ++Line) {
    auto Drop = std::min(InitialIndentation, Line->FirstNonspaceOffset);
    Stream << '\n' << Line->Text.drop_front(Drop);
  }

  Stream.flush();
  return Result;
}

size_t language::markup::measureIndentation(StringRef Text) {
  static constexpr toolchain::StringLiteral IndentChars(" \v\f\t");
  size_t FirstNonIndentPos = Text.find_first_not_of(IndentChars);
  if (FirstNonIndentPos == StringRef::npos)
    return Text.size();
  return FirstNonIndentPos;
}

void LineListBuilder::addLine(toolchain::StringRef Text, language::SourceRange Range) {
  Lines.push_back({Text, Range});
}

LineList LineListBuilder::takeLineList() const {
  return LineList(Context.allocateCopy(ArrayRef<Line>(Lines)));
}

static unsigned measureASCIIArt(StringRef S, unsigned NumLeadingSpaces) {
  StringRef Spaces = S.substr(0, NumLeadingSpaces);
  if (Spaces.size() != NumLeadingSpaces)
    return 0;
  if (Spaces.find_first_not_of(' ') != StringRef::npos)
    return 0;

  S = S.drop_front(NumLeadingSpaces);

  if (S.starts_with(" * "))
    return NumLeadingSpaces + 3;
  if (S.starts_with(" *\n") || S.starts_with(" *\r\n"))
    return NumLeadingSpaces + 2;
  return 0;
}

LineList MarkupContext::getLineList(language::RawComment RC) {
  LineListBuilder Builder(*this);

  for (const auto &C : RC.Comments) {
    if (C.isLine()) {
      // Skip comment marker.
      unsigned CommentMarkerBytes = 2 + (C.isOrdinary() ? 0 : 1);
      StringRef Cleaned = C.RawText.drop_front(CommentMarkerBytes);

      // Drop trailing newline.
      Cleaned = Cleaned.rtrim("\n\r");
      auto CleanedStartLoc =
          C.Range.getStart().getAdvancedLocOrInvalid(CommentMarkerBytes);
      auto CleanedEndLoc =
          CleanedStartLoc.getAdvancedLocOrInvalid(Cleaned.size());
      Builder.addLine(Cleaned, { CleanedStartLoc, CleanedEndLoc });
    } else {
      // Skip comment markers at the beginning and at the end.
      unsigned CommentMarkerBytes = 2 + (C.isOrdinary() ? 0 : 1);
      StringRef Cleaned = C.RawText.drop_front(CommentMarkerBytes);

      if (Cleaned.ends_with("*/"))
        Cleaned = Cleaned.drop_back(2);
      else if (Cleaned.ends_with("/"))
        Cleaned = Cleaned.drop_back(1);

      language::SourceLoc CleanedStartLoc =
          C.Range.getStart().getAdvancedLocOrInvalid(CommentMarkerBytes);

      // Determine if we have leading decorations in this block comment.
      bool HasASCIIArt = false;
      if (language::startsWithNewline(Cleaned)) {
        unsigned NewlineBytes = language::measureNewline(Cleaned);
        Cleaned = Cleaned.drop_front(NewlineBytes);
        CleanedStartLoc = CleanedStartLoc.getAdvancedLocOrInvalid(NewlineBytes);
        HasASCIIArt = measureASCIIArt(Cleaned, C.ColumnIndent - 1) != 0;
      }

      while (!Cleaned.empty()) {
        size_t Pos = Cleaned.find_first_of("\n\r");
        if (Pos == StringRef::npos)
          Pos = Cleaned.size();

        // Skip over ASCII art, if present.
        if (HasASCIIArt)
          if (unsigned ASCIIArtBytes =
                  measureASCIIArt(Cleaned, C.ColumnIndent - 1)) {
            Cleaned = Cleaned.drop_front(ASCIIArtBytes);
            CleanedStartLoc =
            CleanedStartLoc.getAdvancedLocOrInvalid(ASCIIArtBytes);
            Pos -= ASCIIArtBytes;
          }

        StringRef Line = Cleaned.substr(0, Pos);
        auto CleanedEndLoc = CleanedStartLoc.getAdvancedLocOrInvalid(Pos);

        Builder.addLine(Line, { CleanedStartLoc, CleanedEndLoc });

        Cleaned = Cleaned.drop_front(Pos);
        unsigned NewlineBytes = language::measureNewline(Cleaned);
        Cleaned = Cleaned.drop_front(NewlineBytes);
        Pos += NewlineBytes;
        CleanedStartLoc = CleanedStartLoc.getAdvancedLocOrInvalid(Pos);
      }
    }
  }
  return Builder.takeLineList();
}

LineList LineList::subListWithRange(MarkupContext &MC, size_t StartLine,
                                    size_t EndLine, size_t StartColumn,
                                    size_t EndColumn) const {
  auto TrimmedLines = ArrayRef<Line>(Lines.begin() + StartLine,
                                     Lines.begin() + EndLine);

  LineListBuilder Builder(MC);
  if (TrimmedLines.empty())
    return LineList();

  auto FirstLine = TrimmedLines.begin();
  auto End = TrimmedLines.end();
  auto LastLine = End - 1;

  for (auto Line = FirstLine; Line != End; ++Line) {
    auto T = Line->Text;
    auto RangeStart = Line->Range.Start;
    auto RangeEnd = Line->Range.End;

    if (Line == LastLine) {
      T = T.drop_back(T.size() - EndColumn);
      RangeEnd = RangeStart.getAdvancedLocOrInvalid(EndColumn);
    }

    if (Line == FirstLine) {
      T = T.drop_front(StartColumn);
      RangeStart = RangeStart.getAdvancedLocOrInvalid(StartColumn);
    }

    Builder.addLine(T, {RangeStart, RangeEnd});
  }

  return Builder.takeLineList();
}
