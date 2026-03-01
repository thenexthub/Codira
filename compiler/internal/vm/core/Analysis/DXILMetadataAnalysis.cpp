//=- DXILMetadataAnalysis.cpp - Representation of Module metadata -*- C++ -*=//
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

#include "vm/core/Analysis/DXILMetadataAnalysis.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/IR/Module.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/ErrorHandling.h"

#define DEBUG_TYPE "dxil-metadata-analysis"

using namespace vm::core;
using namespace dxil;

static ModuleMetadataInfo collectMetadataInfo(Module &M) {
  ModuleMetadataInfo MMDAI;
  const Triple &TT = M.getTargetTriple();
  MMDAI.DXILVersion = TT.getDXILVersion();
  MMDAI.ShaderModelVersion = TT.getOSVersion();
  MMDAI.ShaderProfile = TT.getEnvironment();
  NamedMDNode *ValidatorVerNode = M.getNamedMetadata("dx.valver");
  if (ValidatorVerNode) {
    auto *ValVerMD = cast<MDNode>(ValidatorVerNode->getOperand(0));
    auto *MajorMD = mdconst::extract<ConstantInt>(ValVerMD->getOperand(0));
    auto *MinorMD = mdconst::extract<ConstantInt>(ValVerMD->getOperand(1));
    MMDAI.ValidatorVersion =
        VersionTuple(MajorMD->getZExtValue(), MinorMD->getZExtValue());
  }

  // For all HLSL Shader functions
  for (auto &F : M.functions()) {
    if (!F.hasFnAttribute("hlsl.shader"))
      continue;

    EntryProperties EFP(&F);
    // Get "hlsl.shader" attribute
    Attribute EntryAttr = F.getFnAttribute("hlsl.shader");
    assert(EntryAttr.isValid() &&
           "Invalid value specified for HLSL function attribute hlsl.shader");
    StringRef EntryProfile = EntryAttr.getValueAsString();
    Triple T("", "", "", EntryProfile);
    EFP.ShaderStage = T.getEnvironment();
    // Get numthreads attribute value, if one exists
    StringRef NumThreadsStr =
        F.getFnAttribute("hlsl.numthreads").getValueAsString();
    if (!NumThreadsStr.empty()) {
      SmallVector<StringRef> NumThreadsVec;
      NumThreadsStr.split(NumThreadsVec, ',');
      assert(NumThreadsVec.size() == 3 && "Invalid numthreads specified");
      // Read in the three component values of numthreads
      [[maybe_unused]] bool Success =
          toolchain::to_integer(NumThreadsVec[0], EFP.NumThreadsX, 10);
      assert(Success && "Failed to parse X component of numthreads");
      Success = toolchain::to_integer(NumThreadsVec[1], EFP.NumThreadsY, 10);
      assert(Success && "Failed to parse Y component of numthreads");
      Success = toolchain::to_integer(NumThreadsVec[2], EFP.NumThreadsZ, 10);
      assert(Success && "Failed to parse Z component of numthreads");
    }
    // Get wavesize attribute value, if one exists
    StringRef WaveSizeStr =
        F.getFnAttribute("hlsl.wavesize").getValueAsString();
    if (!WaveSizeStr.empty()) {
      SmallVector<StringRef> WaveSizeVec;
      WaveSizeStr.split(WaveSizeVec, ',');
      assert(WaveSizeVec.size() == 3 && "Invalid wavesize specified");
      // Read in the three component values of numthreads
      [[maybe_unused]] bool Success =
          toolchain::to_integer(WaveSizeVec[0], EFP.WaveSizeMin, 10);
      assert(Success && "Failed to parse Min component of wavesize");
      Success = toolchain::to_integer(WaveSizeVec[1], EFP.WaveSizeMax, 10);
      assert(Success && "Failed to parse Max component of wavesize");
      Success = toolchain::to_integer(WaveSizeVec[2], EFP.WaveSizePref, 10);
      assert(Success && "Failed to parse Preferred component of wavesize");
    }
    MMDAI.EntryPropertyVec.push_back(EFP);
  }
  return MMDAI;
}

void ModuleMetadataInfo::print(raw_ostream &OS) const {
  OS << "Shader Model Version : " << ShaderModelVersion.getAsString() << "\n";
  OS << "DXIL Version : " << DXILVersion.getAsString() << "\n";
  OS << "Target Shader Stage : "
     << Triple::getEnvironmentTypeName(ShaderProfile) << "\n";
  OS << "Validator Version : " << ValidatorVersion.getAsString() << "\n";
  for (const auto &EP : EntryPropertyVec) {
    OS << " " << EP.Entry->getName() << "\n";
    OS << "  Function Shader Stage : "
       << Triple::getEnvironmentTypeName(EP.ShaderStage) << "\n";
    OS << "  NumThreads: " << EP.NumThreadsX << "," << EP.NumThreadsY << ","
       << EP.NumThreadsZ << "\n";
  }
}

//===----------------------------------------------------------------------===//
// DXILMetadataAnalysis and DXILMetadataAnalysisPrinterPass

// Provide an explicit template instantiation for the static ID.
AnalysisKey DXILMetadataAnalysis::Key;

toolchain::dxil::ModuleMetadataInfo
DXILMetadataAnalysis::run(Module &M, ModuleAnalysisManager &AM) {
  return collectMetadataInfo(M);
}

PreservedAnalyses
DXILMetadataAnalysisPrinterPass::run(Module &M, ModuleAnalysisManager &AM) {
  toolchain::dxil::ModuleMetadataInfo &Data = AM.getResult<DXILMetadataAnalysis>(M);

  Data.print(OS);
  return PreservedAnalyses::all();
}

//===----------------------------------------------------------------------===//
// DXILMetadataAnalysisWrapperPass

DXILMetadataAnalysisWrapperPass::DXILMetadataAnalysisWrapperPass()
    : ModulePass(ID) {}

DXILMetadataAnalysisWrapperPass::~DXILMetadataAnalysisWrapperPass() = default;

void DXILMetadataAnalysisWrapperPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.setPreservesAll();
}

bool DXILMetadataAnalysisWrapperPass::runOnModule(Module &M) {
  MetadataInfo.reset(new ModuleMetadataInfo(collectMetadataInfo(M)));
  return false;
}

void DXILMetadataAnalysisWrapperPass::releaseMemory() { MetadataInfo.reset(); }

void DXILMetadataAnalysisWrapperPass::print(raw_ostream &OS,
                                            const Module *) const {
  if (!MetadataInfo) {
    OS << "No module metadata info has been built!\n";
    return;
  }
  MetadataInfo->print(dbgs());
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD
void DXILMetadataAnalysisWrapperPass::dump() const { print(dbgs(), nullptr); }
#endif

INITIALIZE_PASS(DXILMetadataAnalysisWrapperPass, "dxil-metadata-analysis",
                "DXIL Module Metadata analysis", false, true)
char DXILMetadataAnalysisWrapperPass::ID = 0;
