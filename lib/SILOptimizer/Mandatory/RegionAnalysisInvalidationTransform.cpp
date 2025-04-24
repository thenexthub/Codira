//===--- RegionAnalysisInvalidationTransform.cpp --------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2023 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "language/SILOptimizer/Analysis/RegionAnalysis.h"
#include "language/SILOptimizer/Transforms/AnalysisInvalidationTransform.h"

using namespace language;

SILTransform *swift::createRegionAnalysisInvalidationTransform() {
  return new AnalysisInvalidationTransform<RegionAnalysis>();
}
