//===--- sil_passpipeline_dumper_main.cpp ---------------------------------===//
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
///
/// \file
///
/// This is a simple tool that dumps out a yaml description of one of the
/// current list of pass pipelines. Meant to be used to script on top of
/// sil-opt.
///
//===----------------------------------------------------------------------===//

#include "language/Basic/Toolchain.h"
#include "language/Basic/ToolchainInitializer.h"
#include "language/SILOptimizer/PassManager/PassPipeline.h"
#include "toolchain/Support/CommandLine.h"

using namespace language;

struct SILPassPipelineDumperOptions {
  toolchain::cl::opt<PassPipelineKind>
    PipelineKind = toolchain::cl::opt<PassPipelineKind>(toolchain::cl::desc("<pipeline kind>"),
                                                   toolchain::cl::values(
#define PASSPIPELINE(NAME, DESCRIPTION)                                        \
  clEnumValN(PassPipelineKind::NAME, #NAME, DESCRIPTION),
#include "language/SILOptimizer/PassManager/PassPipeline.def"
                                                        clEnumValN(0, "", "")));
};

namespace toolchain {
toolchain::raw_ostream &operator<<(toolchain::raw_ostream &os, PassPipelineKind Kind) {
  switch (Kind) {
#define PASSPIPELINE(NAME, DESCRIPTION)                                        \
  case PassPipelineKind::NAME:                                                 \
    return os << #NAME;
#include "language/SILOptimizer/PassManager/PassPipeline.def"
  }
  toolchain_unreachable("Unhandled PassPipelineKind in switch");
}
} // namespace toolchain

int sil_passpipeline_dumper_main(ArrayRef<const char *> argv, void *MainAddr) {
  INITIALIZE_LLVM();

  SILPassPipelineDumperOptions options;

  toolchain::cl::ParseCommandLineOptions(argv.size(), argv.data(),
                                    "Codira SIL Pass Pipeline Dumper\n");

  // TODO: add options to manipulate this.
  SILOptions Opt;

  switch (options.PipelineKind) {
#define PASSPIPELINE(NAME, DESCRIPTION)                                        \
  case PassPipelineKind::NAME: {                                               \
    SILPassPipelinePlan::get##NAME##PassPipeline(Opt).print(toolchain::outs());     \
    break;                                                                     \
  }
#include "language/SILOptimizer/PassManager/PassPipeline.def"
  }

  toolchain::outs() << '\n';

  return 0;
}
