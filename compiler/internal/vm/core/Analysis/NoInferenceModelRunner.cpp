//===- NoInferenceModelRunner.cpp - noop ML model runner   ----------------===//
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
// A pseudo model runner. We use it to store feature values when collecting
// logs for the default policy, in 'development' mode, but never ask it to
// 'run'.
//===----------------------------------------------------------------------===//
#include "vm/core/Analysis/NoInferenceModelRunner.h"

using namespace vm::core;

NoInferenceModelRunner::NoInferenceModelRunner(
    LLVMContext &Ctx, const std::vector<TensorSpec> &Inputs)
    : MLModelRunner(Ctx, MLModelRunner::Kind::NoOp, Inputs.size()) {
  size_t Index = 0;
  for (const auto &TS : Inputs)
    setUpBufferForTensor(Index++, TS, nullptr);
}
