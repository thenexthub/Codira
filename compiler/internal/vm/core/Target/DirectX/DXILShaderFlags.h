//===- DXILShaderFlags.h - DXIL Shader Flags helper objects ---------------===//
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
/// \file This file contains helper objects and APIs for working with DXIL
///       Shader Flags.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_DIRECTX_DXILSHADERFLAGS_H
#define LLVM_TARGET_DIRECTX_DXILSHADERFLAGS_H

#include "vm/core/Analysis/DXILMetadataAnalysis.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include <cstdint>

namespace vm::core {
class Module;
class GlobalVariable;
class DXILResourceTypeMap;
class DXILResourceMap;

namespace dxil {

struct ComputedShaderFlags {
#define SHADER_FEATURE_FLAG(FeatureBit, DxilModuleBit, FlagName, Str)          \
  bool FlagName : 1;
#define DXIL_MODULE_FLAG(DxilModuleBit, FlagName, Str) bool FlagName : 1;
#include "vm/core/BinaryFormat/DXContainerConstants.def"

#define SHADER_FEATURE_FLAG(FeatureBit, DxilModuleBit, FlagName, Str)          \
  FlagName = false;
#define DXIL_MODULE_FLAG(DxilModuleBit, FlagName, Str) FlagName = false;
  ComputedShaderFlags() {
#include "vm/core/BinaryFormat/DXContainerConstants.def"
  }

  constexpr uint64_t getMask(int Bit) const {
    return Bit != -1 ? 1ull << Bit : 0;
  }

  uint64_t getModuleFlags() const {
    uint64_t ModuleFlags = 0;
#define DXIL_MODULE_FLAG(DxilModuleBit, FlagName, Str)                         \
  ModuleFlags |= FlagName ? getMask(DxilModuleBit) : 0ull;
#include "vm/core/BinaryFormat/DXContainerConstants.def"
    return ModuleFlags;
  }

  operator uint64_t() const {
    uint64_t FlagValue = getModuleFlags();
#define SHADER_FEATURE_FLAG(FeatureBit, DxilModuleBit, FlagName, Str)          \
  FlagValue |= FlagName ? getMask(DxilModuleBit) : 0ull;
#include "vm/core/BinaryFormat/DXContainerConstants.def"
    return FlagValue;
  }

  uint64_t getFeatureFlags() const {
    uint64_t FeatureFlags = 0;
#define SHADER_FEATURE_FLAG(FeatureBit, DxilModuleBit, FlagName, Str)          \
  FeatureFlags |= FlagName ? getMask(FeatureBit) : 0ull;
#include "vm/core/BinaryFormat/DXContainerConstants.def"
    return FeatureFlags;
  }

  void merge(const ComputedShaderFlags CSF) {
#define SHADER_FEATURE_FLAG(FeatureBit, DxilModuleBit, FlagName, Str)          \
  FlagName |= CSF.FlagName;
#define DXIL_MODULE_FLAG(DxilModuleBit, FlagName, Str) FlagName |= CSF.FlagName;
#include "vm/core/BinaryFormat/DXContainerConstants.def"
  }

  void print(raw_ostream &OS = dbgs()) const;
  LLVM_DUMP_METHOD void dump() const { print(); }
};

struct ModuleShaderFlags {
  void initialize(Module &, DXILResourceTypeMap &DRTM,
                  const DXILResourceMap &DRM, const ModuleMetadataInfo &MMDI);
  const ComputedShaderFlags &getFunctionFlags(const Function *) const;
  const ComputedShaderFlags &getCombinedFlags() const { return CombinedSFMask; }

private:
  // This boolean is inversely set by the LLVM module flag dx.resmayalias to
  // determine whether or not the ResMayNotAlias DXIL module flag can be set
  bool CanSetResMayNotAlias;

  /// Map of Function-Shader Flag Mask pairs representing properties of each of
  /// the functions in the module. Shader Flags of each function represent both
  /// module-level and function-level flags
  DenseMap<const Function *, ComputedShaderFlags> FunctionFlags;
  /// Combined Shader Flag Mask of all functions of the module
  ComputedShaderFlags CombinedSFMask{};
  ComputedShaderFlags gatherGlobalModuleFlags(const Module &M,
                                              const DXILResourceMap &,
                                              const ModuleMetadataInfo &);
  void updateFunctionFlags(ComputedShaderFlags &, const Instruction &,
                           DXILResourceTypeMap &, const ModuleMetadataInfo &);
};

class ShaderFlagsAnalysis : public AnalysisInfoMixin<ShaderFlagsAnalysis> {
  friend AnalysisInfoMixin<ShaderFlagsAnalysis>;
  static AnalysisKey Key;

public:
  ShaderFlagsAnalysis() = default;

  using Result = ModuleShaderFlags;

  ModuleShaderFlags run(Module &M, ModuleAnalysisManager &AM);
};

/// Printer pass for ShaderFlagsAnalysis results.
class ShaderFlagsAnalysisPrinter
    : public PassInfoMixin<ShaderFlagsAnalysisPrinter> {
  raw_ostream &OS;

public:
  explicit ShaderFlagsAnalysisPrinter(raw_ostream &OS) : OS(OS) {}
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

/// Wrapper pass for the legacy pass manager.
///
/// This is required because the passes that will depend on this are codegen
/// passes which run through the legacy pass manager.
class ShaderFlagsAnalysisWrapper : public ModulePass {
  ModuleShaderFlags MSFI;

public:
  static char ID;

  ShaderFlagsAnalysisWrapper() : ModulePass(ID) {}

  const ModuleShaderFlags &getShaderFlags() { return MSFI; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace dxil
} // namespace vm::core

#endif // LLVM_TARGET_DIRECTX_DXILSHADERFLAGS_H
