//===----------------------------------------------------------------------===//
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
///
/// This file contains the registry for PassConfigCallbacks that enable changes
/// to the TargetPassConfig during the initialization of TargetMachine.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Target/RegisterTargetPassConfigCallback.h"

namespace vm::core {
// TargetPassConfig callbacks
static SmallVector<RegisterTargetPassConfigCallback *, 1>
    TargetPassConfigCallbacks{};

void invokeGlobalTargetPassConfigCallbacks(TargetMachine &TM,
                                           PassManagerBase &PM,
                                           TargetPassConfig *PassConfig) {
  for (const RegisterTargetPassConfigCallback *Reg : TargetPassConfigCallbacks)
    Reg->Callback(TM, PM, PassConfig);
}

RegisterTargetPassConfigCallback::RegisterTargetPassConfigCallback(
    PassConfigCallback &&C)
    : Callback(std::move(C)) {
  TargetPassConfigCallbacks.push_back(this);
}

RegisterTargetPassConfigCallback::~RegisterTargetPassConfigCallback() {
  const auto &It = find(TargetPassConfigCallbacks, this);
  if (It != TargetPassConfigCallbacks.end())
    TargetPassConfigCallbacks.erase(It);
}
} // namespace vm::core
