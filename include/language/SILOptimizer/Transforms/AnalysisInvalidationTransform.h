//===--- AnalysisInvalidationTransform.h ----------------------------------===//
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
///
/// This file defines a transform that can be used to explicitly invalidate an
/// analysis in the pass pipeline. It is intended to be used in a situation
/// where passes are reusing state from an analysis and one wants to explicitly
/// placed into the pass pipeline that it is expected that the analysis be
/// invalidated after the series of passes complete. If we were to just place
/// the invalidation in the last run of the passes, it is possible for a
/// programmer later to add another pass to the end of the pipeline. This would
/// then force recomputation and may prevent invalidation from happening. So by
/// doing this, it is clear to someone adding a new pass that this needs to
/// happen last.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_ANALYSISINVALIDATIONTRANSFORM_H
#define LANGUAGE_SILOPTIMIZER_UTILS_ANALYSISINVALIDATIONTRANSFORM_H

#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

namespace language {

template <typename AnalysisTy>
struct AnalysisInvalidationTransform : public SILFunctionTransform {
  void run() override {
    getAnalysis<AnalysisTy>()->invalidate();
  }
};

} // namespace language

#endif
