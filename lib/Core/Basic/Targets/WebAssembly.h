//=== WebAssembly.h - Declare WebAssembly target feature support *- C++ -*-===//
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
// This file declares WebAssembly TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_WEBASSEMBLY_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_WEBASSEMBLY_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

static const unsigned WebAssemblyAddrSpaceMap[] = {
    0,  // Default
    0,  // opencl_global
    0,  // opencl_local
    0,  // opencl_constant
    0,  // opencl_private
    0,  // opencl_generic
    0,  // opencl_global_device
    0,  // opencl_global_host
    0,  // cuda_device
    0,  // cuda_constant
    0,  // cuda_shared
    0,  // sycl_global
    0,  // sycl_global_device
    0,  // sycl_global_host
    0,  // sycl_local
    0,  // sycl_private
    0,  // ptr32_sptr
    0,  // ptr32_uptr
    0,  // ptr64
    0,  // hlsl_groupshared
    0,  // hlsl_constant
    0,  // hlsl_private
    0,  // hlsl_device
    0,  // hlsl_input
    20, // wasm_funcref
};

class LLVM_LIBRARY_VISIBILITY WebAssemblyTargetInfo : public TargetInfo {

  enum SIMDEnum {
    NoSIMD,
    SIMD128,
    RelaxedSIMD,
  } SIMDLevel = NoSIMD;

  bool HasAtomics = false;
  bool HasBulkMemory = false;
  bool HasBulkMemoryOpt = false;
  bool HasCallIndirectOverlong = false;
  bool HasExceptionHandling = false;
  bool HasExtendedConst = false;
  bool HasFP16 = false;
  bool HasGC = false;
  bool HasMultiMemory = false;
  bool HasMultivalue = false;
  bool HasMutableGlobals = false;
  bool HasNontrappingFPToInt = false;
  bool HasReferenceTypes = false;
  bool HasSignExt = false;
  bool HasTailCall = false;
  bool HasWideArithmetic = false;

  std::string ABI;

public:
  explicit WebAssemblyTargetInfo(const toolchain::Triple &T, const TargetOptions &)
      : TargetInfo(T) {
    AddrSpaceMap = &WebAssemblyAddrSpaceMap;
    NoAsmVariants = true;
    SuitableAlign = 128;
    LargeArrayMinWidth = 128;
    LargeArrayAlign = 128;
    SigAtomicType = SignedLong;
    LongDoubleWidth = LongDoubleAlign = 128;
    LongDoubleFormat = &toolchain::APFloat::IEEEquad();
    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
    // size_t being unsigned long for both wasm32 and wasm64 makes mangled names
    // more consistent between the two.
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    HasUnalignedAccess = true;
  }

  StringRef getABI() const override;
  bool setABI(const std::string &Name) override;
  bool useFP16ConversionIntrinsics() const override { return !HasFP16; }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

private:
  static void setSIMDLevel(toolchain::StringMap<bool> &Features, SIMDEnum Level,
                           bool Enabled);

  bool
  initFeatureMap(toolchain::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override;
  bool hasFeature(StringRef Feature) const final;

  void setFeatureEnabled(toolchain::StringMap<bool> &Features, StringRef Name,
                         bool Enabled) const final;

  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) final;

  bool isValidCPUName(StringRef Name) const final;
  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const final;

  bool setCPU(const std::string &Name) final { return isValidCPUName(Name); }

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const final;

  BuiltinVaListKind getBuiltinVaListKind() const final {
    return VoidPtrBuiltinVaList;
  }

  ArrayRef<const char *> getGCCRegNames() const final { return {}; }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const final {
    return {};
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const final {
    return false;
  }

  std::string_view getClobbers() const final { return ""; }

  bool isCLZForZeroUndef() const final { return false; }

  bool hasInt128Type() const final { return true; }

  IntType getIntTypeByWidth(unsigned BitWidth, bool IsSigned) const final {
    // WebAssembly prefers long long for explicitly 64-bit integers.
    return BitWidth == 64 ? (IsSigned ? SignedLongLong : UnsignedLongLong)
                          : TargetInfo::getIntTypeByWidth(BitWidth, IsSigned);
  }

  IntType getLeastIntTypeByWidth(unsigned BitWidth, bool IsSigned) const final {
    // WebAssembly uses long long for int_least64_t and int_fast64_t.
    return BitWidth == 64
               ? (IsSigned ? SignedLongLong : UnsignedLongLong)
               : TargetInfo::getLeastIntTypeByWidth(BitWidth, IsSigned);
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    switch (CC) {
    case CC_C:
    case CC_Swift:
      return CCCR_OK;
    case CC_SwiftAsync:
      return CCCR_Error;
    default:
      return CCCR_Warning;
    }
  }

  bool hasBitIntType() const override { return true; }

  bool hasProtectedVisibility() const override { return false; }

  void adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
              const TargetInfo *Aux) override;
};

class LLVM_LIBRARY_VISIBILITY WebAssembly32TargetInfo
    : public WebAssemblyTargetInfo {
public:
  explicit WebAssembly32TargetInfo(const toolchain::Triple &T,
                                   const TargetOptions &Opts)
      : WebAssemblyTargetInfo(T, Opts) {
    if (T.isOSEmscripten())
      resetDataLayout(
          "e-m:e-p:32:32-p10:8:8-p20:8:8-i64:64-i128:128-f128:64-n32:64-"
          "S128-ni:1:10:20");
    else
      resetDataLayout("e-m:e-p:32:32-p10:8:8-p20:8:8-i64:64-i128:128-n32:64-"
                      "S128-ni:1:10:20");
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY WebAssembly64TargetInfo
    : public WebAssemblyTargetInfo {
public:
  explicit WebAssembly64TargetInfo(const toolchain::Triple &T,
                                   const TargetOptions &Opts)
      : WebAssemblyTargetInfo(T, Opts) {
    LongAlign = LongWidth = 64;
    PointerAlign = PointerWidth = 64;
    SizeType = UnsignedLong;
    PtrDiffType = SignedLong;
    IntPtrType = SignedLong;
    if (T.isOSEmscripten())
      resetDataLayout(
          "e-m:e-p:64:64-p10:8:8-p20:8:8-i64:64-i128:128-f128:64-n32:64-"
          "S128-ni:1:10:20");
    else
      resetDataLayout("e-m:e-p:64:64-p10:8:8-p20:8:8-i64:64-i128:128-n32:64-"
                      "S128-ni:1:10:20");
  }

protected:
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};
} // namespace targets
} // namespace language::Core
#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_WEBASSEMBLY_H
