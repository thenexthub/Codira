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

#include "cse.h"
#include "libarkbase/utils/logger.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/dominators_tree.h"
#include "algorithm"
#include "compiler_logger.h"

namespace ark::compiler {
void Cse::LocalCse()
{
    deletedInsts_.clear();
    candidates_.clear();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        candidates_.clear();
        for (auto inst : bb->Insts()) {
            if (!IsLegalExp(inst)) {
                continue;
            }
            // match the insts in candidates
            Exp exp = GetExp(inst);
            if (AllNotIn(candidates_, inst)) {
                InstVector emptyTmp(GetGraph()->GetLocalAllocator()->Adapter());
                emptyTmp.emplace_back(inst);
                candidates_.emplace(exp, std::move(emptyTmp));
                continue;
            }
            if (NotIn(candidates_, exp)) {
                exp = GetExpCommutative(inst);
            }

            auto &cntainer = candidates_.at(exp);
            auto iter = std::find_if(cntainer.begin(), cntainer.end(), Finder(inst));
            if (iter != cntainer.end()) {
                inst->ReplaceUsers(*iter);
                deletedInsts_.emplace_back(inst);
            } else {
                cntainer.emplace_back(inst);
            }
        }
    }
    RemoveInstsIn(&deletedInsts_);
}

void Cse::TryAddReplacePair(Inst *inst)
{
    if (!IsLegalExp(inst) || AllNotIn(candidates_, inst)) {
        return;
    }
    Exp exp = NotIn(candidates_, GetExp(inst)) ? GetExpCommutative(inst) : GetExp(inst);
    auto &cntainer = candidates_.at(exp);
    auto iter = std::find_if(cntainer.begin(), cntainer.end(), Finder(inst));
    if (iter != cntainer.end()) {
        replacePair_.emplace_back(inst, *iter);
    }
}

void Cse::CollectTreeForest()
{
    replacePair_.clear();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->IsEndBlock()) {
            continue;
        }
        candidates_.clear();
        for (auto inst : bb->Insts()) {
            if (!IsLegalExp(inst)) {
                continue;
            }
            auto exp = GetExp(inst);
            if (AllNotIn(candidates_, inst)) {
                InstVector vectmp(GetGraph()->GetLocalAllocator()->Adapter());
                vectmp.emplace_back(inst);
                candidates_.emplace(exp, std::move(vectmp));
            } else if (!NotIn(candidates_, exp)) {
                candidates_.at(exp).emplace_back(inst);
            } else {
                candidates_.at(GetExpCommutative(inst)).emplace_back(inst);
            }
        }
        if (candidates_.empty()) {
            continue;
        }
        for (auto domed : bb->GetDominatedBlocks()) {
            for (auto inst : domed->Insts()) {
                TryAddReplacePair(inst);
            }
        }
    }
}

/// Convert the tree-forest structure of replace_pair into star-forest structure
void Cse::ConvertTreeForestToStarForest()
{
    minReplaceStar_.clear();
    for (auto rpair : replacePair_) {
        Inst *temp = rpair.second;
        auto it = replacePair_.begin();
        do {
            it = replacePair_.begin();
            while (it != replacePair_.end() && it->first != temp) {
                it++;
            }
            if (it != replacePair_.end()) {
                temp = it->second;
            }
        } while (it != replacePair_.end());

        minReplaceStar_.emplace_back(temp, rpair.first);
    }
}

void Cse::EliminateAlongDomTree()
{
    // Eliminate along Dominator tree (based on star structure)
    deletedInsts_.clear();
    for (auto pair : minReplaceStar_) {
        pair.second->ReplaceUsers(pair.first);
        deletedInsts_.emplace_back(pair.second);
    }
    RemoveInstsIn(&deletedInsts_);
}

void Cse::BuildSetOfPairs(BasicBlock *block)
{
    sameExpPair_.clear();
    auto bbl = block->GetPredsBlocks()[0];
    auto bbr = block->GetPredsBlocks()[1];
    for (auto instl : bbl->Insts()) {
        if (!IsLegalExp(instl) || InVector(deletedInsts_, instl)) {
            continue;
        }
        Exp expl = GetExp(instl);
        if (!NotIn(sameExpPair_, expl)) {
            auto &pair = sameExpPair_.at(expl);
            pair.first.emplace_back(instl);
            continue;
        }
        if (!AllNotIn(sameExpPair_, instl)) {
            auto &pair = sameExpPair_.at(GetExpCommutative(instl));
            pair.first.emplace_back(instl);
            continue;
        }
        for (auto instr : bbr->Insts()) {
            if (!IsLegalExp(instr) || (NotSameExp(GetExp(instr), expl) && NotSameExp(GetExpCommutative(instr), expl)) ||
                InVector(deletedInsts_, instr)) {
                continue;
            }
            if (!NotIn(sameExpPair_, expl)) {
                auto &pair = sameExpPair_.at(expl);
                pair.second.emplace_back(instr);
                continue;
            }
            InstVector emptyl(GetGraph()->GetLocalAllocator()->Adapter());
            emptyl.emplace_back(instl);
            InstVector emptyr(GetGraph()->GetLocalAllocator()->Adapter());
            emptyr.emplace_back(instr);
            sameExpPair_.emplace(expl, std::make_pair(std::move(emptyl), std::move(emptyr)));
        }
    }
}

void Cse::DomTreeCse()
{
    CollectTreeForest();
    ConvertTreeForestToStarForest();
    EliminateAlongDomTree();
}

void Cse::GlobalCse()
{
    // Cse should not be used in OSR mode.
    if (GetGraph()->IsOsrMode()) {
        return;
    }

    deletedInsts_.clear();
    matchedTuple_.clear();
    static constexpr size_t TWO_PREDS = 2;
    // Find out redundant insts
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->GetPredsBlocks().size() != TWO_PREDS) {
            continue;
        }
        BuildSetOfPairs(bb);
        if (sameExpPair_.empty()) {
            continue;
        }
        // Build the set of matched insts
        for (auto inst : bb->Insts()) {
            if (!IsLegalExp(inst) || AllNotIn(sameExpPair_, inst)) {
                continue;
            }
            Exp exp = NotIn(sameExpPair_, GetExp(inst)) ? GetExpCommutative(inst) : GetExp(inst);
            auto &pair = sameExpPair_.find(exp)->second;
            ASSERT(!pair.first.empty());
            auto instl = *pair.first.begin();
            // If one decides to enable Cse in OSR mode then
            // ensure that inst's basic block is not OsrEntry.
            deletedInsts_.emplace_back(inst);
            auto lrpair = std::make_pair(instl, *(pair.second.begin()));
            matchedTuple_.emplace_back(inst, std::move(lrpair));
        }
    }
    // Add phi instruction
    matchedTuple_.shrink_to_fit();
    for (auto tuple : matchedTuple_) {
        auto inst = tuple.first;
        auto phi = GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
        inst->ReplaceUsers(phi);
        inst->GetBasicBlock()->AppendPhi(phi);
        auto &pair = tuple.second;
        phi->AppendInput(pair.first);
        phi->AppendInput(pair.second);
    }
    RemoveInstsIn(&deletedInsts_);
}

bool Cse::RunImpl()
{
    LocalCse();
    DomTreeCse();
    GlobalCse();
    if (!isApplied_) {
        COMPILER_LOG(DEBUG, CSE_OPT) << "Cse does not find duplicate insts";
    }
    return isApplied_;
}
}  // namespace ark::compiler
