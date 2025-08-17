//===--- WebAssembly.cpp - Implement WebAssembly target feature support ---===//
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
//
// This file implements WebAssembly TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "WebAssembly.h"
#include "Targets.h"
#include "language/Core/Basic/Builtins.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/TargetBuiltins.h"
#include "toolchain/ADT/StringSwitch.h"

using namespace language::Core;
using namespace language::Core::targets;

static constexpr int NumBuiltins =
    language::Core::WebAssembly::LastTSBuiltin - Builtin::FirstTSBuiltin;

static constexpr toolchain::StringTable BuiltinStrings =
    CLANG_BUILTIN_STR_TABLE_START
#define BUILTIN CLANG_BUILTIN_STR_TABLE
#define TARGET_BUILTIN CLANG_TARGET_BUILTIN_STR_TABLE
#include "language/Core/Basic/BuiltinsWebAssembly.def"
    ;

static constexpr auto BuiltinInfos = Builtin::MakeInfos<NumBuiltins>({
#define BUILTIN CLANG_BUILTIN_ENTRY
#define TARGET_BUILTIN CLANG_TARGET_BUILTIN_ENTRY
#define LIBBUILTIN CLANG_LIBBUILTIN_ENTRY
#include "language/Core/Basic/BuiltinsWebAssembly.def"
});

static constexpr toolchain::StringLiteral ValidCPUNames[] = {
    {"mvp"}, {"bleeding-edge"}, {"generic"}, {"lime1"}};

StringRef WebAssemblyTargetInfo::getABI() const { return ABI; }

bool WebAssemblyTargetInfo::setABI(const std::string &Name) {
  if (Name != "mvp" && Name != "experimental-mv")
    return false;

  ABI = Name;
  return true;
}

bool WebAssemblyTargetInfo::hasFeature(StringRef Feature) const {
  return toolchain::StringSwitch<bool>(Feature)
      .Case("atomics", HasAtomics)
      .Case("bulk-memory", HasBulkMemory)
      .Case("bulk-memory-opt", HasBulkMemoryOpt)
      .Case("call-indirect-overlong", HasCallIndirectOverlong)
      .Case("exception-handling", HasExceptionHandling)
      .Case("extended-const", HasExtendedConst)
      .Case("fp16", HasFP16)
      .Case("gc", HasGC)
      .Case("multimemory", HasMultiMemory)
      .Case("multivalue", HasMultivalue)
      .Case("mutable-globals", HasMutableGlobals)
      .Case("nontrapping-fptoint", HasNontrappingFPToInt)
      .Case("reference-types", HasReferenceTypes)
      .Case("relaxed-simd", SIMDLevel >= RelaxedSIMD)
      .Case("sign-ext", HasSignExt)
      .Case("simd128", SIMDLevel >= SIMD128)
      .Case("tail-call", HasTailCall)
      .Case("wide-arithmetic", HasWideArithmetic)
      .Default(false);
}

bool WebAssemblyTargetInfo::isValidCPUName(StringRef Name) const {
  return toolchain::is_contained(ValidCPUNames, Name);
}

void WebAssemblyTargetInfo::fillValidCPUList(
    SmallVectorImpl<StringRef> &Values) const {
  Values.append(std::begin(ValidCPUNames), std::end(ValidCPUNames));
}

void WebAssemblyTargetInfo::getTargetDefines(const LangOptions &Opts,
                                             MacroBuilder &Builder) const {
  defineCPUMacros(Builder, "wasm", /*Tuning=*/false);
  if (HasAtomics)
    Builder.defineMacro("__wasm_atomics__");
  if (HasBulkMemory)
    Builder.defineMacro("__wasm_bulk_memory__");
  if (HasBulkMemoryOpt)
    Builder.defineMacro("__wasm_bulk_memory_opt__");
  if (HasExceptionHandling)
    Builder.defineMacro("__wasm_exception_handling__");
  if (HasExtendedConst)
    Builder.defineMacro("__wasm_extended_const__");
  if (HasMultiMemory)
    Builder.defineMacro("__wasm_multimemory__");
  if (HasFP16)
    Builder.defineMacro("__wasm_fp16__");
  if (HasGC)
    Builder.defineMacro("__wasm_gc__");
  if (HasMultivalue)
    Builder.defineMacro("__wasm_multivalue__");
  if (HasMutableGlobals)
    Builder.defineMacro("__wasm_mutable_globals__");
  if (HasNontrappingFPToInt)
    Builder.defineMacro("__wasm_nontrapping_fptoint__");
  if (HasReferenceTypes)
    Builder.defineMacro("__wasm_reference_types__");
  if (SIMDLevel >= RelaxedSIMD)
    Builder.defineMacro("__wasm_relaxed_simd__");
  if (HasSignExt)
    Builder.defineMacro("__wasm_sign_ext__");
  if (SIMDLevel >= SIMD128)
    Builder.defineMacro("__wasm_simd128__");
  if (HasTailCall)
    Builder.defineMacro("__wasm_tail_call__");
  if (HasWideArithmetic)
    Builder.defineMacro("__wasm_wide_arithmetic__");

  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_1");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_2");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4");
  Builder.defineMacro("__GCC_HAVE_SYNC_COMPARE_AND_SWAP_8");
}

void WebAssemblyTargetInfo::setSIMDLevel(toolchain::StringMap<bool> &Features,
                                         SIMDEnum Level, bool Enabled) {
  if (Enabled) {
    switch (Level) {
    case RelaxedSIMD:
      Features["relaxed-simd"] = true;
      [[fallthrough]];
    case SIMD128:
      Features["simd128"] = true;
      [[fallthrough]];
    case NoSIMD:
      break;
    }
    return;
  }

  switch (Level) {
  case NoSIMD:
  case SIMD128:
    Features["simd128"] = false;
    [[fallthrough]];
  case RelaxedSIMD:
    Features["relaxed-simd"] = false;
    break;
  }
}

void WebAssemblyTargetInfo::setFeatureEnabled(toolchain::StringMap<bool> &Features,
                                              StringRef Name,
                                              bool Enabled) const {
  if (Name == "simd128")
    setSIMDLevel(Features, SIMD128, Enabled);
  else if (Name == "relaxed-simd")
    setSIMDLevel(Features, RelaxedSIMD, Enabled);
  else
    Features[Name] = Enabled;
}

bool WebAssemblyTargetInfo::initFeatureMap(
    toolchain::StringMap<bool> &Features, DiagnosticsEngine &Diags, StringRef CPU,
    const std::vector<std::string> &FeaturesVec) const {
  auto addGenericFeatures = [&]() {
    Features["bulk-memory"] = true;
    Features["bulk-memory-opt"] = true;
    Features["call-indirect-overlong"] = true;
    Features["multivalue"] = true;
    Features["mutable-globals"] = true;
    Features["nontrapping-fptoint"] = true;
    Features["reference-types"] = true;
    Features["sign-ext"] = true;
  };
  auto addLime1Features = [&]() {
    // Lime1:
    // <https://github.com/WebAssembly/tool-conventions/blob/main/Lime.md#lime1>
    Features["bulk-memory-opt"] = true;
    Features["call-indirect-overlong"] = true;
    Features["extended-const"] = true;
    Features["multivalue"] = true;
    Features["mutable-globals"] = true;
    Features["nontrapping-fptoint"] = true;
    Features["sign-ext"] = true;
  };
  auto addBleedingEdgeFeatures = [&]() {
    addGenericFeatures();
    Features["atomics"] = true;
    Features["exception-handling"] = true;
    Features["extended-const"] = true;
    Features["fp16"] = true;
    Features["gc"] = true;
    Features["multimemory"] = true;
    Features["tail-call"] = true;
    Features["wide-arithmetic"] = true;
    setSIMDLevel(Features, RelaxedSIMD, true);
  };
  if (CPU == "generic") {
    addGenericFeatures();
  } else if (CPU == "lime1") {
    addLime1Features();
  } else if (CPU == "bleeding-edge") {
    addBleedingEdgeFeatures();
  }

  return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
}

bool WebAssemblyTargetInfo::handleTargetFeatures(
    std::vector<std::string> &Features, DiagnosticsEngine &Diags) {
  for (const auto &Feature : Features) {
    if (Feature == "+atomics") {
      HasAtomics = true;
      continue;
    }
    if (Feature == "-atomics") {
      HasAtomics = false;
      continue;
    }
    if (Feature == "+bulk-memory") {
      HasBulkMemory = true;
      continue;
    }
    if (Feature == "-bulk-memory") {
      HasBulkMemory = false;
      continue;
    }
    if (Feature == "+bulk-memory-opt") {
      HasBulkMemoryOpt = true;
      continue;
    }
    if (Feature == "-bulk-memory-opt") {
      HasBulkMemoryOpt = false;
      continue;
    }
    if (Feature == "+call-indirect-overlong") {
      HasCallIndirectOverlong = true;
      continue;
    }
    if (Feature == "-call-indirect-overlong") {
      HasCallIndirectOverlong = false;
      continue;
    }
    if (Feature == "+exception-handling") {
      HasExceptionHandling = true;
      continue;
    }
    if (Feature == "-exception-handling") {
      HasExceptionHandling = false;
      continue;
    }
    if (Feature == "+extended-const") {
      HasExtendedConst = true;
      continue;
    }
    if (Feature == "-extended-const") {
      HasExtendedConst = false;
      continue;
    }
    if (Feature == "+fp16") {
      SIMDLevel = std::max(SIMDLevel, SIMD128);
      HasFP16 = true;
      continue;
    }
    if (Feature == "-fp16") {
      HasFP16 = false;
      continue;
    }
    if (Feature == "+gc") {
      HasGC = true;
      continue;
    }
    if (Feature == "-gc") {
      HasGC = false;
      continue;
    }
    if (Feature == "+multimemory") {
      HasMultiMemory = true;
      continue;
    }
    if (Feature == "-multimemory") {
      HasMultiMemory = false;
      continue;
    }
    if (Feature == "+multivalue") {
      HasMultivalue = true;
      continue;
    }
    if (Feature == "-multivalue") {
      HasMultivalue = false;
      continue;
    }
    if (Feature == "+mutable-globals") {
      HasMutableGlobals = true;
      continue;
    }
    if (Feature == "-mutable-globals") {
      HasMutableGlobals = false;
      continue;
    }
    if (Feature == "+nontrapping-fptoint") {
      HasNontrappingFPToInt = true;
      continue;
    }
    if (Feature == "-nontrapping-fptoint") {
      HasNontrappingFPToInt = false;
      continue;
    }
    if (Feature == "+reference-types") {
      HasReferenceTypes = true;
      continue;
    }
    if (Feature == "-reference-types") {
      HasReferenceTypes = false;
      continue;
    }
    if (Feature == "+relaxed-simd") {
      SIMDLevel = std::max(SIMDLevel, RelaxedSIMD);
      continue;
    }
    if (Feature == "-relaxed-simd") {
      SIMDLevel = std::min(SIMDLevel, SIMDEnum(RelaxedSIMD - 1));
      continue;
    }
    if (Feature == "+sign-ext") {
      HasSignExt = true;
      continue;
    }
    if (Feature == "-sign-ext") {
      HasSignExt = false;
      continue;
    }
    if (Feature == "+simd128") {
      SIMDLevel = std::max(SIMDLevel, SIMD128);
      continue;
    }
    if (Feature == "-simd128") {
      SIMDLevel = std::min(SIMDLevel, SIMDEnum(SIMD128 - 1));
      continue;
    }
    if (Feature == "+tail-call") {
      HasTailCall = true;
      continue;
    }
    if (Feature == "-tail-call") {
      HasTailCall = false;
      continue;
    }
    if (Feature == "+wide-arithmetic") {
      HasWideArithmetic = true;
      continue;
    }
    if (Feature == "-wide-arithmetic") {
      HasWideArithmetic = false;
      continue;
    }

    Diags.Report(diag::err_opt_not_valid_with_opt)
        << Feature << "-target-feature";
    return false;
  }

  // gc implies reference-types
  if (HasGC) {
    HasReferenceTypes = true;
  }

  // bulk-memory-opt is a subset of bulk-memory.
  if (HasBulkMemory) {
    HasBulkMemoryOpt = true;
  }

  // The reference-types feature included the change to `call_indirect`
  // encodings to support overlong immediates.
  if (HasReferenceTypes) {
    HasCallIndirectOverlong = true;
  }

  return true;
}

toolchain::SmallVector<Builtin::InfosShard>
WebAssemblyTargetInfo::getTargetBuiltins() const {
  return {{&BuiltinStrings, BuiltinInfos}};
}

void WebAssemblyTargetInfo::adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
                                   const TargetInfo *Aux) {
  TargetInfo::adjust(Diags, Opts, Aux);
  // Turn off POSIXThreads and ThreadModel so that we don't predefine _REENTRANT
  // or __STDCPP_THREADS__ if we will eventually end up stripping atomics
  // because they are unsupported.
  if (!HasAtomics || !HasBulkMemory) {
    Opts.POSIXThreads = false;
    Opts.setThreadModel(LangOptions::ThreadModelKind::Single);
    Opts.ThreadsafeStatics = false;
  }
}

void WebAssembly32TargetInfo::getTargetDefines(const LangOptions &Opts,
                                               MacroBuilder &Builder) const {
  WebAssemblyTargetInfo::getTargetDefines(Opts, Builder);
  defineCPUMacros(Builder, "wasm32", /*Tuning=*/false);
}

void WebAssembly64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                               MacroBuilder &Builder) const {
  WebAssemblyTargetInfo::getTargetDefines(Opts, Builder);
  defineCPUMacros(Builder, "wasm64", /*Tuning=*/false);
}
