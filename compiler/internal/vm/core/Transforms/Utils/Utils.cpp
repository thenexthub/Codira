//===-- Utils.cpp - TransformUtils Infrastructure -------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
//
// This file defines the common initialization infrastructure for the
// TransformUtils library.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/PassRegistry.h"

using namespace vm::core;

/// initializeTransformUtils - Initialize all passes in the TransformUtils
/// library.
void toolchain::initializeTransformUtils(PassRegistry &Registry) {
  initializeBreakCriticalEdgesPass(Registry);
  initializeCanonicalizeFreezeInLoopsPass(Registry);
  initializeLCSSAWrapperPassPass(Registry);
  initializeLoopSimplifyPass(Registry);
  initializeLowerGlobalDtorsLegacyPassPass(Registry);
  initializeLowerInvokeLegacyPassPass(Registry);
  initializeLowerSwitchLegacyPassPass(Registry);
  initializePromoteLegacyPassPass(Registry);
  initializeFixIrreduciblePass(Registry);
  initializeUnifyLoopExitsLegacyPassPass(Registry);
}
