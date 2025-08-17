//===- ScheduleOrderedAssignments.h --- Assignment scheduling ---*- C++ -*-===//
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
// This file defines a utility to analyze and schedule the evaluation of
// of hlfir::OrderedAssignmentTreeOpInterface trees that represent Fortran
// Forall, Where, user defined assignments and assignments to vector
// subscripted entities.
//===----------------------------------------------------------------------===//

#ifndef OPTIMIZER_HLFIR_TRANSFORM_SCHEDULEORDEREDASSIGNMENTS_H
#define OPTIMIZER_HLFIR_TRANSFORM_SCHEDULEORDEREDASSIGNMENTS_H

#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"

namespace hlfir {

/// Structure to represent that the value yielded by some region
/// must be fully evaluated and saved for all index values at
/// a given point of the ordered assignment tree evaluation.
/// All subsequent evaluation depending on the value yielded
/// by this region will use the value that was saved.
struct SaveEntity {
  mlir::Region *yieldRegion;
  /// Returns the hlfir.yield op argument.
  mlir::Value getSavedValue();
};

/// A run is a list of actions required to evaluate an ordered assignment tree
/// that can be done in the same loop nest.
/// The actions can evaluate and saves element values into temporary or evaluate
/// assignments.
/// The evaluation of an action in a run will cause the evaluation of all the
/// regions that yield entities required to implement the action, except if the
/// region was saved in a previous run, in which case it will use the previously
/// saved value.
struct Run {
  /// An action is either saving the values yielded by a region, or evaluating
  /// the assignment part of an hlfir::RegionAssignOp.
  using Action = std::variant<hlfir::RegionAssignOp, SaveEntity>;
  toolchain::SmallVector<Action> actions;
  toolchain::SmallVector<mlir::MemoryEffects::EffectInstance> memoryEffects;
};

/// List of runs to be executed in order to evaluate an order assignment tree.
using Schedule = toolchain::SmallVector<Run>;

/// Example of schedules and run, and what they mean:
///  Fortran: forall (i=i:10) x(i) = y(i)
///
///  hlfir.forall lb { hlfir.yield %c1} ub { hlfir.yield %c10} do {
///   ^bb1(%i: index)
///     hlfir.region_assign {
///        %yi_addr = hlfir.designate %y(%i)
///        %yi = fir.load %yi_addr
///        hlfir.yield %yi
///     } to {
///        %xi = hlfir.designate %x(%i)
///        hlfir.yield %xi
///     }
///  }
///
///  If the scheduling analysis cannot prove that %x and %y do not overlap, it
///  will generate 2 runs for the schdule. The first containing
///  SaveEntity{rhs_region}, and the second one containing the
///  hlfir.region_assign.
///
///  The lowering of that schedule will have to:
///  For the first run:
///   1. create a temporary to contain all the %yi for all %i
///   2. create a loop nest for the forall, evaluate the %yi and save them
///   inside the loop, but do not evaluate the LHS or assignment.
///   For the second run:
///   3. create a loop nest again for the forall, evaluate the LHS, get the
///   saved %yi, and evaluate %yi to %xi. After all runs:
///   4. clean the temporary for the %yi.
///
/// If the scheduling analysis can prove %x and %y do not overlap, it will
/// generate only one run with the hlfir.region_assign, which will be
/// implemented as a single loop that evaluate %xi, %yi and does %xi = %yi in
/// the loop body.

/// Core function that analyzes an ordered assignment tree and builds a
/// schedule for its evaluation.
/// The main goal of the scheduler is to avoid creating temporary storage
/// (required for SaveEntity). But it can optionally be asked to fuse Forall
/// and Where assignments in the same loop nests when possible since it has the
/// memory effects analysis at hand.
Schedule buildEvaluationSchedule(hlfir::OrderedAssignmentTreeOpInterface root,
                                 bool tryFusingAssignments);

} // namespace hlfir
#endif // OPTIMIZER_HLFIR_TRANSFORM_SCHEDULEORDERASSIGNMENTS_H
