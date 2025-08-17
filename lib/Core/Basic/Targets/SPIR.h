//===--- SPIR.h - Declare SPIR and SPIR-V target feature support *- C++ -*-===//
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
// This file declares SPIR and SPIR-V TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_SPIR_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_SPIR_H

#include "Targets.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/TargetParser/Triple.h"
#include <optional>

namespace language::Core {
namespace targets {

// Used by both the SPIR and SPIR-V targets.
static const unsigned SPIRDefIsPrivMap[] = {
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
    0,  // sycl_global
    0,  // sycl_global_device
    0,  // sycl_global_host
    0,  // sycl_local
    0,  // sycl_private
    0,  // ptr32_sptr
    0,  // ptr32_uptr
    0,  // ptr64
    3,  // hlsl_groupshared
    12, // hlsl_constant
    10, // hlsl_private
    11, // hlsl_device
    7,  // hlsl_input
    // Wasm address space values for this target are dummy values,
    // as it is only enabled for Wasm targets.
    20, // wasm_funcref
};

// Used by both the SPIR and SPIR-V targets.
static const unsigned SPIRDefIsGenMap[] = {
    4, // Default
    1, // opencl_global
    3, // opencl_local
    2, // opencl_constant
    0, // opencl_private
    4, // opencl_generic
    5, // opencl_global_device
    6, // opencl_global_host
    // cuda_* address space mapping is intended for HIPSPV (HIP to SPIR-V
    // translation). This mapping is enabled when the language mode is HIP.
    1, // cuda_device
    // cuda_constant pointer can be casted to default/"flat" pointer, but in
    // SPIR-V casts between constant and generic pointers are not allowed. For
    // this reason cuda_constant is mapped to SPIR-V CrossWorkgroup.
    1,  // cuda_constant
    3,  // cuda_shared
    1,  // sycl_global
    5,  // sycl_global_device
    6,  // sycl_global_host
    3,  // sycl_local
    0,  // sycl_private
    0,  // ptr32_sptr
    0,  // ptr32_uptr
    0,  // ptr64
    3,  // hlsl_groupshared
    0,  // hlsl_constant
    10, // hlsl_private
    11, // hlsl_device
    7,  // hlsl_input
    // Wasm address space values for this target are dummy values,
    // as it is only enabled for Wasm targets.
    20, // wasm_funcref
};

// Base class for SPIR and SPIR-V target info.
class LLVM_LIBRARY_VISIBILITY BaseSPIRTargetInfo : public TargetInfo {
  std::unique_ptr<TargetInfo> HostTarget;

protected:
  BaseSPIRTargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : TargetInfo(Triple) {
    assert((Triple.isSPIR() || Triple.isSPIRV()) &&
           "Invalid architecture for SPIR or SPIR-V.");
    TLSSupported = false;
    VLASupported = false;
    LongWidth = LongAlign = 64;
    AddrSpaceMap = &SPIRDefIsPrivMap;
    UseAddrSpaceMapMangling = true;
    HasLegalHalfType = true;
    HasFloat16 = true;
    // Define available target features
    // These must be defined in sorted order!
    NoAsmVariants = true;

    toolchain::Triple HostTriple(Opts.HostTriple);
    if (!HostTriple.isSPIR() && !HostTriple.isSPIRV() &&
        HostTriple.getArch() != toolchain::Triple::UnknownArch) {
      HostTarget = AllocateTarget(toolchain::Triple(Opts.HostTriple), Opts);

      // Copy properties from host target.
      BoolWidth = HostTarget->getBoolWidth();
      BoolAlign = HostTarget->getBoolAlign();
      IntWidth = HostTarget->getIntWidth();
      IntAlign = HostTarget->getIntAlign();
      HalfWidth = HostTarget->getHalfWidth();
      HalfAlign = HostTarget->getHalfAlign();
      FloatWidth = HostTarget->getFloatWidth();
      FloatAlign = HostTarget->getFloatAlign();
      DoubleWidth = HostTarget->getDoubleWidth();
      DoubleAlign = HostTarget->getDoubleAlign();
      LongWidth = HostTarget->getLongWidth();
      LongAlign = HostTarget->getLongAlign();
      LongLongWidth = HostTarget->getLongLongWidth();
      LongLongAlign = HostTarget->getLongLongAlign();
      MinGlobalAlign =
          HostTarget->getMinGlobalAlign(/* TypeSize = */ 0,
                                        /* HasNonWeakDef = */ true);
      NewAlign = HostTarget->getNewAlign();
      DefaultAlignForAttributeAligned =
          HostTarget->getDefaultAlignForAttributeAligned();
      IntMaxType = HostTarget->getIntMaxType();
      WCharType = HostTarget->getWCharType();
      WIntType = HostTarget->getWIntType();
      Char16Type = HostTarget->getChar16Type();
      Char32Type = HostTarget->getChar32Type();
      Int64Type = HostTarget->getInt64Type();
      SigAtomicType = HostTarget->getSigAtomicType();
      ProcessIDType = HostTarget->getProcessIDType();

      UseBitFieldTypeAlignment = HostTarget->useBitFieldTypeAlignment();
      UseZeroLengthBitfieldAlignment =
          HostTarget->useZeroLengthBitfieldAlignment();
      UseExplicitBitFieldAlignment = HostTarget->useExplicitBitFieldAlignment();
      ZeroLengthBitfieldBoundary = HostTarget->getZeroLengthBitfieldBoundary();

      // This is a bit of a lie, but it controls __GCC_ATOMIC_XXX_LOCK_FREE, and
      // we need those macros to be identical on host and device, because (among
      // other things) they affect which standard library classes are defined,
      // and we need all classes to be defined on both the host and device.
      MaxAtomicInlineWidth = HostTarget->getMaxAtomicInlineWidth();
    }
  }

public:
  // SPIR supports the half type and the only toolchain intrinsic allowed in SPIR is
  // memcpy as per section 3 of the SPIR spec.
  bool useFP16ConversionIntrinsics() const override { return false; }

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override {
    return {};
  }

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

  std::optional<unsigned>
  getDWARFAddressSpace(unsigned AddressSpace) const override {
    return AddressSpace;
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    return (CC == CC_SpirFunction || CC == CC_DeviceKernel) ? CCCR_OK
                                                            : CCCR_Warning;
  }

  CallingConv getDefaultCallingConv() const override {
    return CC_SpirFunction;
  }

  void setAddressSpaceMap(bool DefaultIsGeneric) {
    AddrSpaceMap = DefaultIsGeneric ? &SPIRDefIsGenMap : &SPIRDefIsPrivMap;
  }

  void adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
              const TargetInfo *Aux) override {
    TargetInfo::adjust(Diags, Opts, Aux);
    // FIXME: SYCL specification considers unannotated pointers and references
    // to be pointing to the generic address space. See section 5.9.3 of
    // SYCL 2020 specification.
    // Currently, there is no way of representing SYCL's and HIP/CUDA's default
    // address space language semantic along with the semantics of embedded C's
    // default address space in the same address space map. Hence the map needs
    // to be reset to allow mapping to the desired value of 'Default' entry for
    // SYCL and HIP/CUDA.
    setAddressSpaceMap(
        /*DefaultIsGeneric=*/Opts.SYCLIsDevice ||
        // The address mapping from HIP/CUDA language for device code is only
        // defined for SPIR-V.
        (getTriple().isSPIRV() && Opts.CUDAIsDevice));
  }

  void setSupportedOpenCLOpts() override {
    // Assume all OpenCL extensions and optional core features are supported
    // for SPIR and SPIR-V since they are generic targets.
    supportAllOpenCLOpts();
  }

  bool hasBitIntType() const override { return true; }

  bool hasInt128Type() const override { return false; }
};

class LLVM_LIBRARY_VISIBILITY SPIRTargetInfo : public BaseSPIRTargetInfo {
public:
  SPIRTargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : BaseSPIRTargetInfo(Triple, Opts) {
    assert(Triple.isSPIR() && "Invalid architecture for SPIR.");
    assert(getTriple().getOS() == toolchain::Triple::UnknownOS &&
           "SPIR target must use unknown OS");
    assert(getTriple().getEnvironment() == toolchain::Triple::UnknownEnvironment &&
           "SPIR target must use unknown environment type");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool hasFeature(StringRef Feature) const override {
    return Feature == "spir";
  }

  bool checkArithmeticFenceSupported() const override { return true; }
};

class LLVM_LIBRARY_VISIBILITY SPIR32TargetInfo : public SPIRTargetInfo {
public:
  SPIR32TargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : SPIRTargetInfo(Triple, Opts) {
    assert(Triple.getArch() == toolchain::Triple::spir &&
           "Invalid architecture for 32-bit SPIR.");
    PointerWidth = PointerAlign = 32;
    SizeType = TargetInfo::UnsignedInt;
    PtrDiffType = IntPtrType = TargetInfo::SignedInt;
    // SPIR32 has support for atomic ops if atomic extension is enabled.
    // Take the maximum because it's possible the Host supports wider types.
    MaxAtomicInlineWidth = std::max<unsigned char>(MaxAtomicInlineWidth, 32);
    resetDataLayout("e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-"
                    "v96:128-v192:256-v256:256-v512:512-v1024:1024-G1");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY SPIR64TargetInfo : public SPIRTargetInfo {
public:
  SPIR64TargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : SPIRTargetInfo(Triple, Opts) {
    assert(Triple.getArch() == toolchain::Triple::spir64 &&
           "Invalid architecture for 64-bit SPIR.");
    PointerWidth = PointerAlign = 64;
    SizeType = TargetInfo::UnsignedLong;
    PtrDiffType = IntPtrType = TargetInfo::SignedLong;
    // SPIR64 has support for atomic ops if atomic extension is enabled.
    // Take the maximum because it's possible the Host supports wider types.
    MaxAtomicInlineWidth = std::max<unsigned char>(MaxAtomicInlineWidth, 64);
    resetDataLayout("e-i64:64-v16:16-v24:32-v32:32-v48:64-"
                    "v96:128-v192:256-v256:256-v512:512-v1024:1024-G1");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY BaseSPIRVTargetInfo : public BaseSPIRTargetInfo {
public:
  BaseSPIRVTargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : BaseSPIRTargetInfo(Triple, Opts) {
    assert(Triple.isSPIRV() && "Invalid architecture for SPIR-V.");
  }

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  bool hasFeature(StringRef Feature) const override {
    return Feature == "spirv";
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY SPIRVTargetInfo : public BaseSPIRVTargetInfo {
public:
  SPIRVTargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : BaseSPIRVTargetInfo(Triple, Opts) {
    assert(Triple.getArch() == toolchain::Triple::spirv &&
           "Invalid architecture for Logical SPIR-V.");
    assert(Triple.getOS() == toolchain::Triple::Vulkan &&
           Triple.getVulkanVersion() != toolchain::VersionTuple(0) &&
           "Logical SPIR-V requires a valid Vulkan environment.");
    assert(Triple.getEnvironment() >= toolchain::Triple::Pixel &&
           Triple.getEnvironment() <= toolchain::Triple::Amplification &&
           "Logical SPIR-V environment must be a valid shader stage.");
    PointerWidth = PointerAlign = 64;

    // SPIR-V IDs are represented with a single 32-bit word.
    SizeType = TargetInfo::UnsignedInt;
    resetDataLayout("e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-"
                    "v256:256-v512:512-v1024:1024-n8:16:32:64-G10");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY SPIRV32TargetInfo : public BaseSPIRVTargetInfo {
public:
  SPIRV32TargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : BaseSPIRVTargetInfo(Triple, Opts) {
    assert(Triple.getArch() == toolchain::Triple::spirv32 &&
           "Invalid architecture for 32-bit SPIR-V.");
    assert(getTriple().getOS() == toolchain::Triple::UnknownOS &&
           "32-bit SPIR-V target must use unknown OS");
    assert(getTriple().getEnvironment() == toolchain::Triple::UnknownEnvironment &&
           "32-bit SPIR-V target must use unknown environment type");
    PointerWidth = PointerAlign = 32;
    SizeType = TargetInfo::UnsignedInt;
    PtrDiffType = IntPtrType = TargetInfo::SignedInt;
    // SPIR-V has core support for atomic ops, and Int32 is always available;
    // we take the maximum because it's possible the Host supports wider types.
    MaxAtomicInlineWidth = std::max<unsigned char>(MaxAtomicInlineWidth, 32);
    resetDataLayout("e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-"
                    "v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64-G1");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;
};

class LLVM_LIBRARY_VISIBILITY SPIRV64TargetInfo : public BaseSPIRVTargetInfo {
public:
  SPIRV64TargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : BaseSPIRVTargetInfo(Triple, Opts) {
    assert(Triple.getArch() == toolchain::Triple::spirv64 &&
           "Invalid architecture for 64-bit SPIR-V.");
    assert(getTriple().getOS() == toolchain::Triple::UnknownOS &&
           "64-bit SPIR-V target must use unknown OS");
    assert(getTriple().getEnvironment() == toolchain::Triple::UnknownEnvironment &&
           "64-bit SPIR-V target must use unknown environment type");
    PointerWidth = PointerAlign = 64;
    SizeType = TargetInfo::UnsignedLong;
    PtrDiffType = IntPtrType = TargetInfo::SignedLong;
    // SPIR-V has core support for atomic ops, and Int64 is always available;
    // we take the maximum because it's possible the Host supports wider types.
    MaxAtomicInlineWidth = std::max<unsigned char>(MaxAtomicInlineWidth, 64);
    resetDataLayout("e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-"
                    "v256:256-v512:512-v1024:1024-n8:16:32:64-G1");
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  const toolchain::omp::GV &getGridValue() const override {
    return toolchain::omp::SPIRVGridValues;
  }

  std::optional<LangAS> getConstantAddressSpace() const override {
    return ConstantAS;
  }
  void adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
              const TargetInfo *Aux) override {
    BaseSPIRVTargetInfo::adjust(Diags, Opts, Aux);
    // opencl_constant will map to UniformConstant in SPIR-V
    if (Opts.OpenCL)
      ConstantAS = LangAS::opencl_constant;
  }

private:
  // opencl_global will map to CrossWorkgroup in SPIR-V
  LangAS ConstantAS = LangAS::opencl_global;
};

class LLVM_LIBRARY_VISIBILITY SPIRV64AMDGCNTargetInfo final
    : public BaseSPIRVTargetInfo {
public:
  SPIRV64AMDGCNTargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts)
      : BaseSPIRVTargetInfo(Triple, Opts) {
    assert(Triple.getArch() == toolchain::Triple::spirv64 &&
           "Invalid architecture for 64-bit AMDGCN SPIR-V.");
    assert(Triple.getVendor() == toolchain::Triple::VendorType::AMD &&
           "64-bit AMDGCN SPIR-V target must use AMD vendor");
    assert(getTriple().getOS() == toolchain::Triple::OSType::AMDHSA &&
           "64-bit AMDGCN SPIR-V target must use AMDHSA OS");
    assert(getTriple().getEnvironment() == toolchain::Triple::UnknownEnvironment &&
           "64-bit SPIR-V target must use unknown environment type");
    PointerWidth = PointerAlign = 64;
    SizeType = TargetInfo::UnsignedLong;
    PtrDiffType = IntPtrType = TargetInfo::SignedLong;
    AddrSpaceMap = &SPIRDefIsGenMap;

    resetDataLayout("e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-"
                    "v256:256-v512:512-v1024:1024-n32:64-S32-G1-P4-A0");

    BFloat16Width = BFloat16Align = 16;
    BFloat16Format = &toolchain::APFloat::BFloat();

    HasLegalHalfType = true;
    HasFloat16 = true;
    HalfArgsAndReturns = true;

    MaxAtomicPromoteWidth = MaxAtomicInlineWidth = 64;
  }

  bool hasBFloat16Type() const override { return true; }

  ArrayRef<const char *> getGCCRegNames() const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  bool initFeatureMap(toolchain::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                      StringRef,
                      const std::vector<std::string> &) const override;

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override;

  std::string convertConstraint(const char *&Constraint) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  void setAuxTarget(const TargetInfo *Aux) override;

  void adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
              const TargetInfo *Aux) override {
    TargetInfo::adjust(Diags, Opts, Aux);
  }

  bool hasInt128Type() const override { return TargetInfo::hasInt128Type(); }
};

} // namespace targets
} // namespace language::Core
#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_SPIR_H
