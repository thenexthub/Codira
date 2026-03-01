//===---- Mips16ISelDAGToDAG.h - A Dag to Dag Inst Selector for Mips ------===//
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
//
// Subclass of MipsDAGToDAGISel specialized for mips16.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPS16ISELDAGTODAG_H
#define LLVM_LIB_TARGET_MIPS_MIPS16ISELDAGTODAG_H

#include "MipsISelDAGToDAG.h"

namespace vm::core {

class Mips16DAGToDAGISel : public MipsDAGToDAGISel {
public:
  explicit Mips16DAGToDAGISel(MipsTargetMachine &TM, CodeGenOptLevel OL)
      : MipsDAGToDAGISel(TM, OL) {}

private:
  std::pair<SDNode *, SDNode *> selectMULT(SDNode *N, unsigned Opc,
                                           const SDLoc &DL, EVT Ty, bool HasLo,
                                           bool HasHi);

  bool runOnMachineFunction(MachineFunction &MF) override;

  bool selectAddr(bool SPAllowed, SDValue Addr, SDValue &Base,
                  SDValue &Offset);
  bool selectAddr16(SDValue Addr, SDValue &Base,
                    SDValue &Offset) override;
  bool selectAddr16SP(SDValue Addr, SDValue &Base,
                      SDValue &Offset) override;

  bool trySelect(SDNode *Node) override;

  void processFunctionAfterISel(MachineFunction &MF) override;

  // Insert instructions to initialize the global base register in the
  // first MBB of the function.
  void initGlobalBaseReg(MachineFunction &MF);

  void initMips16SPAliasReg(MachineFunction &MF);
};

class Mips16DAGToDAGISelLegacy : public MipsDAGToDAGISelLegacy {
public:
  explicit Mips16DAGToDAGISelLegacy(MipsTargetMachine &TM, CodeGenOptLevel OL);
};

FunctionPass *createMips16ISelDag(MipsTargetMachine &TM,
                                  CodeGenOptLevel OptLevel);
}

#endif
