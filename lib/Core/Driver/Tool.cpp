//===--- Tool.cpp - Compilation Tools -------------------------------------===//
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

#include "language/Core/Driver/Tool.h"
#include "language/Core/Driver/InputInfo.h"

using namespace language::Core::driver;

Tool::Tool(const char *_Name, const char *_ShortName, const ToolChain &TC)
    : Name(_Name), ShortName(_ShortName), TheToolChain(TC) {}

Tool::~Tool() {
}

void Tool::ConstructJobMultipleOutputs(Compilation &C, const JobAction &JA,
                                       const InputInfoList &Outputs,
                                       const InputInfoList &Inputs,
                                       const toolchain::opt::ArgList &TCArgs,
                                       const char *LinkingOutput) const {
  assert(Outputs.size() == 1 && "Expected only one output by default!");
  ConstructJob(C, JA, Outputs.front(), Inputs, TCArgs, LinkingOutput);
}
