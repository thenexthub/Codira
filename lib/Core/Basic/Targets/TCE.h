//===--- TCE.h - Declare TCE target feature support -------------*- C++ -*-===//
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
// This file declares TCE TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_TCE_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_TCE_H

#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/Triple.h"

namespace language::Core {
namespace targets {

// toolchain and clang cannot be used directly to output native binaries for
// target, but is used to compile C code to toolchain bitcode with correct
// type and alignment information.
//
// TCE uses the toolchain bitcode as input and uses it for generating customized
// target processor and program binary. TCE co-design environment is
// publicly available in http://tce.cs.tut.fi

static const unsigned TCEOpenCLAddrSpaceMap[] = {
    0, // Default
    3, // opencl_global
    4, // opencl_local
    5, // opencl_constant
    0, // opencl_private
    1, // opencl_global_device
    1, // opencl_global_host
    // FIXME: generic has to be added to the target
    0, // opencl_generic
    0, // cuda_device
    0, // cuda_constant
    0, // cuda_shared
    0, // sycl_global
    0, // sycl_global_device
    0, // sycl_global_host
    0, // sycl_local
    0, // sycl_private
    0, // ptr32_sptr
    0, // ptr32_uptr
    0, // ptr64
    0, // hlsl_groupshared
    0, // hlsl_constant
    0, // hlsl_private
    0, // hlsl_device
    0, // hlsl_input
    // Wasm address space values for this target are dummy values,
    // as it is only enabled for Wasm targets.
    20, // wasm_funcref
};

class LLVM_LIBRARY_VISIBILITY TCETargetInfo : public TargetInfo {
public:
  TCETargetInfo(const toolchain::Triple &Triple, const TargetOptions &)
      : TargetInfo(Triple) {
    TLSSupported = false;
    IntWidth = 32;
    LongWidth = LongLongWidth = 32;
    PointerWidth = 32;
    IntAlign = 32;
    LongAlign = LongLongAlign = 32;
    PointerAlign = 32;
    SuitableAlign = 32;
    SizeType = UnsignedInt;
    IntMaxType = SignedLong;
    IntPtrType = SignedInt;
    PtrDiffType = SignedInt;
    FloatWidth = 32;
    FloatAlign = 32;
    DoubleWidth = 32;
    DoubleAlign = 32;
    LongDoubleWidth = 32;
    LongDoubleAlign = 32;
    FloatFormat = &toolchain::APFloat::IEEEsingle();
    DoubleFormat = &toolchain::APFloat::IEEEsingle();
    LongDoubleFormat = &toolchain::APFloat::IEEEsingle();
    resetDataLayout("E-p:32:32:32-i1:8:8-i8:8:32-"
                    "i16:16:32-i32:32:32-i64:32:32-"
                    "f32:32:32-f64:32:32-v64:32:32-"
                    "v128:32:32-v256:32:32-v512:32:32-"
                    "v1024:32:32-a0:0:32-n32");
    AddrSpaceMap = &TCEOpenCLAddrSpaceMap;
    UseAddrSpaceMapMangling = true;
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool hasFeature(StringRef Feature) const override { return Feature == "tce"; }

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

  std::string_view getClobbers() const override { return ""; }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::VoidPtrBuiltinVaList;
  }

  ArrayRef<const char *> getGCCRegNames() const override { return {}; }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override {
    return true;
  }

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }
};

class LLVM_LIBRARY_VISIBILITY TCELETargetInfo : public TCETargetInfo {
public:
  TCELETargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : TCETargetInfo(Triple, Opts) {
    BigEndian = false;

    resetDataLayout("e-p:32:32:32-i1:8:8-i8:8:32-"
                    "i16:16:32-i32:32:32-i64:32:32-"
                    "f32:32:32-f64:32:32-v64:32:32-"
                    "v128:32:32-v256:32:32-v512:32:32-"
                    "v1024:32:32-a0:0:32-n32");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};
} // namespace targets
} // namespace language::Core
#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_TCE_H
