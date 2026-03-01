//===-- AMDGPUTargetStreamer.h - AMDGPU Target Streamer --------*- C++ -*--===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#ifndef LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUTARGETSTREAMER_H
#define LLVM_LIB_TARGET_AMDGPU_MCTARGETDESC_AMDGPUTARGETSTREAMER_H

#include "Utils/AMDGPUBaseInfo.h"
#include "Utils/AMDGPUPALMetadata.h"
#include "vm/core/MC/MCStreamer.h"

namespace vm::core {

class MCELFStreamer;
class MCSymbol;
class formatted_raw_ostream;

namespace AMDGPU {

struct AMDGPUMCKernelCodeT;
struct MCKernelDescriptor;
namespace HSAMD {
struct Metadata;
}
} // namespace AMDGPU

class AMDGPUTargetStreamer : public MCTargetStreamer {
  AMDGPUPALMetadata PALMetadata;

protected:
  // TODO: Move HSAMetadataStream to AMDGPUTargetStreamer.
  std::optional<AMDGPU::IsaInfo::AMDGPUTargetID> TargetID;
  unsigned CodeObjectVersion;

  MCContext &getContext() const { return Streamer.getContext(); }

public:
  AMDGPUTargetStreamer(MCStreamer &S)
      : MCTargetStreamer(S),
        // Assume the default COV for now, EmitDirectiveAMDHSACodeObjectVersion
        // will update this if it is encountered.
        CodeObjectVersion(AMDGPU::getDefaultAMDHSACodeObjectVersion()) {}

  AMDGPUPALMetadata *getPALMetadata() { return &PALMetadata; }

  virtual void EmitDirectiveAMDGCNTarget(){};

  virtual void EmitDirectiveAMDHSACodeObjectVersion(unsigned COV) {
    CodeObjectVersion = COV;
  }

  virtual void EmitAMDKernelCodeT(AMDGPU::AMDGPUMCKernelCodeT &Header) {};

  virtual void EmitAMDGPUSymbolType(StringRef SymbolName, unsigned Type){};

  virtual void emitAMDGPULDS(MCSymbol *Symbol, unsigned Size, Align Alignment) {
  }

  virtual void EmitMCResourceInfo(
      const MCSymbol *NumVGPR, const MCSymbol *NumAGPR,
      const MCSymbol *NumExplicitSGPR, const MCSymbol *NumNamedBarrier,
      const MCSymbol *PrivateSegmentSize, const MCSymbol *UsesVCC,
      const MCSymbol *UsesFlatScratch, const MCSymbol *HasDynamicallySizedStack,
      const MCSymbol *HasRecursion, const MCSymbol *HasIndirectCall) {};

  virtual void EmitMCResourceMaximums(const MCSymbol *MaxVGPR,
                                      const MCSymbol *MaxAGPR,
                                      const MCSymbol *MaxSGPR,
                                      const MCSymbol *MaxNamedBarrier) {};

  /// \returns True on success, false on failure.
  virtual bool EmitISAVersion() { return true; }

  /// \returns True on success, false on failure.
  virtual bool EmitHSAMetadataV3(StringRef HSAMetadataString);

  /// Emit HSA Metadata
  ///
  /// When \p Strict is true, known metadata elements must already be
  /// well-typed. When \p Strict is false, known types are inferred and
  /// the \p HSAMetadata structure is updated with the correct types.
  ///
  /// \returns True on success, false on failure.
  virtual bool EmitHSAMetadata(msgpack::Document &HSAMetadata, bool Strict) {
    return true;
  }

  /// \returns True on success, false on failure.
  virtual bool EmitHSAMetadata(const AMDGPU::HSAMD::Metadata &HSAMetadata) {
    return true;
  }

  /// \returns True on success, false on failure.
  virtual bool EmitCodeEnd(const MCSubtargetInfo &STI) { return true; }

  virtual void
  EmitAmdhsaKernelDescriptor(const MCSubtargetInfo &STI, StringRef KernelName,
                             const AMDGPU::MCKernelDescriptor &KernelDescriptor,
                             const MCExpr *NextVGPR, const MCExpr *NextSGPR,
                             const MCExpr *ReserveVCC,
                             const MCExpr *ReserveFlatScr) {}

  static StringRef getArchNameFromElfMach(unsigned ElfMach);
  static unsigned getElfMach(StringRef GPU);

  const std::optional<AMDGPU::IsaInfo::AMDGPUTargetID> &getTargetID() const {
    return TargetID;
  }
  std::optional<AMDGPU::IsaInfo::AMDGPUTargetID> &getTargetID() {
    return TargetID;
  }
  void initializeTargetID(const MCSubtargetInfo &STI) {
    assert(TargetID == std::nullopt && "TargetID can only be initialized once");
    TargetID.emplace(STI);
  }
  void initializeTargetID(const MCSubtargetInfo &STI, StringRef FeatureString) {
    initializeTargetID(STI);

    assert(getTargetID() != std::nullopt && "TargetID is None");
    getTargetID()->setTargetIDFromFeaturesString(FeatureString);
  }
};

class AMDGPUTargetAsmStreamer final : public AMDGPUTargetStreamer {
  formatted_raw_ostream &OS;
public:
  AMDGPUTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);

  void finish() override;

  void EmitDirectiveAMDGCNTarget() override;

  void EmitDirectiveAMDHSACodeObjectVersion(unsigned COV) override;

  void EmitAMDKernelCodeT(AMDGPU::AMDGPUMCKernelCodeT &Header) override;

  void EmitAMDGPUSymbolType(StringRef SymbolName, unsigned Type) override;

  void emitAMDGPULDS(MCSymbol *Sym, unsigned Size, Align Alignment) override;

  void EmitMCResourceInfo(
      const MCSymbol *NumVGPR, const MCSymbol *NumAGPR,
      const MCSymbol *NumExplicitSGPR, const MCSymbol *NumNamedBarrier,
      const MCSymbol *PrivateSegmentSize, const MCSymbol *UsesVCC,
      const MCSymbol *UsesFlatScratch, const MCSymbol *HasDynamicallySizedStack,
      const MCSymbol *HasRecursion, const MCSymbol *HasIndirectCall) override;

  void EmitMCResourceMaximums(const MCSymbol *MaxVGPR, const MCSymbol *MaxAGPR,
                              const MCSymbol *MaxSGPR,
                              const MCSymbol *MaxNamedBarrier) override;

  /// \returns True on success, false on failure.
  bool EmitISAVersion() override;

  /// \returns True on success, false on failure.
  bool EmitHSAMetadata(msgpack::Document &HSAMetadata, bool Strict) override;

  /// \returns True on success, false on failure.
  bool EmitCodeEnd(const MCSubtargetInfo &STI) override;

  void
  EmitAmdhsaKernelDescriptor(const MCSubtargetInfo &STI, StringRef KernelName,
                             const AMDGPU::MCKernelDescriptor &KernelDescriptor,
                             const MCExpr *NextVGPR, const MCExpr *NextSGPR,
                             const MCExpr *ReserveVCC,
                             const MCExpr *ReserveFlatScr) override;
};

class AMDGPUTargetELFStreamer final : public AMDGPUTargetStreamer {
  const MCSubtargetInfo &STI;
  MCStreamer &Streamer;

  void EmitNote(StringRef Name, const MCExpr *DescSize, unsigned NoteType,
                function_ref<void(MCELFStreamer &)> EmitDesc);

  unsigned getEFlags();

  unsigned getEFlagsR600();
  unsigned getEFlagsAMDGCN();

  unsigned getEFlagsUnknownOS();
  unsigned getEFlagsAMDHSA();
  unsigned getEFlagsAMDPAL();
  unsigned getEFlagsMesa3D();

  unsigned getEFlagsV3();
  unsigned getEFlagsV4();
  unsigned getEFlagsV6();

public:
  AMDGPUTargetELFStreamer(MCStreamer &S, const MCSubtargetInfo &STI);

  MCELFStreamer &getStreamer();

  void finish() override;

  void EmitDirectiveAMDGCNTarget() override;

  void EmitAMDKernelCodeT(AMDGPU::AMDGPUMCKernelCodeT &Header) override;

  void EmitAMDGPUSymbolType(StringRef SymbolName, unsigned Type) override;

  void emitAMDGPULDS(MCSymbol *Sym, unsigned Size, Align Alignment) override;

  /// \returns True on success, false on failure.
  bool EmitISAVersion() override;

  /// \returns True on success, false on failure.
  bool EmitHSAMetadata(msgpack::Document &HSAMetadata, bool Strict) override;

  /// \returns True on success, false on failure.
  bool EmitCodeEnd(const MCSubtargetInfo &STI) override;

  void
  EmitAmdhsaKernelDescriptor(const MCSubtargetInfo &STI, StringRef KernelName,
                             const AMDGPU::MCKernelDescriptor &KernelDescriptor,
                             const MCExpr *NextVGPR, const MCExpr *NextSGPR,
                             const MCExpr *ReserveVCC,
                             const MCExpr *ReserveFlatScr) override;
};
}
#endif
