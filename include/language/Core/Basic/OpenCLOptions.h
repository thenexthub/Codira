//===--- OpenCLOptions.h ----------------------------------------*- C++ -*-===//
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
/// Defines the language::Core::OpenCLOptions class.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_OPENCLOPTIONS_H
#define LANGUAGE_CORE_BASIC_OPENCLOPTIONS_H

#include "language/Core/Basic/LangOptions.h"
#include "toolchain/ADT/StringMap.h"

namespace language::Core {

class DiagnosticsEngine;
class TargetInfo;

namespace {
// This enum maps OpenCL version(s) into value. These values are used as
// a mask to indicate in which OpenCL version(s) extension is a core or
// optional core feature.
enum OpenCLVersionID : unsigned int {
  OCL_C_10 = 0x1,
  OCL_C_11 = 0x2,
  OCL_C_12 = 0x4,
  OCL_C_20 = 0x8,
  OCL_C_30 = 0x10,
  OCL_C_ALL = 0x1f,
  OCL_C_11P = OCL_C_ALL ^ OCL_C_10,              // OpenCL C 1.1+
  OCL_C_12P = OCL_C_ALL ^ (OCL_C_10 | OCL_C_11), // OpenCL C 1.2+
};

static inline OpenCLVersionID encodeOpenCLVersion(unsigned OpenCLVersion) {
  switch (OpenCLVersion) {
  default:
    toolchain_unreachable("Unknown OpenCL version code");
  case 100:
    return OCL_C_10;
  case 110:
    return OCL_C_11;
  case 120:
    return OCL_C_12;
  case 200:
    return OCL_C_20;
  case 300:
    return OCL_C_30;
  }
}

// Check if OpenCL C version is contained in a given encoded OpenCL C version
// mask.
static inline bool isOpenCLVersionContainedInMask(const LangOptions &LO,
                                                  unsigned Mask) {
  auto CLVer = LO.getOpenCLCompatibleVersion();
  OpenCLVersionID Code = encodeOpenCLVersion(CLVer);
  return Mask & Code;
}

} // end anonymous namespace

/// OpenCL supported extensions and optional core features
class OpenCLOptions {

public:
  // OpenCL C v1.2 s6.5 - All program scope variables must be declared in the
  // __constant address space.
  // OpenCL C v2.0 s6.5.1 - Variables defined at program scope and static
  // variables inside a function can also be declared in the global
  // address space.
  // OpenCL C v3.0 s6.7.1 - Variables at program scope or static or extern
  // variables inside functions can be declared in global address space if
  // the __opencl_c_program_scope_global_variables feature is supported
  // C++ for OpenCL inherits rule from OpenCL C v2.0.
  bool areProgramScopeVariablesSupported(const LangOptions &Opts) const {
    return Opts.getOpenCLCompatibleVersion() == 200 ||
           (Opts.getOpenCLCompatibleVersion() == 300 &&
            isSupported("__opencl_c_program_scope_global_variables", Opts));
  }

  struct OpenCLOptionInfo {
    // Does this option have pragma.
    bool WithPragma = false;

    // Option starts to be available in this OpenCL version
    unsigned Avail = 100U;

    // Option becomes core feature in this OpenCL versions
    unsigned Core = 0U;

    // Option becomes optional core feature in this OpenCL versions
    unsigned Opt = 0U;

    // Is this option supported
    bool Supported = false;

    // Is this option enabled
    bool Enabled = false;

    OpenCLOptionInfo() = default;
    OpenCLOptionInfo(bool Pragma, unsigned AvailV, unsigned CoreV,
                     unsigned OptV)
        : WithPragma(Pragma), Avail(AvailV), Core(CoreV), Opt(OptV) {}

    bool isCore() const { return Core != 0U; }

    bool isOptionalCore() const { return Opt != 0U; }

    // Is option available in OpenCL version \p LO.
    bool isAvailableIn(const LangOptions &LO) const {
      // In C++ mode all extensions should work at least as in v2.0.
      return LO.getOpenCLCompatibleVersion() >= Avail;
    }

    // Is core option in OpenCL version \p LO.
    bool isCoreIn(const LangOptions &LO) const {
      return isAvailableIn(LO) && isOpenCLVersionContainedInMask(LO, Core);
    }

    // Is optional core option in OpenCL version \p LO.
    bool isOptionalCoreIn(const LangOptions &LO) const {
      return isAvailableIn(LO) && isOpenCLVersionContainedInMask(LO, Opt);
    }
  };

  bool isKnown(toolchain::StringRef Ext) const;

  // For core or optional core feature check that it is supported
  // by a target, for any other option (extension) check that it is
  // enabled via pragma
  bool isAvailableOption(toolchain::StringRef Ext, const LangOptions &LO) const;

  bool isWithPragma(toolchain::StringRef Ext) const;

  // Is supported as either an extension or an (optional) core feature for
  // OpenCL version \p LO.
  bool isSupported(toolchain::StringRef Ext, const LangOptions &LO) const;

  // Is supported OpenCL core feature for OpenCL version \p LO.
  // For supported extension, return false.
  bool isSupportedCore(toolchain::StringRef Ext, const LangOptions &LO) const;

  // Is supported optional core OpenCL feature for OpenCL version \p LO.
  // For supported extension, return false.
  bool isSupportedOptionalCore(toolchain::StringRef Ext,
                               const LangOptions &LO) const;

  // Is supported optional core or core OpenCL feature for OpenCL version \p
  // LO. For supported extension, return false.
  bool isSupportedCoreOrOptionalCore(toolchain::StringRef Ext,
                                     const LangOptions &LO) const;

  // Is supported OpenCL extension for OpenCL version \p LO.
  // For supported core or optional core feature, return false.
  bool isSupportedExtension(toolchain::StringRef Ext, const LangOptions &LO) const;

  // FIXME: Whether extension should accept pragma should not
  // be reset dynamically. But it currently required when
  // registering new extensions via pragmas.
  void acceptsPragma(toolchain::StringRef Ext, bool V = true);

  void enable(toolchain::StringRef Ext, bool V = true);

  /// Enable or disable support for OpenCL extensions
  /// \param Ext name of the extension (not prefixed with '+' or '-')
  /// \param V value to set for a extension
  void support(toolchain::StringRef Ext, bool V = true);

  OpenCLOptions();

  // Set supported options based on target settings and language version
  void addSupport(const toolchain::StringMap<bool> &FeaturesMap,
                  const LangOptions &Opts);

  // Disable all extensions
  void disableAll();

  friend class ASTWriter;
  friend class ASTReader;

  using OpenCLOptionInfoMap = toolchain::StringMap<OpenCLOptionInfo>;

  template <typename... Args>
  static bool isOpenCLOptionCoreIn(const LangOptions &LO, Args &&... args) {
    return OpenCLOptionInfo(std::forward<Args>(args)...).isCoreIn(LO);
  }

  template <typename... Args>
  static bool isOpenCLOptionAvailableIn(const LangOptions &LO,
                                        Args &&... args) {
    return OpenCLOptionInfo(std::forward<Args>(args)...).isAvailableIn(LO);
  }

  // Diagnose feature dependencies for OpenCL C 3.0. Return false if target
  // doesn't follow these requirements.
  static bool diagnoseUnsupportedFeatureDependencies(const TargetInfo &TI,
                                                     DiagnosticsEngine &Diags);

  // Diagnose that features and equivalent extension are set to same values.
  // Return false if target doesn't follow these requirements.
  static bool diagnoseFeatureExtensionDifferences(const TargetInfo &TI,
                                                  DiagnosticsEngine &Diags);

private:
  // Option is enabled via pragma
  bool isEnabled(toolchain::StringRef Ext) const;

  OpenCLOptionInfoMap OptMap;
};

} // end namespace language::Core

#endif
