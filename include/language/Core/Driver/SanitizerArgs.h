//===--- SanitizerArgs.h - Arguments for sanitizer tools  -------*- C++ -*-===//
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
#ifndef LANGUAGE_CORE_DRIVER_SANITIZERARGS_H
#define LANGUAGE_CORE_DRIVER_SANITIZERARGS_H

#include "language/Core/Basic/Sanitizers.h"
#include "language/Core/Driver/Types.h"
#include "toolchain/Option/Arg.h"
#include "toolchain/Option/ArgList.h"
#include "toolchain/Transforms/Instrumentation/AddressSanitizerOptions.h"
#include <string>
#include <vector>

namespace language::Core {
namespace driver {

class ToolChain;

class SanitizerArgs {
  SanitizerSet Sanitizers;
  SanitizerSet RecoverableSanitizers;
  SanitizerSet TrapSanitizers;
  SanitizerSet MergeHandlers;
  SanitizerMaskCutoffs SkipHotCutoffs;
  SanitizerSet AnnotateDebugInfo;

  std::vector<std::string> UserIgnorelistFiles;
  std::vector<std::string> SystemIgnorelistFiles;
  std::vector<std::string> CoverageAllowlistFiles;
  std::vector<std::string> CoverageIgnorelistFiles;
  std::vector<std::string> BinaryMetadataIgnorelistFiles;
  int CoverageFeatures = 0;
  int CoverageStackDepthCallbackMin = 0;
  int BinaryMetadataFeatures = 0;
  int OverflowPatternExclusions = 0;
  int MsanTrackOrigins = 0;
  bool MsanUseAfterDtor = true;
  bool MsanParamRetval = true;
  bool CfiCrossDso = false;
  bool CfiICallGeneralizePointers = false;
  bool CfiICallNormalizeIntegers = false;
  bool CfiCanonicalJumpTables = false;
  bool KcfiArity = false;
  int AsanFieldPadding = 0;
  bool SharedRuntime = false;
  bool StableABI = false;
  bool AsanUseAfterScope = true;
  bool AsanPoisonCustomArrayCookie = false;
  bool AsanGlobalsDeadStripping = false;
  bool AsanUseOdrIndicator = false;
  bool AsanInvalidPointerCmp = false;
  bool AsanInvalidPointerSub = false;
  bool AsanOutlineInstrumentation = false;
  toolchain::AsanDtorKind AsanDtorKind = toolchain::AsanDtorKind::Invalid;
  std::string HwasanAbi;
  bool LinkRuntimes = true;
  bool LinkCXXRuntimes = false;
  bool NeedPIE = false;
  bool SafeStackRuntime = false;
  bool Stats = false;
  bool TsanMemoryAccess = true;
  bool TsanFuncEntryExit = true;
  bool TsanAtomics = true;
  bool MinimalRuntime = false;
  // True if cross-dso CFI support if provided by the system (i.e. Android).
  bool ImplicitCfiRuntime = false;
  bool NeedsMemProfRt = false;
  bool HwasanUseAliases = false;
  toolchain::AsanDetectStackUseAfterReturnMode AsanUseAfterReturn =
      toolchain::AsanDetectStackUseAfterReturnMode::Invalid;

  std::string MemtagMode;

public:
  /// Parses the sanitizer arguments from an argument list.
  SanitizerArgs(const ToolChain &TC, const toolchain::opt::ArgList &Args,
                bool DiagnoseErrors = true);

  bool needsSharedRt() const { return SharedRuntime; }
  bool needsStableAbi() const { return StableABI; }

  bool needsMemProfRt() const { return NeedsMemProfRt; }
  bool needsAsanRt() const { return Sanitizers.has(SanitizerKind::Address); }
  bool needsHwasanRt() const {
    return Sanitizers.has(SanitizerKind::HWAddress);
  }
  bool needsHwasanAliasesRt() const {
    return needsHwasanRt() && HwasanUseAliases;
  }
  bool needsTysanRt() const { return Sanitizers.has(SanitizerKind::Type); }
  bool needsTsanRt() const { return Sanitizers.has(SanitizerKind::Thread); }
  bool needsMsanRt() const { return Sanitizers.has(SanitizerKind::Memory); }
  bool needsFuzzer() const { return Sanitizers.has(SanitizerKind::Fuzzer); }
  bool needsLsanRt() const {
    return Sanitizers.has(SanitizerKind::Leak) &&
           !Sanitizers.has(SanitizerKind::Address) &&
           !Sanitizers.has(SanitizerKind::HWAddress);
  }
  bool needsFuzzerInterceptors() const;
  bool needsUbsanRt() const;
  bool needsUbsanCXXRt() const;
  bool requiresMinimalRuntime() const { return MinimalRuntime; }
  bool needsDfsanRt() const { return Sanitizers.has(SanitizerKind::DataFlow); }
  bool needsSafeStackRt() const { return SafeStackRuntime; }
  bool needsCfiCrossDsoRt() const;
  bool needsCfiCrossDsoDiagRt() const;
  bool needsStatsRt() const { return Stats; }
  bool needsScudoRt() const { return Sanitizers.has(SanitizerKind::Scudo); }
  bool needsNsanRt() const {
    return Sanitizers.has(SanitizerKind::NumericalStability);
  }
  bool needsRtsanRt() const { return Sanitizers.has(SanitizerKind::Realtime); }

  bool hasMemTag() const {
    return hasMemtagHeap() || hasMemtagStack() || hasMemtagGlobals();
  }
  bool hasMemtagHeap() const {
    return Sanitizers.has(SanitizerKind::MemtagHeap);
  }
  bool hasMemtagStack() const {
    return Sanitizers.has(SanitizerKind::MemtagStack);
  }
  bool hasMemtagGlobals() const {
    return Sanitizers.has(SanitizerKind::MemtagGlobals);
  }
  const std::string &getMemtagMode() const {
    assert(!MemtagMode.empty());
    return MemtagMode;
  }

  bool hasShadowCallStack() const {
    return Sanitizers.has(SanitizerKind::ShadowCallStack);
  }

  bool requiresPIE() const;
  bool needsUnwindTables() const;
  bool needsLTO() const;
  bool linkRuntimes() const { return LinkRuntimes; }
  bool linkCXXRuntimes() const { return LinkCXXRuntimes; }
  bool hasCrossDsoCfi() const { return CfiCrossDso; }
  bool hasAnySanitizer() const { return !Sanitizers.empty(); }
  void addArgs(const ToolChain &TC, const toolchain::opt::ArgList &Args,
               toolchain::opt::ArgStringList &CmdArgs, types::ID InputType) const;
};

}  // namespace driver
}  // namespace language::Core

#endif
