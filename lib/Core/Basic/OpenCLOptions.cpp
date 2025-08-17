//===--- OpenCLOptions.cpp---------------------------------------*- C++ -*-===//
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

#include "language/Core/Basic/OpenCLOptions.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/TargetInfo.h"

namespace language::Core {

// First feature in a pair requires the second one to be supported.
static const std::pair<StringRef, StringRef> DependentFeaturesList[] = {
    {"__opencl_c_read_write_images", "__opencl_c_images"},
    {"__opencl_c_3d_image_writes", "__opencl_c_images"},
    {"__opencl_c_pipes", "__opencl_c_generic_address_space"},
    {"__opencl_c_device_enqueue", "__opencl_c_generic_address_space"},
    {"__opencl_c_device_enqueue", "__opencl_c_program_scope_global_variables"}};

// Extensions and equivalent feature pairs.
static const std::pair<StringRef, StringRef> FeatureExtensionMap[] = {
    {"cl_khr_fp64", "__opencl_c_fp64"},
    {"cl_khr_3d_image_writes", "__opencl_c_3d_image_writes"}};

bool OpenCLOptions::isKnown(toolchain::StringRef Ext) const {
  return OptMap.contains(Ext);
}

bool OpenCLOptions::isAvailableOption(toolchain::StringRef Ext,
                                      const LangOptions &LO) const {
  if (!isKnown(Ext))
    return false;

  auto &OptInfo = OptMap.find(Ext)->getValue();
  if (OptInfo.isCoreIn(LO) || OptInfo.isOptionalCoreIn(LO))
    return isSupported(Ext, LO);

  return isEnabled(Ext);
}

bool OpenCLOptions::isEnabled(toolchain::StringRef Ext) const {
  auto I = OptMap.find(Ext);
  return I != OptMap.end() && I->getValue().Enabled;
}

bool OpenCLOptions::isWithPragma(toolchain::StringRef Ext) const {
  auto E = OptMap.find(Ext);
  return E != OptMap.end() && E->second.WithPragma;
}

bool OpenCLOptions::isSupported(toolchain::StringRef Ext,
                                const LangOptions &LO) const {
  auto I = OptMap.find(Ext);
  return I != OptMap.end() && I->getValue().Supported &&
         I->getValue().isAvailableIn(LO);
}

bool OpenCLOptions::isSupportedCore(toolchain::StringRef Ext,
                                    const LangOptions &LO) const {
  auto I = OptMap.find(Ext);
  return I != OptMap.end() && I->getValue().Supported &&
         I->getValue().isCoreIn(LO);
}

bool OpenCLOptions::isSupportedOptionalCore(toolchain::StringRef Ext,
                                            const LangOptions &LO) const {
  auto I = OptMap.find(Ext);
  return I != OptMap.end() && I->getValue().Supported &&
         I->getValue().isOptionalCoreIn(LO);
}

bool OpenCLOptions::isSupportedCoreOrOptionalCore(toolchain::StringRef Ext,
                                                  const LangOptions &LO) const {
  return isSupportedCore(Ext, LO) || isSupportedOptionalCore(Ext, LO);
}

bool OpenCLOptions::isSupportedExtension(toolchain::StringRef Ext,
                                         const LangOptions &LO) const {
  auto I = OptMap.find(Ext);
  return I != OptMap.end() && I->getValue().Supported &&
         I->getValue().isAvailableIn(LO) &&
         !isSupportedCoreOrOptionalCore(Ext, LO);
}

void OpenCLOptions::enable(toolchain::StringRef Ext, bool V) {
  OptMap[Ext].Enabled = V;
}

void OpenCLOptions::acceptsPragma(toolchain::StringRef Ext, bool V) {
  OptMap[Ext].WithPragma = V;
}

void OpenCLOptions::support(toolchain::StringRef Ext, bool V) {
  assert(!Ext.empty() && "Extension is empty.");
  assert(Ext[0] != '+' && Ext[0] != '-');
  OptMap[Ext].Supported = V;
}

OpenCLOptions::OpenCLOptions() {
#define OPENCL_GENERIC_EXTENSION(Ext, ...)                                     \
  OptMap.insert_or_assign(#Ext, OpenCLOptionInfo{__VA_ARGS__});
#include "language/Core/Basic/OpenCLExtensions.def"
}

void OpenCLOptions::addSupport(const toolchain::StringMap<bool> &FeaturesMap,
                               const LangOptions &Opts) {
  for (const auto &F : FeaturesMap) {
    const auto &Name = F.getKey();
    if (F.getValue() && isKnown(Name) && OptMap[Name].isAvailableIn(Opts))
      support(Name);
  }
}

void OpenCLOptions::disableAll() {
  for (auto &Opt : OptMap)
    Opt.getValue().Enabled = false;
}

bool OpenCLOptions::diagnoseUnsupportedFeatureDependencies(
    const TargetInfo &TI, DiagnosticsEngine &Diags) {
  auto OpenCLFeaturesMap = TI.getSupportedOpenCLOpts();

  bool IsValid = true;
  for (auto &FeaturePair : DependentFeaturesList) {
    auto Feature = FeaturePair.first;
    auto Dep = FeaturePair.second;
    if (TI.hasFeatureEnabled(OpenCLFeaturesMap, Feature) &&
        !TI.hasFeatureEnabled(OpenCLFeaturesMap, Dep)) {
      IsValid = false;
      Diags.Report(diag::err_opencl_feature_requires) << Feature << Dep;
    }
  }
  return IsValid;
}

bool OpenCLOptions::diagnoseFeatureExtensionDifferences(
    const TargetInfo &TI, DiagnosticsEngine &Diags) {
  auto OpenCLFeaturesMap = TI.getSupportedOpenCLOpts();

  bool IsValid = true;
  for (auto &ExtAndFeat : FeatureExtensionMap)
    if (TI.hasFeatureEnabled(OpenCLFeaturesMap, ExtAndFeat.first) !=
        TI.hasFeatureEnabled(OpenCLFeaturesMap, ExtAndFeat.second)) {
      IsValid = false;
      Diags.Report(diag::err_opencl_extension_and_feature_differs)
          << ExtAndFeat.first << ExtAndFeat.second;
    }
  return IsValid;
}

} // end namespace language::Core
