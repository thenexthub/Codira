//===-- lib/Support/OpenMP-utils.cpp ----------------------------*- C++ -*-===//
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

#include "language/Compability/Support/OpenMP-utils.h"

#include "mlir/IR/OpDefinition.h"

namespace language::Compability::common::openmp {
mlir::Block *genEntryBlock(mlir::OpBuilder &builder, const EntryBlockArgs &args,
    mlir::Region &region) {
  assert(args.isValid() && "invalid args");
  assert(region.empty() && "non-empty region");

  toolchain::SmallVector<mlir::Type> types;
  toolchain::SmallVector<mlir::Location> locs;
  unsigned numVars = args.hasDeviceAddr.vars.size() + args.hostEvalVars.size() +
      args.inReduction.vars.size() + args.map.vars.size() +
      args.priv.vars.size() + args.reduction.vars.size() +
      args.taskReduction.vars.size() + args.useDeviceAddr.vars.size() +
      args.useDevicePtr.vars.size();
  types.reserve(numVars);
  locs.reserve(numVars);

  auto extractTypeLoc = [&types, &locs](toolchain::ArrayRef<mlir::Value> vals) {
    toolchain::transform(vals, std::back_inserter(types),
        [](mlir::Value v) { return v.getType(); });
    toolchain::transform(vals, std::back_inserter(locs),
        [](mlir::Value v) { return v.getLoc(); });
  };

  // Populate block arguments in clause name alphabetical order to match
  // expected order by the BlockArgOpenMPOpInterface.
  extractTypeLoc(args.hasDeviceAddr.vars);
  extractTypeLoc(args.hostEvalVars);
  extractTypeLoc(args.inReduction.vars);
  extractTypeLoc(args.map.vars);
  extractTypeLoc(args.priv.vars);
  extractTypeLoc(args.reduction.vars);
  extractTypeLoc(args.taskReduction.vars);
  extractTypeLoc(args.useDeviceAddr.vars);
  extractTypeLoc(args.useDevicePtr.vars);

  return builder.createBlock(&region, {}, types, locs);
}
} // namespace language::Compability::common::openmp
