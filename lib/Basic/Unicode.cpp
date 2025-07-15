//===--- Unicode.cpp - Unicode utilities ----------------------------------===//
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

#include "language/Basic/Assertions.h"
#include "language/Basic/Unicode.h"
#include "language/Basic/Compiler.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/ConvertUTF.h"

using namespace language;

// HACK: Allow support for many newer emoji by overriding behavior of ZWJ and
// emoji modifiers. This does not make the breaks correct for any version of
// Unicode, but shifts the ways in which it is incorrect to be less harmful.
//
// TODO: Remove this hack and reevaluate whether we should have any static
// notion of what a grapheme is.
//
// Returns true if lhs and rhs shouldn't be considered as having a grapheme
// break between them. That is, whether we're overriding the behavior of the
// hard coded Unicode 8 rules surrounding ZWJ and emoji modifiers.
static inline bool graphemeBreakOverride(toolchain::UTF32 lhs, toolchain::UTF32 rhs) {
  // Assume ZWJ sequences produce new emoji
  if (lhs == 0x200D) {
    return true;
  }

  // Permit continuing regional indicators
  if (rhs >= 0x1F3FB && rhs <= 0x1F3FF) {
    return true;
  }

  // Permit emoji tag sequences
  if (rhs >= 0xE0020 && rhs <= 0xE007F) {
    return true;
  }

  return false;
}

StringRef language::unicode::extractFirstExtendedGraphemeCluster(StringRef S) {
  // Extended grapheme cluster segmentation algorithm as described in Unicode
  // Standard Annex #29.
  if (S.empty())
    return StringRef();

  const toolchain::UTF8 *SourceStart =
    reinterpret_cast<const toolchain::UTF8 *>(S.data());

  const toolchain::UTF8 *SourceNext = SourceStart;
  toolchain::UTF32 C[2];
  toolchain::UTF32 *TargetStart = C;

  ConvertUTF8toUTF32(&SourceNext, SourceStart + S.size(), &TargetStart, C + 1,
                     toolchain::lenientConversion);
  if (TargetStart == C) {
    // The source string contains an ill-formed subsequence at the end.
    return S;
  }

  GraphemeClusterBreakProperty GCBForC0 = getGraphemeClusterBreakProperty(C[0]);
  while (true) {
    size_t C1Offset = SourceNext - SourceStart;
    ConvertUTF8toUTF32(&SourceNext, SourceStart + S.size(), &TargetStart, C + 2,
                       toolchain::lenientConversion);

    if (TargetStart == C + 1) {
      // End of source string or the source string contains an ill-formed
      // subsequence at the end.
      return S.slice(0, C1Offset);
    }

    GraphemeClusterBreakProperty GCBForC1 =
        getGraphemeClusterBreakProperty(C[1]);
    if (isExtendedGraphemeClusterBoundary(GCBForC0, GCBForC1) &&
        !graphemeBreakOverride(C[0], C[1]))
      return S.slice(0, C1Offset);

    C[0] = C[1];
    TargetStart = C + 1;
    GCBForC0 = GCBForC1;
  }
}

static bool extractFirstUnicodeScalarImpl(StringRef S, unsigned &Scalar) {
  if (S.empty())
    return false;

  const toolchain::UTF8 *SourceStart =
    reinterpret_cast<const toolchain::UTF8 *>(S.data());

  const toolchain::UTF8 *SourceNext = SourceStart;
  toolchain::UTF32 C;
  toolchain::UTF32 *TargetStart = &C;

  ConvertUTF8toUTF32(&SourceNext, SourceStart + S.size(), &TargetStart,
                     TargetStart + 1, toolchain::lenientConversion);
  if (TargetStart == &C) {
    // The source string contains an ill-formed subsequence at the end.
    return false;
  }

  Scalar = C;
  return size_t(SourceNext - SourceStart) == S.size();
}

bool language::unicode::isSingleUnicodeScalar(StringRef S) {
  unsigned Scalar;
  return extractFirstUnicodeScalarImpl(S, Scalar);
}

unsigned language::unicode::extractFirstUnicodeScalar(StringRef S) {
  unsigned Scalar;
  bool Result = extractFirstUnicodeScalarImpl(S, Scalar);
  assert(Result && "string does not consist of one Unicode scalar");
  (void)Result;
  return Scalar;
}

bool language::unicode::isWellFormedUTF8(StringRef S) {
  const toolchain::UTF8 *begin = S.bytes_begin();
  return toolchain::isLegalUTF8String(&begin, S.bytes_end());
}

std::string language::unicode::sanitizeUTF8(StringRef Text) {
  toolchain::SmallString<256> Builder;
  Builder.reserve(Text.size());
  const toolchain::UTF8* Data = reinterpret_cast<const toolchain::UTF8*>(Text.begin());
  const toolchain::UTF8* End = reinterpret_cast<const toolchain::UTF8*>(Text.end());
  StringRef Replacement = LANGUAGE_UTF8("\ufffd");
  while (Data < End) {
    auto Step = toolchain::getNumBytesForUTF8(*Data);
    if (Data + Step > End) {
      Builder.append(Replacement);
      break;
    }

    if (toolchain::isLegalUTF8Sequence(Data, Data + Step)) {
      Builder.append(Data, Data + Step);
    } else {

      // If malformed, add replacement characters.
      Builder.append(Replacement);
    }
    Data += Step;
  }
  return std::string(Builder.str());
}
