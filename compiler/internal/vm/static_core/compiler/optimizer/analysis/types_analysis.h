/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMPILER_OPTIMIZER_ANALYSIS_TYPES_ANALISYS_H
#define COMPILER_OPTIMIZER_ANALYSIS_TYPES_ANALISYS_H

#include "optimizer/pass.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {
/**
 * TypesAnalysis sets assumed types(dynamic type) to phi instruction.
 * If a PHI is user of a CastValueToAnyType, the type of CastValueToAnyType is set as assumed type of the PHI
 * If a PHI is input of AnyTypeCheck, the type of CastValueToAnyType is set as assumed type of the PHI
 * If a PHI has assumed type and we try to set another type, we change assumed type to undefine.
 * If an assumed type is set for PHI, then we try to assign the type for all PHI that are inputs of the current PHI
 */
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class TypesAnalysis final : public Analysis, public GraphVisitor {
public:
    explicit TypesAnalysis(Graph *graph) : Analysis(graph) {}
    NO_MOVE_SEMANTIC(TypesAnalysis);
    NO_COPY_SEMANTIC(TypesAnalysis);
    ~TypesAnalysis() override = default;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    const char *GetPassName() const override
    {
        return "TypesAnalysis";
    }

    bool RunImpl() override;

protected:
    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst);
    static void VisitAnyTypeCheck(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"
private:
    void MarkedPhiRec(PhiInst *phi, AnyBaseType type);
    Marker marker_ {UNDEF_MARKER};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_TYPES_ANALISYS_H
