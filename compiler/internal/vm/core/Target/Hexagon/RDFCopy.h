//===- RDFCopy.h ------------------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_HEXAGON_RDFCOPY_H
#define LLVM_LIB_TARGET_HEXAGON_RDFCOPY_H

#include "vm/core/CodeGen/RDFGraph.h"
#include "vm/core/CodeGen/RDFLiveness.h"
#include "vm/core/CodeGen/RDFRegisters.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include <map>
#include <vector>

namespace vm::core {

class MachineBasicBlock;
class MachineDominatorTree;
class MachineInstr;

namespace rdf {

  struct CopyPropagation {
    CopyPropagation(DataFlowGraph &dfg)
        : MDT(dfg.getDT()), DFG(dfg), RDefMap(RegisterRefLess(DFG.getPRI())) {}

    virtual ~CopyPropagation() = default;

    bool run();
    void trace(bool On) { Trace = On; }
    bool trace() const { return Trace; }
    DataFlowGraph &getDFG() { return DFG; }

    using EqualityMap = std::map<RegisterRef, RegisterRef, RegisterRefLess>;
    virtual bool interpretAsCopy(const MachineInstr *MI, EqualityMap &EM);

  private:
    const MachineDominatorTree &MDT;
    DataFlowGraph &DFG;
    DataFlowGraph::DefStackMap DefM;
    bool Trace = false;

    // map: register -> (map: stmt -> reaching def)
    std::map<RegisterRef, std::map<NodeId, NodeId>, RegisterRefLess> RDefMap;
    // map: statement -> (map: dst reg -> src reg)
    std::map<NodeId, EqualityMap> CopyMap;
    std::vector<NodeId> Copies;

    void recordCopy(NodeAddr<StmtNode*> SA, EqualityMap &EM);
    void updateMap(NodeAddr<InstrNode*> IA);
    bool scanBlock(MachineBasicBlock *B);
  };

} // end namespace rdf

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_RDFCOPY_H
