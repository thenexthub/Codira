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

#include "language/Basic/LLVM.h"
#include "language/Basic/LLVMInitialize.h"
#include "language/SILOptimizer/PassManager/PassPipeline.h"
#include "llvm/Support/CommandLine.h"

using namespace language;

struct SILPassPipelineDumperOptions {
  llvm::cl::opt<PassPipelineKind>
    PipelineKind = llvm::cl::opt<PassPipelineKind>(llvm::cl::desc("<pipeline kind>"),
                                                   llvm::cl::values(
#define PASSPIPELINE(NAME, DESCRIPTION)                                        \
  clEnumValN(PassPipelineKind::NAME, #NAME, DESCRIPTION),
#include "language/SILOptimizer/PassManager/PassPipeline.def"
                                                        clEnumValN(0, "", "")));
};

namespace llvm {
llvm::raw_ostream &operator<<(llvm::raw_ostream &os, PassPipelineKind Kind) {
  switch (Kind) {
#define PASSPIPELINE(NAME, DESCRIPTION)                                        \
  case PassPipelineKind::NAME:                                                 \
    return os << #NAME;
#include "language/SILOptimizer/PassManager/PassPipeline.def"
  }
  llvm_unreachable("Unhandled PassPipelineKind in switch");
}
} // namespace llvm

int sil_passpipeline_dumper_main(ArrayRef<const char *> argv, void *MainAddr) {
  INITIALIZE_LLVM();

  SILPassPipelineDumperOptions options;

  llvm::cl::ParseCommandLineOptions(argv.size(), argv.data(),
                                    "Swift SIL Pass Pipeline Dumper\n");

  // TODO: add options to manipulate this.
  SILOptions Opt;

  switch (options.PipelineKind) {
#define PASSPIPELINE(NAME, DESCRIPTION)                                        \
  case PassPipelineKind::NAME: {                                               \
    SILPassPipelinePlan::get##NAME##PassPipeline(Opt).print(llvm::outs());     \
    break;                                                                     \
  }
#include "language/SILOptimizer/PassManager/PassPipeline.def"
  }

  llvm::outs() << '\n';

  return 0;
}
