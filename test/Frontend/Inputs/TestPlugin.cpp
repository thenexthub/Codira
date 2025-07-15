//===---------------------- TestPlugin.cpp --------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

// Used by -load-pass-plugin

#include "toolchain/Pass.h"
#include "toolchain/Passes/PassBuilder.h"
#include "toolchain/Passes/PassPlugin.h"

using namespace toolchain;

namespace {

void runTestPlugin(Function &F) {
  errs() << "TestPlugin: ";
  errs().write_escaped(F.getName()) << '\n';
}

struct TestPluginPass : PassInfoMixin<TestPluginPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
    runTestPlugin(F);
    return PreservedAnalyses::all();
  }
};

} // namespace

PassPluginLibraryInfo getTestPluginInfo() {
  return {TOOLCHAIN_PLUGIN_API_VERSION, "TestPlugin", TOOLCHAIN_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerVectorizerStartEPCallback(
                [](toolchain::FunctionPassManager &PM, OptimizationLevel Level) {
                  PM.addPass(TestPluginPass());
            });
          }};
}

extern "C" TOOLCHAIN_ATTRIBUTE_WEAK ::toolchain::PassPluginLibraryInfo
toolchainGetPassPluginInfo() {
  return getTestPluginInfo();
}
