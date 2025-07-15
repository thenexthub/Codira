//===--- Debug.h - Requirement machine debugging flags ----------*- C++ -*-===//
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

#ifndef LANGUAGE_RQM_DEBUG_H
#define LANGUAGE_RQM_DEBUG_H

#include "language/Basic/OptionSet.h"

namespace language {

namespace rewriting {

enum class DebugFlags : unsigned {
  /// Print debug output when simplifying terms.
  Simplify = (1<<0),

  /// Print debug output when adding rules.
  Add = (1<<1),

  /// Print debug output from the Knuth-Bendix algorithm.
  Completion = (1<<2),

  /// Print debug output from property map construction.
  PropertyMap = (1<<3),

  /// Print debug output when unifying concrete types in the property map.
  ConcreteUnification = (1<<4),

  /// Print debug output when concretizing nested types in the property map.
  ConcretizeNestedTypes = (1<<5),

  /// Print debug output when inferring conditional requirements in the
  /// property map.
  ConditionalRequirements = (1<<6),

  /// Print debug output from the homotopy reduction algorithm.
  HomotopyReduction = (1<<7),

  /// Print more detailed debug output from the homotopy reduction algorithm.
  HomotopyReductionDetail = (1<<8),

  /// Print debug output from the minimal conformances algorithm.
  MinimalConformances = (1<<9),

  /// Print more detailed debug output from the minimal conformances algorithm.
  MinimalConformancesDetail = (1<<10),

  /// Print debug output from the protocol dependency graph.
  ProtocolDependencies = (1<<11),

  /// Print debug output from generic signature minimization.
  Minimization = (1<<12),

  /// Print redundant rules and their replacement paths.
  RedundantRules = (1<<13),

  /// Print more detail about redundant rules.
  RedundantRulesDetail = (1<<14),

  /// Print debug output from the concrete contraction pre-processing pass.
  ConcreteContraction = (1<<15),

  /// Print a trace of requirement machines constructed and how long each took.
  Timers = (1<<16),

  /// Print conflicting rules.
  ConflictingRules = (1<<17),

  /// Print debug output from concrete equivalence class splitting during
  /// minimization.
  SplitConcreteEquivalenceClass = (1<<18),
};

using DebugOptions = OptionSet<DebugFlags>;

}

}

#endif
