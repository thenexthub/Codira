// WebAssemblyMachineFunctionInfo.h-WebAssembly machine function info-*- C++ -*-
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
/// This file declares WebAssembly-specific per-machine-function
/// information.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYMACHINEFUNCTIONINFO_H

#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "vm/core/CodeGen/MIRYamlMapping.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/MC/MCSymbolWasm.h"

namespace vm::core {
class WebAssemblyTargetLowering;

struct WasmEHFuncInfo;

namespace yaml {
struct WebAssemblyFunctionInfo;
}

/// This class is derived from MachineFunctionInfo and contains private
/// WebAssembly-specific information for each MachineFunction.
class WebAssemblyFunctionInfo final : public MachineFunctionInfo {
  std::vector<MVT> Params;
  std::vector<MVT> Results;
  std::vector<MVT> Locals;

  /// A mapping from CodeGen vreg index to WebAssembly register number.
  std::vector<unsigned> WARegs;

  /// A mapping from CodeGen vreg index to a boolean value indicating whether
  /// the given register is considered to be "stackified", meaning it has been
  /// determined or made to meet the stack requirements:
  ///   - single use (per path)
  ///   - single def (per path)
  ///   - defined and used in LIFO order with other stack registers
  BitVector VRegStackified;

  // A virtual register holding the pointer to the vararg buffer for vararg
  // functions. It is created and set in TLI::LowerFormalArguments and read by
  // TLI::LowerVASTART
  unsigned VarargVreg = -1U;

  // A virtual register holding the base pointer for functions that have
  // overaligned values on the user stack.
  unsigned BasePtrVreg = -1U;
  // A virtual register holding the frame base. This is either FP or SP
  // after it has been replaced by a vreg
  unsigned FrameBaseVreg = -1U;
  // The local holding the frame base. This is either FP or SP
  // after WebAssemblyExplicitLocals
  unsigned FrameBaseLocal = -1U;

  // Function properties.
  bool CFGStackified = false;

public:
  explicit WebAssemblyFunctionInfo(const Function &F,
                                   const TargetSubtargetInfo *STI) {}
  ~WebAssemblyFunctionInfo() override;

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  void initializeBaseYamlFields(MachineFunction &MF,
                                const yaml::WebAssemblyFunctionInfo &YamlMFI);

  void addParam(MVT VT) { Params.push_back(VT); }
  const std::vector<MVT> &getParams() const { return Params; }

  void addResult(MVT VT) { Results.push_back(VT); }
  const std::vector<MVT> &getResults() const { return Results; }

  void clearParamsAndResults() {
    Params.clear();
    Results.clear();
  }

  void setNumLocals(size_t NumLocals) { Locals.resize(NumLocals, MVT::i32); }
  void setLocal(size_t i, MVT VT) { Locals[i] = VT; }
  void addLocal(MVT VT) { Locals.push_back(VT); }
  const std::vector<MVT> &getLocals() const { return Locals; }

  unsigned getVarargBufferVreg() const {
    assert(VarargVreg != -1U && "Vararg vreg hasn't been set");
    return VarargVreg;
  }
  void setVarargBufferVreg(unsigned Reg) { VarargVreg = Reg; }

  unsigned getBasePointerVreg() const {
    assert(BasePtrVreg != -1U && "Base ptr vreg hasn't been set");
    return BasePtrVreg;
  }
  void setFrameBaseVreg(unsigned Reg) { FrameBaseVreg = Reg; }
  unsigned getFrameBaseVreg() const {
    assert(FrameBaseVreg != -1U && "Frame base vreg hasn't been set");
    return FrameBaseVreg;
  }
  void clearFrameBaseVreg() { FrameBaseVreg = -1U; }
  // Return true if the frame base physreg has been replaced by a virtual reg.
  bool isFrameBaseVirtual() const { return FrameBaseVreg != -1U; }
  void setFrameBaseLocal(unsigned Local) { FrameBaseLocal = Local; }
  unsigned getFrameBaseLocal() const {
    assert(FrameBaseLocal != -1U && "Frame base local hasn't been set");
    return FrameBaseLocal;
  }
  void setBasePointerVreg(unsigned Reg) { BasePtrVreg = Reg; }

  void stackifyVReg(MachineRegisterInfo &MRI, Register VReg) {
    assert(MRI.getUniqueVRegDef(VReg));
    auto I = VReg.virtRegIndex();
    if (I >= VRegStackified.size())
      VRegStackified.resize(I + 1);
    VRegStackified.set(I);
  }
  void unstackifyVReg(Register VReg) {
    auto I = VReg.virtRegIndex();
    if (I < VRegStackified.size())
      VRegStackified.reset(I);
  }
  bool isVRegStackified(Register VReg) const {
    auto I = VReg.virtRegIndex();
    if (I >= VRegStackified.size())
      return false;
    return VRegStackified.test(I);
  }

  void initWARegs(MachineRegisterInfo &MRI);
  void setWAReg(Register VReg, unsigned WAReg) {
    assert(WAReg != WebAssembly::UnusedReg);
    auto I = VReg.virtRegIndex();
    assert(I < WARegs.size());
    WARegs[I] = WAReg;
  }
  unsigned getWAReg(Register VReg) const {
    auto I = VReg.virtRegIndex();
    assert(I < WARegs.size());
    return WARegs[I];
  }

  bool isCFGStackified() const { return CFGStackified; }
  void setCFGStackified(bool Value = true) { CFGStackified = Value; }
};

void computeLegalValueVTs(const WebAssemblyTargetLowering &TLI,
                          LLVMContext &Ctx, const DataLayout &DL, Type *Ty,
                          SmallVectorImpl<MVT> &ValueVTs);

void computeLegalValueVTs(const Function &F, const TargetMachine &TM, Type *Ty,
                          SmallVectorImpl<MVT> &ValueVTs);

// Compute the signature for a given FunctionType (Ty). Note that it's not the
// signature for ContextFunc (ContextFunc is just used to get varous context)
void computeSignatureVTs(const FunctionType *Ty, const Function *TargetFunc,
                         const Function &ContextFunc, const TargetMachine &TM,
                         SmallVectorImpl<MVT> &Params,
                         SmallVectorImpl<MVT> &Results);

void valTypesFromMVTs(ArrayRef<MVT> In, SmallVectorImpl<wasm::ValType> &Out);

wasm::WasmSignature *signatureFromMVTs(MCContext &Ctx,
                                       const SmallVectorImpl<MVT> &Results,
                                       const SmallVectorImpl<MVT> &Params);

namespace yaml {

using BBNumberMap = DenseMap<int, int>;

struct WebAssemblyFunctionInfo final : public yaml::MachineFunctionInfo {
  std::vector<FlowStringValue> Params;
  std::vector<FlowStringValue> Results;
  bool CFGStackified = false;
  // The same as WasmEHFuncInfo's SrcToUnwindDest, but stored in the mapping of
  // BB numbers
  BBNumberMap SrcToUnwindDest;

  WebAssemblyFunctionInfo() = default;
  WebAssemblyFunctionInfo(const toolchain::MachineFunction &MF,
                          const toolchain::WebAssemblyFunctionInfo &MFI);

  void mappingImpl(yaml::IO &YamlIO) override;
  ~WebAssemblyFunctionInfo() override = default;
};

template <> struct MappingTraits<WebAssemblyFunctionInfo> {
  static void mapping(IO &YamlIO, WebAssemblyFunctionInfo &MFI) {
    YamlIO.mapOptional("params", MFI.Params, std::vector<FlowStringValue>());
    YamlIO.mapOptional("results", MFI.Results, std::vector<FlowStringValue>());
    YamlIO.mapOptional("isCFGStackified", MFI.CFGStackified, false);
    YamlIO.mapOptional("wasmEHFuncInfo", MFI.SrcToUnwindDest);
  }
};

template <> struct CustomMappingTraits<BBNumberMap> {
  static void inputOne(IO &YamlIO, StringRef Key,
                       BBNumberMap &SrcToUnwindDest) {
    YamlIO.mapRequired(Key, SrcToUnwindDest[std::atoi(Key.str().c_str())]);
  }

  static void output(IO &YamlIO, BBNumberMap &SrcToUnwindDest) {
    for (auto [Src, Dest] : SrcToUnwindDest)
      YamlIO.mapRequired(std::to_string(Src), Dest);
  }
};

} // end namespace yaml

} // end namespace vm::core

#endif
