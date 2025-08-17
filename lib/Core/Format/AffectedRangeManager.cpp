//===--- AffectedRangeManager.cpp - Format C++ code -----------------------===//
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
/// This file implements AffectRangeManager class.
///
//===----------------------------------------------------------------------===//

#include "AffectedRangeManager.h"

#include "FormatToken.h"
#include "TokenAnnotator.h"

namespace language::Core {
namespace format {

bool AffectedRangeManager::computeAffectedLines(
    SmallVectorImpl<AnnotatedLine *> &Lines) {
  ArrayRef<AnnotatedLine *>::iterator I = Lines.begin();
  ArrayRef<AnnotatedLine *>::iterator E = Lines.end();
  bool SomeLineAffected = false;
  const AnnotatedLine *PreviousLine = nullptr;
  while (I != E) {
    AnnotatedLine *Line = *I;
    assert(Line->First);
    Line->LeadingEmptyLinesAffected = affectsLeadingEmptyLines(*Line->First);

    // If a line is part of a preprocessor directive, it needs to be formatted
    // if any token within the directive is affected.
    if (Line->InPPDirective) {
      FormatToken *Last = Line->Last;
      const auto *PPEnd = I + 1;
      while (PPEnd != E && !(*PPEnd)->First->HasUnescapedNewline) {
        Last = (*PPEnd)->Last;
        ++PPEnd;
      }

      if (affectsTokenRange(*Line->First, *Last,
                            /*IncludeLeadingNewlines=*/false)) {
        SomeLineAffected = true;
        markAllAsAffected(I, PPEnd);
      }
      I = PPEnd;
      continue;
    }

    if (nonPPLineAffected(Line, PreviousLine, Lines))
      SomeLineAffected = true;

    PreviousLine = Line;
    ++I;
  }
  return SomeLineAffected;
}

bool AffectedRangeManager::affectsCharSourceRange(
    const CharSourceRange &Range) {
  for (const CharSourceRange &R : Ranges) {
    if (!SourceMgr.isBeforeInTranslationUnit(Range.getEnd(), R.getBegin()) &&
        !SourceMgr.isBeforeInTranslationUnit(R.getEnd(), Range.getBegin())) {
      return true;
    }
  }
  return false;
}

bool AffectedRangeManager::affectsTokenRange(const FormatToken &First,
                                             const FormatToken &Last,
                                             bool IncludeLeadingNewlines) {
  SourceLocation Start = First.WhitespaceRange.getBegin();
  if (!IncludeLeadingNewlines)
    Start = Start.getLocWithOffset(First.LastNewlineOffset);
  SourceLocation End = Last.getStartOfNonWhitespace();
  End = End.getLocWithOffset(Last.TokenText.size());
  CharSourceRange Range = CharSourceRange::getCharRange(Start, End);
  return affectsCharSourceRange(Range);
}

bool AffectedRangeManager::affectsLeadingEmptyLines(const FormatToken &Tok) {
  CharSourceRange EmptyLineRange = CharSourceRange::getCharRange(
      Tok.WhitespaceRange.getBegin(),
      Tok.WhitespaceRange.getBegin().getLocWithOffset(Tok.LastNewlineOffset));
  return affectsCharSourceRange(EmptyLineRange);
}

void AffectedRangeManager::markAllAsAffected(
    ArrayRef<AnnotatedLine *>::iterator I,
    ArrayRef<AnnotatedLine *>::iterator E) {
  while (I != E) {
    (*I)->Affected = true;
    markAllAsAffected((*I)->Children.begin(), (*I)->Children.end());
    ++I;
  }
}

bool AffectedRangeManager::nonPPLineAffected(
    AnnotatedLine *Line, const AnnotatedLine *PreviousLine,
    SmallVectorImpl<AnnotatedLine *> &Lines) {
  bool SomeLineAffected = false;
  Line->ChildrenAffected = computeAffectedLines(Line->Children);
  if (Line->ChildrenAffected)
    SomeLineAffected = true;

  // Stores whether one of the line's tokens is directly affected.
  bool SomeTokenAffected = false;
  // Stores whether we need to look at the leading newlines of the next token
  // in order to determine whether it was affected.
  bool IncludeLeadingNewlines = false;

  // Stores whether the first child line of any of this line's tokens is
  // affected.
  bool SomeFirstChildAffected = false;

  assert(Line->First);
  for (FormatToken *Tok = Line->First; Tok; Tok = Tok->Next) {
    // Determine whether 'Tok' was affected.
    if (affectsTokenRange(*Tok, *Tok, IncludeLeadingNewlines))
      SomeTokenAffected = true;

    // Determine whether the first child of 'Tok' was affected.
    if (!Tok->Children.empty() && Tok->Children.front()->Affected)
      SomeFirstChildAffected = true;

    IncludeLeadingNewlines = Tok->Children.empty();
  }

  // Was this line moved, i.e. has it previously been on the same line as an
  // affected line?
  bool LineMoved = PreviousLine && PreviousLine->Affected &&
                   Line->First->NewlinesBefore == 0;

  bool IsContinuedComment =
      Line->First->is(tok::comment) && !Line->First->Next &&
      Line->First->NewlinesBefore < 2 && PreviousLine &&
      PreviousLine->Affected && PreviousLine->Last->is(tok::comment);

  bool IsAffectedClosingBrace =
      Line->First->is(tok::r_brace) &&
      Line->MatchingOpeningBlockLineIndex != UnwrappedLine::kInvalidIndex &&
      Lines[Line->MatchingOpeningBlockLineIndex]->Affected;

  if (SomeTokenAffected || SomeFirstChildAffected || LineMoved ||
      IsContinuedComment || IsAffectedClosingBrace) {
    Line->Affected = true;
    SomeLineAffected = true;
  }
  return SomeLineAffected;
}

} // namespace format
} // namespace language::Core
