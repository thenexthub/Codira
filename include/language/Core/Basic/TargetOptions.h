//===--- TargetOptions.h ----------------------------------------*- C++ -*-===//
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
///
/// \file
/// Defines the language::Core::TargetOptions class.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_TARGETOPTIONS_H
#define LANGUAGE_CORE_BASIC_TARGETOPTIONS_H

#include "language/Core/Basic/OpenCLOptions.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/Target/TargetOptions.h"
#include <string>
#include <vector>

namespace language::Core {

/// Options for controlling the target.
class TargetOptions {
public:
  /// The name of the target triple to compile for.
  std::string Triple;

  /// When compiling for the device side, contains the triple used to compile
  /// for the host.
  std::string HostTriple;

  /// If given, the name of the target CPU to generate code for.
  std::string CPU;

  /// If given, the name of the target CPU to tune code for.
  std::string TuneCPU;

  /// If given, the unit to use for floating point math.
  std::string FPMath;

  /// If given, the name of the target ABI to use.
  std::string ABI;

  /// The EABI version to use
  toolchain::EABI EABIVersion = toolchain::EABI::Default;

  /// If given, the version string of the linker in use.
  std::string LinkerVersion;

  /// The list of target specific features to enable or disable, as written on the command line.
  std::vector<std::string> FeaturesAsWritten;

  /// The list of target specific features to enable or disable -- this should
  /// be a list of strings starting with by '+' or '-'.
  std::vector<std::string> Features;

  /// The map of which features have been enabled disabled based on the command
  /// line.
  toolchain::StringMap<bool> FeatureMap;

  /// Supported OpenCL extensions and optional core features.
  toolchain::StringMap<bool> OpenCLFeaturesMap;

  /// The list of OpenCL extensions to enable or disable, as written on
  /// the command line.
  std::vector<std::string> OpenCLExtensionsAsWritten;

  /// If given, enables support for __int128_t and __uint128_t types.
  bool ForceEnableInt128 = false;

  /// \brief If enabled, use 32-bit pointers for accessing const/local/shared
  /// address space.
  bool NVPTXUseShortPointers = false;

  /// \brief Code object version for AMDGPU.
  toolchain::CodeObjectVersionKind CodeObjectVersion =
      toolchain::CodeObjectVersionKind::COV_None;

  /// \brief Enumeration values for AMDGPU printf lowering scheme
  enum class AMDGPUPrintfKind {
    /// printf lowering scheme involving hostcalls, currently used by HIP
    /// programs by default
    Hostcall = 0,

    /// printf lowering scheme involving implicit printf buffers,
    Buffered = 1,
  };

  /// \brief AMDGPU Printf lowering scheme
  AMDGPUPrintfKind AMDGPUPrintfKindVal = AMDGPUPrintfKind::Hostcall;

  // The code model to be used as specified by the user. Corresponds to
  // CodeModel::Model enum defined in include/toolchain/Support/CodeGen.h, plus
  // "default" for the case when the user has not explicitly specified a
  // code model.
  std::string CodeModel;

  // The large data threshold used for certain code models on certain
  // architectures.
  uint64_t LargeDataThreshold;

  /// The version of the SDK which was used during the compilation.
  /// The option is used for two different purposes:
  /// * on darwin the version is propagated to LLVM where it's used
  ///   to support SDK Version metadata (See D55673).
  /// * CUDA compilation uses it to control parts of CUDA compilation
  ///   in clang that depend on specific version of the CUDA SDK.
  toolchain::VersionTuple SDKVersion;

  /// The name of the darwin target- ariant triple to compile for.
  std::string DarwinTargetVariantTriple;

  /// The version of the darwin target variant SDK which was used during the
  /// compilation.
  toolchain::VersionTuple DarwinTargetVariantSDKVersion;

  /// The validator version for dxil.
  std::string DxilValidatorVersion;

  /// The entry point name for HLSL shader being compiled as specified by -E.
  std::string HLSLEntry;
};

} // end namespace language::Core

#endif
