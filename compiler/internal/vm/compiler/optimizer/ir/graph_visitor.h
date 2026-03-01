/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef COMPILER_OPTIMIZER_IR_GRAPH_VISITOR_H
#define COMPILER_OPTIMIZER_IR_GRAPH_VISITOR_H

#include "opcodes.h"
#include "basicblock.h"

namespace panda::compiler {
class Graph;
/*
 * Base visitor class.
 *
 * Usage example:
 *
 * struct ExampleVisitor: public GraphVisitor {
 *     using GraphVisitor::GraphVisitor;
 *
 *     // Specify blocks to visit and their order
 *     const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
 *     {
 *         return GetGraph()->GetBlocksRPO();
 *     }
 *     // Print special message for Mul instruction
 *     static void VisitMul(GraphVisitor* v, Inst* inst) {
 *         std::cerr << "Multiply instruction\n";
 *     }
 *     // For all other instructions print its opcode
 *     void VisitDefault(Inst* inst) override {
 *         std::cerr << GetOpcodeString(inst->GetOpcode()) << std::endl;
 *     }
 *     // Visitor for all instructions which are the instance of the BinaryOperation
 *     void VisitInst(BinaryOperation* inst) override {
 *         std::cerr << "Visit binary operation\n";
 *     }
 *     #include "visitor.inc"
 * };
 *
 */
class GraphVisitor {
public:
    explicit GraphVisitor() = default;
    virtual ~GraphVisitor() = default;

    /**
     * Specify the order of visiting the blocks.
     * Should return a vector of graph's blocks, for example it could be obtained
     * from `GetGraph()->GetBlocksRPO()`.
     */
    virtual const ArenaVector<BasicBlock *> &GetBlocksToVisit() const = 0;

    virtual void VisitGraph() = 0;
    virtual uint64_t VisitGraphAndCount() = 0;
    virtual void VisitBlock(BasicBlock *bb) = 0;
    virtual void VisitInstruction(Inst *inst) = 0;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define GROUP_DEF(BASE) \
    virtual void VisitInst([[maybe_unused]] BASE *i) {}
    OPCODE_CLASS_LIST(GROUP_DEF)
#undef GROUP_DEF

    NO_COPY_SEMANTIC(GraphVisitor);
    NO_MOVE_SEMANTIC(GraphVisitor);

protected:
    using VisitFunc = void (*)(GraphVisitor *, Inst *);

    /**
     * Method that will be called if derived class doesn't set handler for opcode
     */
    virtual void VisitDefault([[maybe_unused]] Inst *inst) {}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, ...)                           \
    static void Visit##OPCODE(GraphVisitor *v, Inst *i) \
    {                                                   \
        v->VisitDefault(i);                             \
    }
    OPCODE_LIST(INST_DEF)
#undef INST_DEF

    /*
     * `visitor.inc` include must be in the end of all Visitor classes.
     * It defines following:
     * - VisitGraph() method, that aims to iterate over graph.
     * - VisitBlock() method, that aims to iterate over given basic block.
     * - VisitFunc table_ - list of methods for all opcodes.
     */
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_IR_GRAPH_VISITOR_H
