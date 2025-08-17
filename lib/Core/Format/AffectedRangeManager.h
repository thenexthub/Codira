//===--- AffectedRangeManager.h - Format C++ code ---------------*- C++ -*-===//
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
/// AffectedRangeManager class manages affected ranges in the code.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_FORMAT_AFFECTEDRANGEMANAGER_H
#define LANGUAGE_CORE_LIB_FORMAT_AFFECTEDRANGEMANAGER_H

#include "language/Core/Basic/SourceManager.h"

namespace language::Core {
namespace format {

struct FormatToken;
class AnnotatedLine;

class AffectedRangeManager {
public:
  AffectedRangeManager(const SourceManager &SourceMgr,
                       const ArrayRef<CharSourceRange> Ranges)
      : SourceMgr(SourceMgr), Ranges(Ranges) {}

  // Determines which lines are affected by the SourceRanges given as input.
  // Returns \c true if at least one line in \p Lines or one of their
  // children is affected.
  bool computeAffectedLines(SmallVectorImpl<AnnotatedLine *> &Lines);

  // Returns true if 'Range' intersects with one of the input ranges.
  bool affectsCharSourceRange(const CharSourceRange &Range);

private:
  // Returns true if the range from 'First' to 'Last' intersects with one of the
  // input ranges.
  bool affectsTokenRange(const FormatToken &First, const FormatToken &Last,
                         bool IncludeLeadingNewlines);

  // Returns true if one of the input ranges intersect the leading empty lines
  // before 'Tok'.
  bool affectsLeadingEmptyLines(const FormatToken &Tok);

  // Marks all lines between I and E as well as all their children as affected.
  void markAllAsAffected(ArrayRef<AnnotatedLine *>::iterator I,
                         ArrayRef<AnnotatedLine *>::iterator E);

  // Determines whether 'Line' is affected by the SourceRanges given as input.
  // Returns \c true if line or one if its children is affected.
  bool nonPPLineAffected(AnnotatedLine *Line, const AnnotatedLine *PreviousLine,
                         SmallVectorImpl<AnnotatedLine *> &Lines);

  const SourceManager &SourceMgr;
  const SmallVector<CharSourceRange, 8> Ranges;
};

} // namespace format
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_FORMAT_AFFECTEDRANGEMANAGER_H
