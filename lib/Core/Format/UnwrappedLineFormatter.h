//===--- UnwrappedLineFormatter.h - Format C++ code -------------*- C++ -*-===//
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
/// Implements a combinatorial exploration of all the different
/// linebreaks unwrapped lines can be formatted in.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_FORMAT_UNWRAPPEDLINEFORMATTER_H
#define LANGUAGE_CORE_LIB_FORMAT_UNWRAPPEDLINEFORMATTER_H

#include "ContinuationIndenter.h"

namespace language::Core {
namespace format {

class ContinuationIndenter;
class WhitespaceManager;

class UnwrappedLineFormatter {
public:
  UnwrappedLineFormatter(ContinuationIndenter *Indenter,
                         WhitespaceManager *Whitespaces,
                         const FormatStyle &Style,
                         const AdditionalKeywords &Keywords,
                         const SourceManager &SourceMgr,
                         FormattingAttemptStatus *Status)
      : Indenter(Indenter), Whitespaces(Whitespaces), Style(Style),
        Keywords(Keywords), SourceMgr(SourceMgr), Status(Status) {}

  /// Format the current block and return the penalty.
  unsigned format(const SmallVectorImpl<AnnotatedLine *> &Lines,
                  bool DryRun = false, int AdditionalIndent = 0,
                  bool FixBadIndentation = false, unsigned FirstStartColumn = 0,
                  unsigned NextStartColumn = 0, unsigned LastStartColumn = 0);

private:
  /// Add a new line and the required indent before the first Token
  /// of the \c UnwrappedLine if there was no structural parsing error.
  void formatFirstToken(const AnnotatedLine &Line,
                        const AnnotatedLine *PreviousLine,
                        const AnnotatedLine *PrevPrevLine,
                        const SmallVectorImpl<AnnotatedLine *> &Lines,
                        unsigned Indent, unsigned NewlineIndent);

  /// Returns the column limit for a line, taking into account whether we
  /// need an escaped newline due to a continued preprocessor directive.
  unsigned getColumnLimit(bool InPPDirective,
                          const AnnotatedLine *NextLine) const;

  // Cache to store the penalty of formatting a vector of AnnotatedLines
  // starting from a specific additional offset. Improves performance if there
  // are many nested blocks.
  std::map<std::pair<const SmallVectorImpl<AnnotatedLine *> *, unsigned>,
           unsigned>
      PenaltyCache;

  ContinuationIndenter *Indenter;
  WhitespaceManager *Whitespaces;
  const FormatStyle &Style;
  const AdditionalKeywords &Keywords;
  const SourceManager &SourceMgr;
  FormattingAttemptStatus *Status;
};
} // end namespace format
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_FORMAT_UNWRAPPEDLINEFORMATTER_H
