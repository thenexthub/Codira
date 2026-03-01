//=- WebAssemblyMachineFunctionInfo.cpp - WebAssembly Machine Function Info -=//
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
/// This file implements WebAssembly-specific per-machine-function
/// information.
///
//===----------------------------------------------------------------------===//

#include "WebAssemblyMachineFunctionInfo.h"
#include "Utils/WebAssemblyTypeUtilities.h"
#include "WebAssemblyISelLowering.h"
#include "WebAssemblySubtarget.h"
#include "WebAssemblyUtilities.h"
#include "vm/core/CodeGen/Analysis.h"
#include "vm/core/CodeGen/WasmEHFuncInfo.h"
#include "vm/core/Target/TargetMachine.h"
using namespace vm::core;

WebAssemblyFunctionInfo::~WebAssemblyFunctionInfo() = default; // anchor.

MachineFunctionInfo *WebAssemblyFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  // TODO: Implement cloning for WasmEHFuncInfo. This will have invalid block
  // references.
  return DestMF.cloneInfo<WebAssemblyFunctionInfo>(*this);
}

void WebAssemblyFunctionInfo::initWARegs(MachineRegisterInfo &MRI) {
  assert(WARegs.empty());
  unsigned Reg = WebAssembly::UnusedReg;
  WARegs.resize(MRI.getNumVirtRegs(), Reg);
}

void toolchain::computeLegalValueVTs(const WebAssemblyTargetLowering &TLI,
                                LLVMContext &Ctx, const DataLayout &DL,
                                Type *Ty, SmallVectorImpl<MVT> &ValueVTs) {
  SmallVector<EVT, 4> VTs;
  ComputeValueVTs(TLI, DL, Ty, VTs);

  for (EVT VT : VTs) {
    unsigned NumRegs = TLI.getNumRegisters(Ctx, VT);
    MVT RegisterVT = TLI.getRegisterType(Ctx, VT);
    for (unsigned I = 0; I != NumRegs; ++I)
      ValueVTs.push_back(RegisterVT);
  }
}

void toolchain::computeLegalValueVTs(const Function &F, const TargetMachine &TM,
                                Type *Ty, SmallVectorImpl<MVT> &ValueVTs) {
  const DataLayout &DL(F.getDataLayout());
  const WebAssemblyTargetLowering &TLI =
      *TM.getSubtarget<WebAssemblySubtarget>(F).getTargetLowering();
  computeLegalValueVTs(TLI, F.getContext(), DL, Ty, ValueVTs);
}

void toolchain::computeSignatureVTs(const FunctionType *Ty,
                               const Function *TargetFunc,
                               const Function &ContextFunc,
                               const TargetMachine &TM,
                               SmallVectorImpl<MVT> &Params,
                               SmallVectorImpl<MVT> &Results) {
  computeLegalValueVTs(ContextFunc, TM, Ty->getReturnType(), Results);

  MVT PtrVT = MVT::getIntegerVT(TM.createDataLayout().getPointerSizeInBits());
  if (!WebAssembly::canLowerReturn(
          Results.size(),
          &TM.getSubtarget<WebAssemblySubtarget>(ContextFunc))) {
    // WebAssembly can't lower returns of multiple values without demoting to
    // sret unless multivalue is enabled (see
    // WebAssemblyTargetLowering::CanLowerReturn). So replace multiple return
    // values with a poitner parameter.
    Results.clear();
    Params.push_back(PtrVT);
  }

  for (auto *Param : Ty->params())
    computeLegalValueVTs(ContextFunc, TM, Param, Params);
  if (Ty->isVarArg())
    Params.push_back(PtrVT);

  // For swiftcc, emit additional swiftself and swifterror parameters
  // if there aren't. These additional parameters are also passed for caller.
  // They are necessary to match callee and caller signature for indirect
  // call.

  if (TargetFunc && TargetFunc->getCallingConv() == CallingConv::Swift) {
    MVT PtrVT = MVT::getIntegerVT(TM.createDataLayout().getPointerSizeInBits());
    bool HasSwiftErrorArg = false;
    bool HasSwiftSelfArg = false;
    for (const auto &Arg : TargetFunc->args()) {
      HasSwiftErrorArg |= Arg.hasAttribute(Attribute::SwiftError);
      HasSwiftSelfArg |= Arg.hasAttribute(Attribute::SwiftSelf);
    }
    if (!HasSwiftErrorArg)
      Params.push_back(PtrVT);
    if (!HasSwiftSelfArg)
      Params.push_back(PtrVT);
  }
}

void toolchain::valTypesFromMVTs(ArrayRef<MVT> In,
                            SmallVectorImpl<wasm::ValType> &Out) {
  for (MVT Ty : In)
    Out.push_back(WebAssembly::toValType(Ty));
}

wasm::WasmSignature *
toolchain::signatureFromMVTs(MCContext &Ctx, const SmallVectorImpl<MVT> &Results,
                        const SmallVectorImpl<MVT> &Params) {
  auto Sig = Ctx.createWasmSignature();
  valTypesFromMVTs(Results, Sig->Returns);
  valTypesFromMVTs(Params, Sig->Params);
  return Sig;
}

yaml::WebAssemblyFunctionInfo::WebAssemblyFunctionInfo(
    const toolchain::MachineFunction &MF, const toolchain::WebAssemblyFunctionInfo &MFI)
    : CFGStackified(MFI.isCFGStackified()) {
  for (auto VT : MFI.getParams())
    Params.push_back(EVT(VT).getEVTString());
  for (auto VT : MFI.getResults())
    Results.push_back(EVT(VT).getEVTString());

  //  MFI.getWasmEHFuncInfo() is non-null only for functions with the
  //  personality function.

  if (auto *EHInfo = MF.getWasmEHFuncInfo()) {
    // SrcToUnwindDest can contain stale mappings in case BBs are removed in
    // optimizations, in case, for example, they are unreachable. We should not
    // include their info.
    SmallPtrSet<const MachineBasicBlock *, 16> MBBs;
    for (const auto &MBB : MF)
      MBBs.insert(&MBB);
    for (auto KV : EHInfo->SrcToUnwindDest) {
      auto *SrcBB = cast<MachineBasicBlock *>(KV.first);
      auto *DestBB = cast<MachineBasicBlock *>(KV.second);
      if (MBBs.count(SrcBB) && MBBs.count(DestBB))
        SrcToUnwindDest[SrcBB->getNumber()] = DestBB->getNumber();
    }
  }
}

void yaml::WebAssemblyFunctionInfo::mappingImpl(yaml::IO &YamlIO) {
  MappingTraits<WebAssemblyFunctionInfo>::mapping(YamlIO, *this);
}

void WebAssemblyFunctionInfo::initializeBaseYamlFields(
    MachineFunction &MF, const yaml::WebAssemblyFunctionInfo &YamlMFI) {
  CFGStackified = YamlMFI.CFGStackified;
  for (auto VT : YamlMFI.Params)
    addParam(WebAssembly::parseMVT(VT.Value));
  for (auto VT : YamlMFI.Results)
    addResult(WebAssembly::parseMVT(VT.Value));

  // FIXME: WasmEHInfo is defined in the MachineFunction, but serialized
  // here. Either WasmEHInfo should be moved out of MachineFunction, or the
  // serialization handling should be moved to MachineFunction.
  if (WasmEHFuncInfo *WasmEHInfo = MF.getWasmEHFuncInfo()) {
    for (auto KV : YamlMFI.SrcToUnwindDest)
      WasmEHInfo->setUnwindDest(MF.getBlockNumbered(KV.first),
                                MF.getBlockNumbered(KV.second));
  }
}
