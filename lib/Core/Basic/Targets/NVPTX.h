//===--- NVPTX.h - Declare NVPTX target feature support ---------*- C++ -*-===//
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
// This file declares NVPTX TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_NVPTX_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_NVPTX_H

#include "language/Core/Basic/Cuda.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/NVPTXAddrSpace.h"
#include "toolchain/TargetParser/Triple.h"
#include <optional>

namespace language::Core {
namespace targets {

static const unsigned NVPTXAddrSpaceMap[] = {
    0, // Default
    1, // opencl_global
    3, // opencl_local
    4, // opencl_constant
    0, // opencl_private
    // FIXME: generic has to be added to the target
    0, // opencl_generic
    1, // opencl_global_device
    1, // opencl_global_host
    1, // cuda_device
    4, // cuda_constant
    3, // cuda_shared
    1, // sycl_global
    1, // sycl_global_device
    1, // sycl_global_host
    3, // sycl_local
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

/// The DWARF address class. Taken from
/// https://docs.nvidia.com/cuda/archive/10.0/ptx-writers-guide-to-interoperability/index.html#cuda-specific-dwarf
static const int NVPTXDWARFAddrSpaceMap[] = {
    -1, // Default, opencl_private or opencl_generic - not defined
    5,  // opencl_global
    -1,
    8,  // opencl_local or cuda_shared
    4,  // opencl_constant or cuda_constant
};

class LLVM_LIBRARY_VISIBILITY NVPTXTargetInfo : public TargetInfo {
  static const char *const GCCRegNames[];
  OffloadArch GPU;
  uint32_t PTXVersion;
  std::unique_ptr<TargetInfo> HostTarget;

public:
  NVPTXTargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts,
                  unsigned TargetPointerWidth);

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  bool useFP16ConversionIntrinsics() const override { return false; }

  bool
  initFeatureMap(toolchain::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeaturesVec) const override {
    if (GPU != OffloadArch::UNUSED)
      Features[OffloadArchToString(GPU)] = true;
    Features["ptx" + std::to_string(PTXVersion)] = true;
    return TargetInfo::initFeatureMap(Features, Diags, CPU, FeaturesVec);
  }

  bool hasFeature(StringRef Feature) const override;

  virtual bool isAddressSpaceSupersetOf(LangAS A, LangAS B) const override {
    // The generic address space AS(0) is a superset of all the other address
    // spaces used by the backend target.
    return A == B ||
           ((A == LangAS::Default ||
             (isTargetAddressSpace(A) &&
              toTargetAddressSpace(A) ==
                  toolchain::NVPTXAS::ADDRESS_SPACE_GENERIC)) &&
            isTargetAddressSpace(B) &&
            toTargetAddressSpace(B) >= toolchain::NVPTXAS::ADDRESS_SPACE_GENERIC &&
            toTargetAddressSpace(B) <= toolchain::NVPTXAS::ADDRESS_SPACE_LOCAL &&
            toTargetAddressSpace(B) != 2);
  }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    // No aliases.
    return {};
  }

  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    switch (*Name) {
    default:
      return false;
    case 'c':
    case 'h':
    case 'r':
    case 'l':
    case 'f':
    case 'd':
    case 'q':
      Info.setAllowsRegister();
      return true;
    }
  }

  std::string_view getClobbers() const override {
    // FIXME: Is this really right?
    return "";
  }

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  bool isValidCPUName(StringRef Name) const override {
    return StringToOffloadArch(Name) != OffloadArch::UNKNOWN;
  }

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override {
    for (int i = static_cast<int>(OffloadArch::SM_20);
         i < static_cast<int>(OffloadArch::Generic); ++i)
      Values.emplace_back(OffloadArchToString(static_cast<OffloadArch>(i)));
  }

  bool setCPU(const std::string &Name) override {
    GPU = StringToOffloadArch(Name);
    return GPU != OffloadArch::UNKNOWN;
  }

  void setSupportedOpenCLOpts() override {
    auto &Opts = getSupportedOpenCLOpts();
    Opts["cl_clang_storage_class_specifiers"] = true;
    Opts["__cl_clang_function_pointers"] = true;
    Opts["__cl_clang_variadic_functions"] = true;
    Opts["__cl_clang_non_portable_kernel_param_types"] = true;
    Opts["__cl_clang_bitfields"] = true;

    Opts["cl_khr_fp64"] = true;
    Opts["__opencl_c_fp64"] = true;
    Opts["cl_khr_byte_addressable_store"] = true;
    Opts["cl_khr_global_int32_base_atomics"] = true;
    Opts["cl_khr_global_int32_extended_atomics"] = true;
    Opts["cl_khr_local_int32_base_atomics"] = true;
    Opts["cl_khr_local_int32_extended_atomics"] = true;

    Opts["__opencl_c_generic_address_space"] = true;
  }

  const toolchain::omp::GV &getGridValue() const override {
    return toolchain::omp::NVPTXGridValues;
  }

  /// \returns If a target requires an address within a target specific address
  /// space \p AddressSpace to be converted in order to be used, then return the
  /// corresponding target specific DWARF address space.
  ///
  /// \returns Otherwise return std::nullopt and no conversion will be emitted
  /// in the DWARF.
  std::optional<unsigned>
  getDWARFAddressSpace(unsigned AddressSpace) const override {
    if (AddressSpace >= std::size(NVPTXDWARFAddrSpaceMap) ||
        NVPTXDWARFAddrSpaceMap[AddressSpace] < 0)
      return std::nullopt;
    return NVPTXDWARFAddrSpaceMap[AddressSpace];
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    // CUDA compilations support all of the host's calling conventions.
    //
    // TODO: We should warn if you apply a non-default CC to anything other than
    // a host function.
    if (HostTarget)
      return HostTarget->checkCallingConvention(CC);
    return CCCR_Warning;
  }

  bool hasBitIntType() const override { return true; }
  bool hasBFloat16Type() const override { return true; }

  OffloadArch getGPU() const { return GPU; }
};
} // namespace targets
} // namespace language::Core
#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_NVPTX_H
