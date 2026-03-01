//===--- RDFDeadCode.h ----------------------------------------------------===//
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
// RDF-based generic dead code elimination.
//
// The main interface of this class are functions "collect" and "erase".
// This allows custom processing of the function being optimized by a
// particular consumer. The simplest way to use this class would be to
// instantiate an object, and then simply call "collect" and "erase",
// passing the result of "getDeadInstrs()" to it.
// A more complex scenario would be to call "collect" first, then visit
// all post-increment instructions to see if the address update is dead
// or not, and if it is, convert the instruction to a non-updating form.
// After that "erase" can be called with the set of nodes including both,
// dead defs from the updating instructions and the nodes corresponding
// to the dead instructions.

#ifndef RDF_DEADCODE_H
#define RDF_DEADCODE_H

#include "vm/core/CodeGen/RDFGraph.h"
#include "vm/core/CodeGen/RDFLiveness.h"
#include "vm/core/ADT/SetVector.h"

namespace vm::core {
  class MachineRegisterInfo;

namespace rdf {
  struct DeadCodeElimination {
    DeadCodeElimination(DataFlowGraph &dfg, MachineRegisterInfo &mri)
      : Trace(false), DFG(dfg), MRI(mri), LV(mri, dfg) {}

    bool collect();
    bool erase(const SetVector<NodeId> &Nodes);
    void trace(bool On) { Trace = On; }
    bool trace() const { return Trace; }

    SetVector<NodeId> getDeadNodes() { return DeadNodes; }
    SetVector<NodeId> getDeadInstrs() { return DeadInstrs; }
    DataFlowGraph &getDFG() { return DFG; }

  private:
    bool Trace;
    SetVector<NodeId> LiveNodes;
    SetVector<NodeId> DeadNodes;
    SetVector<NodeId> DeadInstrs;
    DataFlowGraph &DFG;
    MachineRegisterInfo &MRI;
    Liveness LV;

    template<typename T> struct SetQueue;

    bool isLiveInstr(NodeAddr<StmtNode*> S) const;
    void scanInstr(NodeAddr<InstrNode*> IA, SetQueue<NodeId> &WorkQ);
    void processDef(NodeAddr<DefNode*> DA, SetQueue<NodeId> &WorkQ);
    void processUse(NodeAddr<UseNode*> UA, SetQueue<NodeId> &WorkQ);
  };
} // namespace rdf
} // namespace vm::core

#endif
