//===-- LinkInModulesPass.h - Module Linking pass ----------------- C++ -*-===//
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
/// \file
///
/// This file provides a pass to link in Modules from a provided
/// BackendConsumer.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_BITCODE_LINKINMODULESPASS_H
#define LLVM_BITCODE_LINKINMODULESPASS_H

#include "BackendConsumer.h"
#include "toolchain/IR/PassManager.h"

namespace toolchain {
class Module;
class ModulePass;
class Pass;

/// Create and return a pass that links in Moduels from a provided
/// BackendConsumer to a given primary Module. Note that this pass is designed
/// for use with the legacy pass manager.
class LinkInModulesPass : public PassInfoMixin<LinkInModulesPass> {
  language::Core::BackendConsumer *BC;

public:
  LinkInModulesPass(language::Core::BackendConsumer *BC);

  PreservedAnalyses run(Module &M, AnalysisManager<Module> &);
  static bool isRequired() { return true; }
};

} // namespace toolchain

#endif
