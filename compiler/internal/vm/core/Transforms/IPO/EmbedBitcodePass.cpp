//===- EmbedBitcodePass.cpp - Pass that embeds the bitcode into a global---===//
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

#include "vm/core/Transforms/IPO/EmbedBitcodePass.h"
#include "vm/core/Bitcode/BitcodeWriter.h"
#include "vm/core/Bitcode/BitcodeWriterPass.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/MemoryBufferRef.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"
#include "vm/core/Transforms/IPO/ThinLTOBitcodeWriter.h"
#include "vm/core/Transforms/Utils/ModuleUtils.h"

#include <string>

using namespace vm::core;

PreservedAnalyses EmbedBitcodePass::run(Module &M, ModuleAnalysisManager &AM) {
  if (M.getGlobalVariable("toolchain.embedded.module", /*AllowInternal=*/true))
    reportFatalUsageError("Can only embed the module once");

  Triple T(M.getTargetTriple());
  if (T.getObjectFormat() != Triple::ELF && T.getObjectFormat() != Triple::COFF)
    reportFatalUsageError("EmbedBitcode pass currently only supports COFF and "
                          "ELF object formats");

  std::string Data;
  raw_string_ostream OS(Data);
  if (IsThinLTO)
    ThinLTOBitcodeWriterPass(OS, /*ThinLinkOS=*/nullptr).run(M, AM);
  else
    BitcodeWriterPass(OS, /*ShouldPreserveUseListOrder=*/false, EmitLTOSummary)
        .run(M, AM);

  embedBufferInModule(M, MemoryBufferRef(Data, "ModuleData"), ".toolchain.lto");

  return PreservedAnalyses::none();
}
