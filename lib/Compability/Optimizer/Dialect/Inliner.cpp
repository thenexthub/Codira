//===-- Inliner.cpp -------------------------------------------------------===//
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

#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "toolchain/Support/CommandLine.h"

static toolchain::cl::opt<bool>
    aggressivelyInline("inline-all",
                       toolchain::cl::desc("aggressively inline everything"),
                       toolchain::cl::init(false));

/// Should we inline the callable `op` into region `reg`?
bool fir::canLegallyInline(mlir::Operation *, mlir::Region *, bool,
                           mlir::IRMapping &) {
  return aggressivelyInline;
}

bool fir::canLegallyInline(mlir::Operation *, mlir::Operation *, bool) {
  return aggressivelyInline;
}
