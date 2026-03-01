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

/**
 * @file
 *
 * This file declares the TyVar Constraints Solving class, which provides topological algorithm to process TyVar
 * Constraints Solving.
 */

#ifndef CODIRA_SEMA_TYVARCONSTRAINTGRAPH_H
#define CODIRA_SEMA_TYVARCONSTRAINTGRAPH_H

#include "Codira/Utils/Casting.h"
#include "Codira/AST/Types.h"
#include "Codira/Sema/TypeManager.h"

namespace Codira {
/*
 * A graph built from constraints (type variables and their lower and upper bounds).
 * The graph records and analyses the dependency of type variables by a topological sorting.
 */
class TyVarConstraintGraph {
    // A map mapping type variables to their constraints.
public:
    TyVarConstraintGraph(const Constraint& m, const TyVars& mayUsedTyVars, TypeManager& tyMgr) : tyMgr(tyMgr)
    {
        PreProcessConstraintGraph(m, mayUsedTyVars);
    }

    // Used to build the graph.
    void PreProcessConstraintGraph(const Constraint& m, const TyVars& mayUsedTyVars);

    // The method TopoOnce tries to get a set of type variables that are most independent and can be solved first.
    Constraint TopoOnce(const Constraint& m);

    // Substitute some type variables in the graph with their instantiated types.
    void ApplyTypeSubst(const TypeSubst& subst)
    {
        std::map<Ptr<TyVar>, int> newIndegree{};
        for (auto pair : std::as_const(indegree)) {
            if (auto tv = DynamicCast<TyVar*>(tyMgr.GetInstantiatedTy(pair.first, subst))) {
                newIndegree.emplace(tv, pair.second);
            }
        }
        this->indegree = newIndegree;

        std::map<Ptr<TyVar>, TyVars> newEdges{};
        for (auto pair : std::as_const(edges)) {
            if (auto tv = DynamicCast<TyVar*>(tyMgr.GetInstantiatedTy(pair.first, subst))) {
                newEdges.emplace(tv, StaticToTyVars(tyMgr.ApplyTypeSubstForTys(subst, pair.second)));
            }
        }
        this->edges = newEdges;

        TyVars newUsedTyVars{};
        for (auto tv : usedTyVars) {
            if (auto tv2 = DynamicCast<TyVar*>(tyMgr.GetInstantiatedTy(tv, subst))) {
                newUsedTyVars.emplace(tv2);
            }
        }
        this->usedTyVars = newUsedTyVars;

        TyVars newSolvedTyVars{};
        for (auto tv : solvedTyVars) {
            if (auto tv2 = DynamicCast<TyVar*>(tyMgr.GetInstantiatedTy(tv, subst))) {
                newSolvedTyVars.emplace(tv2);
            }
        }
        this->solvedTyVars = newSolvedTyVars;
    }

private:
    void FindLoopConstraints(const Constraint& m, Ptr<TyVar> start, Constraint& tyVarsInLoop);
    bool HasLoop(Ptr<TyVar> start, std::stack<Ptr<TyVar>>& loopPath);
    std::map<Ptr<TyVar>, int> indegree;
    std::map<Ptr<TyVar>, TyVars> edges;
    std::map<Ptr<TyVar>, bool> isVisited;
    TyVars solvedTyVars;
    TyVars usedTyVars;
    bool hasNext{true};
    TypeManager& tyMgr;
};
} // namespace Codira
#endif // CODIRA_SEMA_TYVARCONSTRAINTGRAPH_H
