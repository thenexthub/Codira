//===--- AMDGPU.h - Declare AMDGPU target feature support -------*- C++ -*-===//
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
// This file declares AMDGPU TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_BASIC_TARGETS_AMDGPU_H
#define LANGUAGE_CORE_LIB_BASIC_TARGETS_AMDGPU_H

#include "language/Core/Basic/TargetID.h"
#include "language/Core/Basic/TargetInfo.h"
#include "language/Core/Basic/TargetOptions.h"
#include "toolchain/ADT/StringSet.h"
#include "toolchain/Support/AMDGPUAddrSpace.h"
#include "toolchain/Support/Compiler.h"
#include "toolchain/TargetParser/TargetParser.h"
#include "toolchain/TargetParser/Triple.h"
#include <optional>

namespace language::Core {
namespace targets {

class LLVM_LIBRARY_VISIBILITY AMDGPUTargetInfo final : public TargetInfo {

  static const char *const GCCRegNames[];

  static const LangASMap AMDGPUDefIsGenMap;
  static const LangASMap AMDGPUDefIsPrivMap;

  toolchain::AMDGPU::GPUKind GPUKind;
  unsigned GPUFeatures;
  unsigned WavefrontSize;

  /// Whether to use cumode or WGP mode. True for cumode. False for WGP mode.
  bool CUMode;

  /// Whether having image instructions.
  bool HasImage = false;

  /// Target ID is device name followed by optional feature name postfixed
  /// by plus or minus sign delimitted by colon, e.g. gfx908:xnack+:sramecc-.
  /// If the target ID contains feature+, map it to true.
  /// If the target ID contains feature-, map it to false.
  /// If the target ID does not contain a feature (default), do not map it.
  toolchain::StringMap<bool> OffloadArchFeatures;
  std::string TargetID;

  bool hasFP64() const {
    return getTriple().isAMDGCN() ||
           !!(GPUFeatures & toolchain::AMDGPU::FEATURE_FP64);
  }

  /// Has fast fma f32
  bool hasFastFMAF() const {
    return !!(GPUFeatures & toolchain::AMDGPU::FEATURE_FAST_FMA_F32);
  }

  /// Has fast fma f64
  bool hasFastFMA() const { return getTriple().isAMDGCN(); }

  bool hasFMAF() const {
    return getTriple().isAMDGCN() ||
           !!(GPUFeatures & toolchain::AMDGPU::FEATURE_FMA);
  }

  bool hasFullRateDenormalsF32() const {
    return !!(GPUFeatures & toolchain::AMDGPU::FEATURE_FAST_DENORMAL_F32);
  }

  bool hasLDEXPF() const {
    return getTriple().isAMDGCN() ||
           !!(GPUFeatures & toolchain::AMDGPU::FEATURE_LDEXP);
  }

  static bool isAMDGCN(const toolchain::Triple &TT) { return TT.isAMDGCN(); }

  static bool isR600(const toolchain::Triple &TT) {
    return TT.getArch() == toolchain::Triple::r600;
  }

public:
  AMDGPUTargetInfo(const toolchain::Triple &Triple, const TargetOptions &Opts);

  void setAddressSpaceMap(bool DefaultIsPrivate);

  void adjust(DiagnosticsEngine &Diags, LangOptions &Opts,
              const TargetInfo *Aux) override;

  uint64_t getPointerWidthV(LangAS AS) const override {
    if (isR600(getTriple()))
      return 32;
    unsigned TargetAS = getTargetAddressSpace(AS);

    if (TargetAS == toolchain::AMDGPUAS::PRIVATE_ADDRESS ||
        TargetAS == toolchain::AMDGPUAS::LOCAL_ADDRESS)
      return 32;

    return 64;
  }

  uint64_t getPointerAlignV(LangAS AddrSpace) const override {
    return getPointerWidthV(AddrSpace);
  }

  virtual bool isAddressSpaceSupersetOf(LangAS A, LangAS B) const override {
    // The flat address space AS(0) is a superset of all the other address
    // spaces used by the backend target.
    return A == B ||
           ((A == LangAS::Default ||
             (isTargetAddressSpace(A) &&
              toTargetAddressSpace(A) == toolchain::AMDGPUAS::FLAT_ADDRESS)) &&
            isTargetAddressSpace(B) &&
            toTargetAddressSpace(B) >= toolchain::AMDGPUAS::FLAT_ADDRESS &&
            toTargetAddressSpace(B) <= toolchain::AMDGPUAS::PRIVATE_ADDRESS &&
            toTargetAddressSpace(B) != toolchain::AMDGPUAS::REGION_ADDRESS);
  }

  uint64_t getMaxPointerWidth() const override {
    return getTriple().isAMDGCN() ? 64 : 32;
  }

  bool hasBFloat16Type() const override { return isAMDGCN(getTriple()); }

  std::string_view getClobbers() const override { return ""; }

  ArrayRef<const char *> getGCCRegNames() const override;

  ArrayRef<TargetInfo::GCCRegAlias> getGCCRegAliases() const override {
    return {};
  }

  /// Accepted register names: (n, m is unsigned integer, n < m)
  /// v
  /// s
  /// a
  /// {vn}, {v[n]}
  /// {sn}, {s[n]}
  /// {an}, {a[n]}
  /// {S} , where S is a special register name
  ////{v[n:m]}
  /// {s[n:m]}
  /// {a[n:m]}
  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &Info) const override {
    static const ::toolchain::StringSet<> SpecialRegs({
        "exec", "vcc", "flat_scratch", "m0", "scc", "tba", "tma",
        "flat_scratch_lo", "flat_scratch_hi", "vcc_lo", "vcc_hi", "exec_lo",
        "exec_hi", "tma_lo", "tma_hi", "tba_lo", "tba_hi",
    });

    switch (*Name) {
    case 'I':
      Info.setRequiresImmediate(-16, 64);
      return true;
    case 'J':
      Info.setRequiresImmediate(-32768, 32767);
      return true;
    case 'A':
    case 'B':
    case 'C':
      Info.setRequiresImmediate();
      return true;
    default:
      break;
    }

    StringRef S(Name);

    if (S == "DA" || S == "DB") {
      Name++;
      Info.setRequiresImmediate();
      return true;
    }

    bool HasLeftParen = S.consume_front("{");
    if (S.empty())
      return false;
    if (S.front() != 'v' && S.front() != 's' && S.front() != 'a') {
      if (!HasLeftParen)
        return false;
      auto E = S.find('}');
      if (!SpecialRegs.count(S.substr(0, E)))
        return false;
      S = S.drop_front(E + 1);
      if (!S.empty())
        return false;
      // Found {S} where S is a special register.
      Info.setAllowsRegister();
      Name = S.data() - 1;
      return true;
    }
    S = S.drop_front();
    if (!HasLeftParen) {
      if (!S.empty())
        return false;
      // Found s, v or a.
      Info.setAllowsRegister();
      Name = S.data() - 1;
      return true;
    }
    bool HasLeftBracket = S.consume_front("[");
    unsigned long long N;
    if (S.empty() || consumeUnsignedInteger(S, 10, N))
      return false;
    if (S.consume_front(":")) {
      if (!HasLeftBracket)
        return false;
      unsigned long long M;
      if (consumeUnsignedInteger(S, 10, M) || N >= M)
        return false;
    }
    if (HasLeftBracket) {
      if (!S.consume_front("]"))
        return false;
    }
    if (!S.consume_front("}"))
      return false;
    if (!S.empty())
      return false;
    // Found {vn}, {sn}, {an}, {v[n]}, {s[n]}, {a[n]}, {v[n:m]}, {s[n:m]}
    // or {a[n:m]}.
    Info.setAllowsRegister();
    Name = S.data() - 1;
    return true;
  }

  // \p Constraint will be left pointing at the last character of
  // the constraint.  In practice, it won't be changed unless the
  // constraint is longer than one character.
  std::string convertConstraint(const char *&Constraint) const override {

    StringRef S(Constraint);
    if (S == "DA" || S == "DB") {
      return std::string("^") + std::string(Constraint++, 2);
    }

    const char *Begin = Constraint;
    TargetInfo::ConstraintInfo Info("", "");
    if (validateAsmConstraint(Constraint, Info))
      return std::string(Begin).substr(0, Constraint - Begin + 1);

    Constraint = Begin;
    return std::string(1, *Constraint);
  }

  bool
  initFeatureMap(toolchain::StringMap<bool> &Features, DiagnosticsEngine &Diags,
                 StringRef CPU,
                 const std::vector<std::string> &FeatureVec) const override;

  toolchain::SmallVector<Builtin::InfosShard> getTargetBuiltins() const override;

  bool useFP16ConversionIntrinsics() const override { return false; }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  BuiltinVaListKind getBuiltinVaListKind() const override {
    return TargetInfo::CharPtrBuiltinVaList;
  }

  bool isValidCPUName(StringRef Name) const override {
    if (getTriple().isAMDGCN())
      return toolchain::AMDGPU::parseArchAMDGCN(Name) != toolchain::AMDGPU::GK_NONE;
    return toolchain::AMDGPU::parseArchR600(Name) != toolchain::AMDGPU::GK_NONE;
  }

  void fillValidCPUList(SmallVectorImpl<StringRef> &Values) const override;

  bool setCPU(const std::string &Name) override {
    if (getTriple().isAMDGCN()) {
      GPUKind = toolchain::AMDGPU::parseArchAMDGCN(Name);
      GPUFeatures = toolchain::AMDGPU::getArchAttrAMDGCN(GPUKind);
    } else {
      GPUKind = toolchain::AMDGPU::parseArchR600(Name);
      GPUFeatures = toolchain::AMDGPU::getArchAttrR600(GPUKind);
    }

    return GPUKind != toolchain::AMDGPU::GK_NONE;
  }

  void setSupportedOpenCLOpts() override {
    auto &Opts = getSupportedOpenCLOpts();
    Opts["cl_clang_storage_class_specifiers"] = true;
    Opts["__cl_clang_variadic_functions"] = true;
    Opts["__cl_clang_function_pointers"] = true;
    Opts["__cl_clang_non_portable_kernel_param_types"] = true;
    Opts["__cl_clang_bitfields"] = true;

    bool IsAMDGCN = isAMDGCN(getTriple());

    Opts["cl_khr_fp64"] = hasFP64();
    Opts["__opencl_c_fp64"] = hasFP64();

    if (IsAMDGCN || GPUKind >= toolchain::AMDGPU::GK_CEDAR) {
      Opts["cl_khr_byte_addressable_store"] = true;
      Opts["cl_khr_global_int32_base_atomics"] = true;
      Opts["cl_khr_global_int32_extended_atomics"] = true;
      Opts["cl_khr_local_int32_base_atomics"] = true;
      Opts["cl_khr_local_int32_extended_atomics"] = true;
    }

    if (IsAMDGCN) {
      Opts["cl_khr_fp16"] = true;
      Opts["cl_khr_int64_base_atomics"] = true;
      Opts["cl_khr_int64_extended_atomics"] = true;
      Opts["cl_khr_mipmap_image"] = true;
      Opts["cl_khr_mipmap_image_writes"] = true;
      Opts["cl_khr_subgroups"] = true;
      Opts["cl_amd_media_ops"] = true;
      Opts["cl_amd_media_ops2"] = true;

      Opts["__opencl_c_images"] = true;
      Opts["__opencl_c_3d_image_writes"] = true;
      Opts["cl_khr_3d_image_writes"] = true;

      Opts["__opencl_c_generic_address_space"] =
          GPUKind >= toolchain::AMDGPU::GK_GFX700;
    }
  }

  LangAS getOpenCLTypeAddrSpace(OpenCLTypeKind TK) const override {
    switch (TK) {
    case OCLTK_Image:
      return LangAS::opencl_constant;

    case OCLTK_ClkEvent:
    case OCLTK_Queue:
    case OCLTK_ReserveID:
      return LangAS::opencl_global;

    default:
      return TargetInfo::getOpenCLTypeAddrSpace(TK);
    }
  }

  LangAS getOpenCLBuiltinAddressSpace(unsigned AS) const override {
    switch (AS) {
    case 0:
      return LangAS::opencl_generic;
    case 1:
      return LangAS::opencl_global;
    case 3:
      return LangAS::opencl_local;
    case 4:
      return LangAS::opencl_constant;
    case 5:
      return LangAS::opencl_private;
    default:
      return getLangASFromTargetAS(AS);
    }
  }

  LangAS getCUDABuiltinAddressSpace(unsigned AS) const override {
    switch (AS) {
    case 0:
      return LangAS::Default;
    case 1:
      return LangAS::cuda_device;
    case 3:
      return LangAS::cuda_shared;
    case 4:
      return LangAS::cuda_constant;
    default:
      return getLangASFromTargetAS(AS);
    }
  }

  std::optional<LangAS> getConstantAddressSpace() const override {
    return getLangASFromTargetAS(toolchain::AMDGPUAS::CONSTANT_ADDRESS);
  }

  const toolchain::omp::GV &getGridValue() const override {
    switch (WavefrontSize) {
    case 32:
      return toolchain::omp::getAMDGPUGridValues<32>();
    case 64:
      return toolchain::omp::getAMDGPUGridValues<64>();
    default:
      toolchain_unreachable("getGridValue not implemented for this wavesize");
    }
  }

  /// \returns Target specific vtbl ptr address space.
  unsigned getVtblPtrAddressSpace() const override {
    return static_cast<unsigned>(toolchain::AMDGPUAS::CONSTANT_ADDRESS);
  }

  /// \returns If a target requires an address within a target specific address
  /// space \p AddressSpace to be converted in order to be used, then return the
  /// corresponding target specific DWARF address space.
  ///
  /// \returns Otherwise return std::nullopt and no conversion will be emitted
  /// in the DWARF.
  std::optional<unsigned>
  getDWARFAddressSpace(unsigned AddressSpace) const override {
    const unsigned DWARF_Private = 1;
    const unsigned DWARF_Local = 2;
    if (AddressSpace == toolchain::AMDGPUAS::PRIVATE_ADDRESS) {
      return DWARF_Private;
    } else if (AddressSpace == toolchain::AMDGPUAS::LOCAL_ADDRESS) {
      return DWARF_Local;
    } else {
      return std::nullopt;
    }
  }

  CallingConvCheckResult checkCallingConvention(CallingConv CC) const override {
    switch (CC) {
    default:
      return CCCR_Warning;
    case CC_C:
    case CC_DeviceKernel:
      return CCCR_OK;
    }
  }

  // In amdgcn target the null pointer in global, constant, and generic
  // address space has value 0 but in private and local address space has
  // value ~0.
  uint64_t getNullPointerValue(LangAS AS) const override {
    // FIXME: Also should handle region.
    return (AS == LangAS::opencl_local || AS == LangAS::opencl_private ||
            AS == LangAS::sycl_local || AS == LangAS::sycl_private)
               ? ~0
               : 0;
  }

  void setAuxTarget(const TargetInfo *Aux) override;

  bool hasBitIntType() const override { return true; }

  // Record offload arch features since they are needed for defining the
  // pre-defined macros.
  bool handleTargetFeatures(std::vector<std::string> &Features,
                            DiagnosticsEngine &Diags) override {
    HasFullBFloat16 = true;
    auto TargetIDFeatures =
        getAllPossibleTargetIDFeatures(getTriple(), getArchNameAMDGCN(GPUKind));
    for (const auto &F : Features) {
      assert(F.front() == '+' || F.front() == '-');
      if (F == "+wavefrontsize64")
        WavefrontSize = 64;
      else if (F == "+cumode")
        CUMode = true;
      else if (F == "-cumode")
        CUMode = false;
      else if (F == "+image-insts")
        HasImage = true;
      bool IsOn = F.front() == '+';
      StringRef Name = StringRef(F).drop_front();
      if (!toolchain::is_contained(TargetIDFeatures, Name))
        continue;
      assert(!OffloadArchFeatures.contains(Name));
      OffloadArchFeatures[Name] = IsOn;
    }
    return true;
  }

  std::optional<std::string> getTargetID() const override {
    if (!isAMDGCN(getTriple()))
      return std::nullopt;
    // When -target-cpu is not set, we assume generic code that it is valid
    // for all GPU and use an empty string as target ID to represent that.
    if (GPUKind == toolchain::AMDGPU::GK_NONE)
      return std::string("");
    return getCanonicalTargetID(getArchNameAMDGCN(GPUKind),
                                OffloadArchFeatures);
  }

  bool hasHIPImageSupport() const override { return HasImage; }

  std::pair<unsigned, unsigned> hardwareInterferenceSizes() const override {
    // This is imprecise as the value can vary between 64, 128 (even 256!) bytes
    // depending on the level of cache and the target architecture. We select
    // the size that corresponds to the largest L1 cache line for all
    // architectures.
    return std::make_pair(128, 128);
  }
};

} // namespace targets
} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_BASIC_TARGETS_AMDGPU_H
