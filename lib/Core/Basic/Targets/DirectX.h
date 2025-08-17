//===--- DirectX.h - Declare DirectX target feature support -----*- C++ -*-===//
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
// This file declares DXIL TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_DIRECTX_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_DIRECTX_H
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

static const unsigned DirectXAddrSpaceMap[] = {
    0, // Default
    1, // opencl_global
    3, // opencl_local
    2, // opencl_constant
    0, // opencl_private
    4, // opencl_generic
    5, // opencl_global_device
    6, // opencl_global_host
    0, // cuda_device
    0, // cuda_constant
    0, // cuda_shared
    // SYCL address space values for this map are dummy
    0, // sycl_global
    0, // sycl_global_device
    0, // sycl_global_host
    0, // sycl_local
    0, // sycl_private
    0, // ptr32_sptr
    0, // ptr32_uptr
    0, // ptr64
    3, // hlsl_groupshared
    2, // hlsl_constant
    0, // hlsl_private
    0, // hlsl_device
    0, // hlsl_input
    // Wasm address space values for this target are dummy values,
    // as it is only enabled for Wasm targets.
    20, // wasm_funcref
};

class LLVM_LIBRARY_VISIBILITY DirectXTargetInfo : public TargetInfo {
public:
  DirectXTargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    TLSSupported = false;
    VLASupported = false;
    AddrSpaceMap = &DirectXAddrSpaceMap;
    UseAddrSpaceMapMangling = true;
    HasLegalHalfType = true;
    HasFloat16 = true;
    NoAsmVariants = true;
    PlatformMinVersion = Triple.getOSVersion();
    PlatformName = toolchain::Triple::getOSTypeName(Triple.getOS());
    resetDataLayout("e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:"
                    "32-f64:64-n8:16:32:64");
    TheCXXABI.set(TargetCXXABI::GenericItanium);
  }
  bool useFP16ConversionIntrinsics() const override { return false; }
  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool hasFeature(StringRef Feature) const override {
    return Feature == "directx";
  }

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override { return {}; }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  void adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
              const TargetInfo *Aux) override {
    TargetInfo::adjust(Diags, Opts, Aux);
    // The static values this addresses do not apply outside of the same thread
    // This protection is neither available nor needed
    Opts.ThreadsafeStatics = false;
  }
};

} // namespace targets
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_DIRECTX_H
