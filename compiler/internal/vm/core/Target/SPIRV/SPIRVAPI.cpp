//===-- SPIRVAPI.cpp - SPIR-V Backend API ---------------------*- C++ -*---===//
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

#include "SPIRVCommandLine.h"
#include "SPIRVSubtarget.h"
#include "SPIRVTargetMachine.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/CodeGen/CommandFlags.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/LegacyPassManager.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/Verifier.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/TargetSelect.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/TargetParser/SubtargetFeature.h"
#include "vm/core/TargetParser/Triple.h"
#include <optional>
#include <string>
#include <vector>

using namespace vm::core;

namespace {

std::once_flag InitOnceFlag;
void InitializeSPIRVTarget() {
  std::call_once(InitOnceFlag, []() {
    LLVMInitializeSPIRVTargetInfo();
    LLVMInitializeSPIRVTarget();
    LLVMInitializeSPIRVTargetMC();
    LLVMInitializeSPIRVAsmPrinter();
  });
}
} // namespace

namespace vm::core {

// The goal of this function is to facilitate integration of SPIRV Backend into
// tools and libraries by means of exposing an API call that translate LLVM
// module to SPIR-V and write results into a string as binary SPIR-V output,
// providing diagnostics on fail and means of configuring translation.
extern "C" LLVM_EXTERNAL_VISIBILITY bool
SPIRVTranslate(Module *M, std::string &SpirvObj, std::string &ErrMsg,
               const std::vector<std::string> &AllowExtNames,
               toolchain::CodeGenOptLevel OLevel, Triple TargetTriple) {
  // Fallbacks for option values.
  static const std::string DefaultTriple = "spirv64-unknown-unknown";
  static const std::string DefaultMArch = "";

  std::set<SPIRV::Extension::Extension> AllowedExtIds;
  StringRef UnknownExt =
      SPIRVExtensionsParser::checkExtensions(AllowExtNames, AllowedExtIds);
  if (!UnknownExt.empty()) {
    ErrMsg = "Unknown SPIR-V extension: " + UnknownExt.str();
    return false;
  }

  // SPIR-V-specific target initialization.
  InitializeSPIRVTarget();

  if (TargetTriple.getTriple().empty()) {
    TargetTriple.setTriple(DefaultTriple);
    M->setTargetTriple(TargetTriple);
  }
  const Target *TheTarget =
      TargetRegistry::lookupTarget(DefaultMArch, TargetTriple, ErrMsg);
  if (!TheTarget)
    return false;

  // A call to codegen::InitTargetOptionsFromCodeGenFlags(TargetTriple)
  // hits the following assertion: toolchain/lib/CodeGen/CommandFlags.cpp:78:
  // toolchain::FPOpFusion::FPOpFusionMode toolchain::codegen::getFuseFPOps(): Assertion
  // `FuseFPOpsView && "RegisterCodeGenFlags not created."' failed.
  TargetOptions Options;
  std::optional<Reloc::Model> RM;
  std::optional<CodeModel::Model> CM;
  std::unique_ptr<TargetMachine> Target(TheTarget->createTargetMachine(
      TargetTriple, "", "", Options, RM, CM, OLevel));
  if (!Target) {
    ErrMsg = "Could not allocate target machine!";
    return false;
  }

  // Set available extensions.
  SPIRVTargetMachine *STM = static_cast<SPIRVTargetMachine *>(Target.get());
  const_cast<SPIRVSubtarget *>(STM->getSubtargetImpl())
      ->initAvailableExtensions(AllowedExtIds);

  if (M->getCodeModel())
    Target->setCodeModel(*M->getCodeModel());

  std::string DLStr = M->getDataLayoutStr();
  Expected<DataLayout> MaybeDL = DataLayout::parse(
      DLStr.empty() ? Target->createDataLayout().getStringRepresentation()
                    : DLStr);
  if (!MaybeDL) {
    ErrMsg = toString(MaybeDL.takeError());
    return false;
  }
  M->setDataLayout(MaybeDL.get());

  TargetLibraryInfoImpl TLII(M->getTargetTriple());
  legacy::PassManager PM;
  PM.add(new TargetLibraryInfoWrapperPass(TLII));
  std::unique_ptr<MachineModuleInfoWrapperPass> MMIWP(
      new MachineModuleInfoWrapperPass(Target.get()));
  Target->getObjFileLowering()->Initialize(MMIWP->getMMI().getContext(),
                                           *Target);

  SmallString<4096> OutBuffer;
  raw_svector_ostream OutStream(OutBuffer);
  if (Target->addPassesToEmitFile(PM, OutStream, nullptr,
                                  CodeGenFileType::ObjectFile)) {
    ErrMsg = "Target machine cannot emit a file of this type";
    return false;
  }

  PM.run(*M);
  SpirvObj = OutBuffer.str();

  return true;
}

// TODO: Remove this wrapper after existing clients switch into a newer
// implementation of SPIRVTranslate().
extern "C" LLVM_EXTERNAL_VISIBILITY bool
SPIRVTranslateModule(Module *M, std::string &SpirvObj, std::string &ErrMsg,
                     const std::vector<std::string> &AllowExtNames,
                     const std::vector<std::string> &Opts) {
  // optional: Opts[0] is a string representation of Triple,
  // take Module triple otherwise
  Triple TargetTriple = Opts.empty() || Opts[0].empty()
                            ? M->getTargetTriple()
                            : Triple(Triple::normalize(Opts[0]));
  // optional: Opts[1] is a string representation of CodeGenOptLevel,
  // no optimization otherwise
  toolchain::CodeGenOptLevel OLevel = CodeGenOptLevel::None;
  if (Opts.size() > 1 && !Opts[1].empty()) {
    if (auto Level = CodeGenOpt::parseLevel(Opts[1][0])) {
      OLevel = *Level;
    } else {
      ErrMsg = "Invalid optimization level!";
      return false;
    }
  }
  return SPIRVTranslate(M, SpirvObj, ErrMsg, AllowExtNames, OLevel,
                        std::move(TargetTriple));
}

} // namespace vm::core
