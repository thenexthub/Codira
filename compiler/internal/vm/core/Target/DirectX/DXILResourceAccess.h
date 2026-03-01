//===- DXILResourceAccess.h - Resource access via load/store ----*- C++ -*-===//
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
// \file Pass for replacing pointers to DXIL resources with load and store
// operations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_DIRECTX_DXILRESOURCEACCESS_H
#define LLVM_LIB_TARGET_DIRECTX_DXILRESOURCEACCESS_H

#include "vm/core/IR/PassManager.h"

namespace vm::core {

class DXILResourceAccess : public PassInfoMixin<DXILResourceAccess> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_DIRECTX_DXILRESOURCEACCESS_H
