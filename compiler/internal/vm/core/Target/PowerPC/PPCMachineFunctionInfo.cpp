//===-- PPCMachineFunctionInfo.cpp - Private data used for PowerPC --------===//
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

#include "PPCMachineFunctionInfo.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/BinaryFormat/XCOFF.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;
static cl::opt<bool> PPCDisableNonVolatileCR(
    "ppc-disable-non-volatile-cr",
    cl::desc("Disable the use of non-volatile CR register fields"),
    cl::init(false), cl::Hidden);

void PPCFunctionInfo::anchor() {}
PPCFunctionInfo::PPCFunctionInfo(const Function &F,
                                 const TargetSubtargetInfo *STI)
    : DisableNonVolatileCR(PPCDisableNonVolatileCR) {}

MachineFunctionInfo *
PPCFunctionInfo::clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
                       const DenseMap<MachineBasicBlock *, MachineBasicBlock *>
                           &Src2DstMBB) const {
  return DestMF.cloneInfo<PPCFunctionInfo>(*this);
}

MCSymbol *PPCFunctionInfo::getPICOffsetSymbol(MachineFunction &MF) const {
  const DataLayout &DL = MF.getDataLayout();
  return MF.getContext().getOrCreateSymbol(Twine(DL.getPrivateGlobalPrefix()) +
                                           Twine(MF.getFunctionNumber()) +
                                           "$poff");
}

MCSymbol *PPCFunctionInfo::getGlobalEPSymbol(MachineFunction &MF) const {
  const DataLayout &DL = MF.getDataLayout();
  return MF.getContext().getOrCreateSymbol(Twine(DL.getPrivateGlobalPrefix()) +
                                           "func_gep" +
                                           Twine(MF.getFunctionNumber()));
}

MCSymbol *PPCFunctionInfo::getLocalEPSymbol(MachineFunction &MF) const {
  const DataLayout &DL = MF.getDataLayout();
  return MF.getContext().getOrCreateSymbol(Twine(DL.getPrivateGlobalPrefix()) +
                                           "func_lep" +
                                           Twine(MF.getFunctionNumber()));
}

MCSymbol *PPCFunctionInfo::getTOCOffsetSymbol(MachineFunction &MF) const {
  const DataLayout &DL = MF.getDataLayout();
  return MF.getContext().getOrCreateSymbol(Twine(DL.getPrivateGlobalPrefix()) +
                                           "func_toc" +
                                           Twine(MF.getFunctionNumber()));
}

bool PPCFunctionInfo::isLiveInSExt(Register VReg) const {
  for (const std::pair<Register, ISD::ArgFlagsTy> &LiveIn : LiveInAttrs)
    if (LiveIn.first == VReg)
      return LiveIn.second.isSExt();
  return false;
}

bool PPCFunctionInfo::isLiveInZExt(Register VReg) const {
  for (const std::pair<Register, ISD::ArgFlagsTy> &LiveIn : LiveInAttrs)
    if (LiveIn.first == VReg)
      return LiveIn.second.isZExt();
  return false;
}

void PPCFunctionInfo::appendParameterType(ParamType Type) {

  ParamtersType.push_back(Type);
  switch (Type) {
  case FixedType:
    ++FixedParmsNum;
    return;
  case ShortFloatingPoint:
  case LongFloatingPoint:
    ++FloatingParmsNum;
    return;
  case VectorChar:
  case VectorShort:
  case VectorInt:
  case VectorFloat:
    ++VectorParmsNum;
    return;
  }
  llvm_unreachable("Error ParamType type.");
}

uint32_t PPCFunctionInfo::getVecExtParmsType() const {

  uint32_t VectExtParamInfo = 0;
  unsigned ShiftBits = 32 - XCOFF::TracebackTable::WidthOfParamType;
  int Bits = 0;

  if (!hasVectorParms())
    return 0;

  for (const auto &Elt : ParamtersType) {
    switch (Elt) {
    case VectorChar:
      VectExtParamInfo <<= XCOFF::TracebackTable::WidthOfParamType;
      VectExtParamInfo |=
          XCOFF::TracebackTable::ParmTypeIsVectorCharBit >> ShiftBits;
      Bits += XCOFF::TracebackTable::WidthOfParamType;
      break;
    case VectorShort:
      VectExtParamInfo <<= XCOFF::TracebackTable::WidthOfParamType;
      VectExtParamInfo |=
          XCOFF::TracebackTable::ParmTypeIsVectorShortBit >> ShiftBits;
      Bits += XCOFF::TracebackTable::WidthOfParamType;
      break;
    case VectorInt:
      VectExtParamInfo <<= XCOFF::TracebackTable::WidthOfParamType;
      VectExtParamInfo |=
          XCOFF::TracebackTable::ParmTypeIsVectorIntBit >> ShiftBits;
      Bits += XCOFF::TracebackTable::WidthOfParamType;
      break;
    case VectorFloat:
      VectExtParamInfo <<= XCOFF::TracebackTable::WidthOfParamType;
      VectExtParamInfo |=
          XCOFF::TracebackTable::ParmTypeIsVectorFloatBit >> ShiftBits;
      Bits += XCOFF::TracebackTable::WidthOfParamType;
      break;
    default:
      break;
    }

    // There are only 32bits in the VectExtParamInfo.
    if (Bits >= 32)
      break;
  }
  return Bits < 32 ? VectExtParamInfo << (32 - Bits) : VectExtParamInfo;
}

uint32_t PPCFunctionInfo::getParmsType() const {
  uint32_t ParamsTypeInfo = 0;
  unsigned ShiftBits = 32 - XCOFF::TracebackTable::WidthOfParamType;

  int Bits = 0;
  for (const auto &Elt : ParamtersType) {

    if (Bits > 31 || (Bits > 30 && (Elt != FixedType || hasVectorParms())))
      break;

    switch (Elt) {
    case FixedType:
      if (hasVectorParms()) {
        //'00'  ==> fixed parameter if HasVectorParms is true.
        ParamsTypeInfo <<= XCOFF::TracebackTable::WidthOfParamType;
        ParamsTypeInfo |=
            XCOFF::TracebackTable::ParmTypeIsFixedBits >> ShiftBits;
        Bits += XCOFF::TracebackTable::WidthOfParamType;
      } else {
        //'0'  ==> fixed parameter if HasVectorParms is false.
        ParamsTypeInfo <<= 1;
        ++Bits;
      }
      break;
    case ShortFloatingPoint:
      // '10'b => floating point short parameter.
      ParamsTypeInfo <<= XCOFF::TracebackTable::WidthOfParamType;
      ParamsTypeInfo |=
          XCOFF::TracebackTable::ParmTypeIsFloatingBits >> ShiftBits;
      Bits += XCOFF::TracebackTable::WidthOfParamType;
      break;
    case LongFloatingPoint:
      // '11'b => floating point long parameter.
      ParamsTypeInfo <<= XCOFF::TracebackTable::WidthOfParamType;
      ParamsTypeInfo |=
          XCOFF::TracebackTable::ParmTypeIsDoubleBits >> ShiftBits;
      Bits += XCOFF::TracebackTable::WidthOfParamType;
      break;
    case VectorChar:
    case VectorShort:
    case VectorInt:
    case VectorFloat:
      //	'01' ==> vector parameter
      ParamsTypeInfo <<= XCOFF::TracebackTable::WidthOfParamType;
      ParamsTypeInfo |=
          XCOFF::TracebackTable::ParmTypeIsVectorBits >> ShiftBits;
      Bits += XCOFF::TracebackTable::WidthOfParamType;
      break;
    }
  }

  return Bits < 32 ? ParamsTypeInfo << (32 - Bits) : ParamsTypeInfo;
}
