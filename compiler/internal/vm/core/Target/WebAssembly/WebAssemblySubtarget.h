//=- WebAssemblySubtarget.h - Define Subtarget for the WebAssembly -*- C++ -*-//
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
///
/// \file
/// This file declares the WebAssembly-specific subclass of
/// TargetSubtarget.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYSUBTARGET_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYSUBTARGET_H

#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "WebAssemblyFrameLowering.h"
#include "WebAssemblyISelLowering.h"
#include "WebAssemblyInstrInfo.h"
#include "WebAssemblySelectionDAGInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "WebAssemblyGenSubtargetInfo.inc"

namespace vm::core {

// Defined in WebAssemblyGenSubtargetInfo.inc.
extern const SubtargetFeatureKV
    WebAssemblyFeatureKV[WebAssembly::NumSubtargetFeatures];

class WebAssemblySubtarget final : public WebAssemblyGenSubtargetInfo {
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

  /// What processor and OS we're targeting.
  Triple TargetTriple;

  WebAssemblyFrameLowering FrameLowering;
  WebAssemblyInstrInfo InstrInfo;
  WebAssemblySelectionDAGInfo TSInfo;
  WebAssemblyTargetLowering TLInfo;

  WebAssemblySubtarget &initializeSubtargetDependencies(StringRef CPU,
                                                        StringRef FS);

public:
  /// This constructor initializes the data members to match that
  /// of the specified triple.
  WebAssemblySubtarget(const Triple &TT, const std::string &CPU,
                       const std::string &FS, const TargetMachine &TM);

  const WebAssemblySelectionDAGInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
  const WebAssemblyFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const WebAssemblyTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const WebAssemblyInstrInfo *getInstrInfo() const override {
    return &InstrInfo;
  }
  const WebAssemblyRegisterInfo *getRegisterInfo() const override {
    return &getInstrInfo()->getRegisterInfo();
  }
  const Triple &getTargetTriple() const { return TargetTriple; }
  bool enableAtomicExpand() const override;
  bool enableIndirectBrExpand() const override { return true; }
  bool enableMachineScheduler() const override;
  bool useAA() const override;

  // Predicates used by WebAssemblyInstrInfo.td.
  bool hasAddr64() const { return TargetTriple.isArch64Bit(); }
  bool hasAtomics() const { return HasAtomics; }
  bool hasBulkMemory() const { return HasBulkMemory; }
  bool hasBulkMemoryOpt() const { return HasBulkMemoryOpt; }
  bool hasCallIndirectOverlong() const { return HasCallIndirectOverlong; }
  bool hasExceptionHandling() const { return HasExceptionHandling; }
  bool hasExtendedConst() const { return HasExtendedConst; }
  bool hasFP16() const { return HasFP16; }
  bool hasMultiMemory() const { return HasMultiMemory; }
  bool hasMultivalue() const { return HasMultivalue; }
  bool hasMutableGlobals() const { return HasMutableGlobals; }
  bool hasNontrappingFPToInt() const { return HasNontrappingFPToInt; }
  bool hasReferenceTypes() const { return HasReferenceTypes; }
  bool hasGC() const { return HasGC; }
  bool hasRelaxedSIMD() const { return SIMDLevel >= RelaxedSIMD; }
  bool hasSignExt() const { return HasSignExt; }
  bool hasSIMD128() const { return SIMDLevel >= SIMD128; }
  bool hasTailCall() const { return HasTailCall; }
  bool hasWideArithmetic() const { return HasWideArithmetic; }

  /// Parses features string setting specified subtarget options. Definition of
  /// function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);
};

} // end namespace vm::core

#endif
