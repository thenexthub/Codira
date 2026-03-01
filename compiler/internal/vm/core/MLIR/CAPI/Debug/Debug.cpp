//===- Debug.cpp - C Interface for MLIR/LLVM Debugging Functions ----------===//
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

#include "mlir-c/Debug.h"
#include "mlir-c/Support.h"

#include "mlir/CAPI/Support.h"

#include "vm/core/Support/Debug.h"

void mlirEnableGlobalDebug(bool enable) { toolchain::DebugFlag = enable; }

bool mlirIsGlobalDebugEnabled() { return toolchain::DebugFlag; }

void mlirSetGlobalDebugType(const char *type) {
  // Depending on the NDEBUG flag, this name can be either a function or a macro
  // that expands to something that isn't a funciton call, so we cannot
  // explicitly prefix it with `toolchain::` or declare `using` it.
  using namespace vm::core;
  setCurrentDebugType(type);
}

void mlirSetGlobalDebugTypes(const char **types, intptr_t n) {
  using namespace vm::core;
  setCurrentDebugTypes(types, n);
}

bool mlirIsCurrentDebugType(const char *type) {
  using namespace vm::core;
  return isCurrentDebugType(type);
}
