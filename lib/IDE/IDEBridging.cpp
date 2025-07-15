//===----------------------------------------------------------------------===//
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

#include "language/IDE/IDEBridging.h"
#include "toolchain/Support/raw_ostream.h"
#include <climits>

ResolvedLoc::ResolvedLoc(language::CharSourceRange range,
                         std::vector<language::CharSourceRange> labelRanges,
                         std::optional<unsigned> firstTrailingLabel,
                         LabelRangeType labelType, bool isActive,
                         ResolvedLocContext context)
    : range(range), labelRanges(labelRanges),
      firstTrailingLabel(firstTrailingLabel), labelType(labelType),
      isActive(isActive), context(context) {}

ResolvedLoc::ResolvedLoc() {}

BridgedResolvedLoc::BridgedResolvedLoc(BridgedCharSourceRange range,
                                       BridgedCharSourceRangeVector labelRanges,
                                       unsigned firstTrailingLabel,
                                       LabelRangeType labelType, bool isActive,
                                       ResolvedLocContext context)
    : resolvedLoc(
          new ResolvedLoc(range.unbridged(), labelRanges.takeUnbridged(),
                          firstTrailingLabel == UINT_MAX
                              ? std::nullopt
                              : std::optional<unsigned>(firstTrailingLabel),
                          labelType, isActive, context)) {}

BridgedResolvedLocVector::BridgedResolvedLocVector()
    : vector(new std::vector<BridgedResolvedLoc>()) {}

void BridgedResolvedLocVector::push_back(BridgedResolvedLoc Loc) {
  static_cast<std::vector<ResolvedLoc> *>(vector)->push_back(
      Loc.takeUnbridged());
}

BridgedResolvedLocVector::BridgedResolvedLocVector(void *opaqueValue)
    : vector(opaqueValue) {}

void *BridgedResolvedLocVector::getOpaqueValue() const { return vector; }
