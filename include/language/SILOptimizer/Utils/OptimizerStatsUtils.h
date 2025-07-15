//===--- OptimizerStatsUtils.h - Utils for collecting stats  --*- C++ ---*-===//
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

#ifndef LANGUAGE_OPTIMIZER_STATS_UTILS_H
#define LANGUAGE_OPTIMIZER_STATS_UTILS_H

namespace language {
class SILModule;
class SILTransform;
class SILPassManager;

/// Updates SILModule stats before executing the transform \p Transform.
///
/// \param M SILModule to be processed
/// \param Transform the SIL transformation that was just executed
/// \param PM the PassManager being used
void updateSILModuleStatsBeforeTransform(SILModule &M, SILTransform *Transform,
                                         SILPassManager &PM, int PassNumber);

/// Updates SILModule stats after finishing executing the
/// transform \p Transform.
///
/// \param M SILModule to be processed
/// \param Transform the SIL transformation that was just executed
/// \param PM the PassManager being used
void updateSILModuleStatsAfterTransform(SILModule &M, SILTransform *Transform,
                                        SILPassManager &PM, int PassNumber,
                                        int Duration);

} // end namespace language

#endif
