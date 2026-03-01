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

#ifndef PANDA_LOCATIONS_BUILDER_H
#define PANDA_LOCATIONS_BUILDER_H

#include "compiler/optimizer/ir/graph_visitor.h"
#include "compiler/optimizer/pass.h"

namespace ark::compiler {
class ParameterInfo;

template <Arch ARCH>  // NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LocationsBuilder : public GraphVisitor, public Optimization {
public:
    LocationsBuilder(Graph *graph, ParameterInfo *pinfo);

    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "LocationsBuilder";
    }

    bool ShouldDump() const override
    {
        return true;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override;

    static constexpr Target GetTarget()
    {
        return Target(ARCH);
    }
    static constexpr DataType::Type GetWordType()
    {
        return Is64BitsArch(ARCH) ? DataType::UINT64 : DataType::UINT32;
    }

    static Location GetLocationForReturn(Inst *inst);

    static void VisitResolveStatic(GraphVisitor *visitor, Inst *inst);
    static void VisitCallResolvedStatic(GraphVisitor *visitor, Inst *inst);
    static void VisitCallStatic(GraphVisitor *visitor, Inst *inst);
    static void VisitCallVirtual(GraphVisitor *visitor, Inst *inst);
    static void VisitResolveVirtual(GraphVisitor *visitor, Inst *inst);
    static void VisitCallResolvedVirtual(GraphVisitor *visitor, Inst *inst);
    static void VisitCallIndirect(GraphVisitor *visitor, Inst *inst);
    static void VisitCall(GraphVisitor *visitor, Inst *inst);
    static void VisitCallDynamic(GraphVisitor *visitor, Inst *inst);
    static void VisitCallNative(GraphVisitor *visitor, Inst *inst);
    static void VisitIntrinsic(GraphVisitor *visitor, Inst *inst);
    static void VisitParameter(GraphVisitor *visitor, Inst *inst);
    static void VisitReturn(GraphVisitor *visitor, Inst *inst);
    static void VisitThrow(GraphVisitor *visitor, Inst *inst);
    static void VisitNewArray(GraphVisitor *visitor, Inst *inst);
    static void VisitNewObject(GraphVisitor *visitor, Inst *inst);
    static void VisitLiveOut(GraphVisitor *visitor, Inst *inst);
    static void VisitMultiArray(GraphVisitor *visitor, Inst *inst);
    static void VisitStoreStatic(GraphVisitor *visitor, Inst *inst);
    static void VisitResolveByName(GraphVisitor *visitor, Inst *inst);

    void ProcessManagedCall(Inst *inst, ParameterInfo *pinfo = nullptr);
    void ProcessManagedCallStackRange(Inst *inst, size_t rangeStart, ParameterInfo *pinfo = nullptr);

private:
    ParameterInfo *GetResetParameterInfo();
    ParameterInfo *GetParameterInfo()
    {
        return paramsInfo_;
    }

#include "optimizer/ir/visitor.inc"
private:
    ParameterInfo *paramsInfo_ {nullptr};
};

bool RunLocationsBuilder(Graph *graph);
}  // namespace ark::compiler

#endif  // PANDA_LOCATIONS_BUILDER_H
