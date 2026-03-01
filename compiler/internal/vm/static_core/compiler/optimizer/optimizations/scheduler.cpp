/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "scheduler.h"

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "compiler_logger.h"

namespace ark::compiler {
/* Instruction scheduling.
 * Current decisions/limitations
 *
 * 1. Scheduler pass is placed immediately before register allocator.
 * 2. It rearranges instructions only inside the basic block, but not between them.
 * 3. No liveness analysis, only calculating dependencies using barrier/users/alias information.
 * 4. No CPU pipeline/resource modeling, only having dependency costs.
 * 5. Forward list scheduling algorithm with standart critical-path-based priority.
 */
bool Scheduler::RunImpl()
{
    COMPILER_LOG(DEBUG, SCHEDULER) << "Run " << GetPassName();
    SaveStateBridgesBuilder ssb;
    bool result = false;
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (!bb->IsEmpty() && !bb->IsStartBlock()) {
            if (ScheduleBasicBlock(bb)) {
                ssb.FixSaveStatesInBB(bb);
                result = true;
            }
        }
    }
    COMPILER_LOG(DEBUG, SCHEDULER) << GetPassName() << " completed";
    return result;
}

// Dependency helper function
void Scheduler::AddDep(uint32_t *prio, Inst *from, Inst *to, uint32_t latency, Inst *barrier)
{
    if (from == to) {
        return;
    }
    COMPILER_LOG(DEBUG, SCHEDULER) << "Found dependency " << from->GetId() << " -> " << to->GetId() << " latency "
                                   << latency;
    // Estimate old cycle (without scheduling)
    ocycle_[from] = std::max(ocycle_[from], latency + ocycle_[to]);

    // Update instruction priority - "how high instruction is in dependency tree"
    *prio = std::max(*prio, latency + prio_[to]);

    // Do not add cross-barrier dependencies into deps_
    if (barrier == nullptr || old_[to] > old_[barrier]) {
        if (deps_.at(from).count(to) == 1) {
            uint32_t oldLatency = deps_.at(from).at(to);
            if (oldLatency >= latency) {
                return;
            }
        } else {
            numDeps_[to]++;
        }
        deps_.at(from)[to] = latency;
    }
}

// Calculate priority and dependencies
bool Scheduler::BuildAllDeps(BasicBlock *bb)
{
    auto markerHolder = MarkerHolder(GetGraph());
    mrk_ = markerHolder.GetMarker();

    oprev_ = 0;
    numBarriers_ = 0;
    maxPrio_ = 0;

    static constexpr uint32_t TOO_LONG_BB = 64;
    uint32_t numInst = 0;
    uint32_t numBetween = 0;
    uint32_t numSpecial = 0;
    Inst *lastBarrier = nullptr;
    for (auto inst : bb->InstsSafeReverse()) {
        ProcessInst(inst, &numInst, &numBetween, &numSpecial, &lastBarrier);

        if (numSpecial > TOO_LONG_BB || numBetween > TOO_LONG_BB) {
            COMPILER_LOG(DEBUG, SCHEDULER) << "Block " << bb->GetId() << " seems too big for scheduling, skipping";
            Cleanup();
            return false;
        }
    }
    return true;
}

// One instruction deps
void Scheduler::ProcessInst(Inst *inst, uint32_t *numInst, uint32_t *numBetween, uint32_t *numSpecial,
                            Inst **lastBarrier)
{
    uint32_t prio = 0;
    uint32_t instLatency = inst->Latency();
    bool barrier = inst->IsBarrier();

    (*numBetween)++;
    old_.insert({inst, (*numInst)++});
    ocycle_.insert({inst, ++oprev_});
    numDeps_.insert({inst, 0U});
    deps_.emplace(inst, GetGraph()->GetLocalAllocator()->Adapter());

    // Dependency to the barrier
    if (*lastBarrier != nullptr) {
        AddDep(&prio, inst, *lastBarrier, 1U, *lastBarrier);
    }

    // Dependency from barrier
    if (barrier) {
        Inst *oldLastBarrier = *lastBarrier;
        *lastBarrier = inst;
        numBarriers_++;
        *numBetween = 0;
        for (auto user = inst->GetNext(); user != oldLastBarrier; user = user->GetNext()) {
            AddDep(&prio, inst, user, 1U, *lastBarrier);
        }
    }

    // Users
    for (auto &userItem : inst->GetUsers()) {
        auto user = userItem.GetInst();
        if (user->IsMarked(mrk_)) {
            AddDep(&prio, inst, user, instLatency, *lastBarrier);
        }
    }

    if (inst->IsMovableObject()) {
        // We take all SaveState, that has RuntimeCall users, under this reference instruction and create dependence
        // between SaveState and this instruction. See also GraphChecker::CheckSaveStatesWithRuntimeCallUsers.
        for (auto &ss : ssWithRuntimeCall_) {
            AddDep(&prio, inst, ss, 1U, *lastBarrier);
        }
    }

    if (inst->IsMemory() || inst->IsRefSpecial()) {
        ProcessMemory(inst, &prio, *lastBarrier);
        (*numSpecial)++;
    }

    if (inst->CanThrow() || inst->IsRuntimeCall() || inst->IsSaveState()) {
        ProcessSpecial(inst, &prio, *lastBarrier);
        (*numSpecial)++;
    }

    inst->SetMarker(mrk_);
    prio_.insert({inst, prio});
    maxPrio_ = std::max(maxPrio_, prio);
    oprev_ = ocycle_[inst];
}

// Memory
void Scheduler::ProcessMemory(Inst *inst, uint32_t *prio, Inst *lastBarrier)
{
    if (inst->IsRefSpecial()) {
        loads_.push_back(inst);
        return;
    }
    for (auto mem : stores_) {
        if (GetGraph()->CheckInstAlias(inst, mem) != AliasType::NO_ALIAS) {
            AddDep(prio, inst, mem, 1U, lastBarrier);
        }
    }
    if (inst->IsStore()) {
        for (auto mem : loads_) {
            if (mem->IsLoad() && GetGraph()->CheckInstAlias(inst, mem) != AliasType::NO_ALIAS) {
                AddDep(prio, inst, mem, 1U, lastBarrier);
            }
        }
        for (auto ct : special_) {
            AddDep(prio, inst, ct, 1U, lastBarrier);
        }
        stores_.push_back(inst);
    } else {  // means inst->IsLoad()
        loads_.push_back(inst);
    }
}

void Scheduler::ProcessSpecialBoundsCheckI(Inst *inst, uint32_t *prio, Inst *lastBarrier)
{
    auto value = inst->CastToBoundsCheckI()->GetImm();
    // Remove loads with same immediate. No easy way to check arrays are same.
    for (auto load : loads_) {
        if (load->GetOpcode() == Opcode::LoadArrayPairI) {
            auto imm = load->CastToLoadArrayPairI()->GetImm();
            if (imm == value || imm + 1 == value) {
                AddDep(prio, inst, load, 1U, lastBarrier);
            }
        } else if (load->GetOpcode() == Opcode::LoadArrayI && load->CastToLoadArrayI()->GetImm() == value) {
            AddDep(prio, inst, load, 1U, lastBarrier);
        }
    }
}

// CanThrow or SaveState can't be rearranged, and stores can't be moved over them
void Scheduler::ProcessSpecial(Inst *inst, uint32_t *prio, Inst *lastBarrier)
{
    for (auto mem : stores_) {
        AddDep(prio, inst, mem, 1U, lastBarrier);
    }
    for (auto ct : special_) {
        AddDep(prio, inst, ct, 1U, lastBarrier);
    }

    if (inst->IsSaveState() && inst != inst->GetBasicBlock()->GetFirstInst()) {
        // SaveStates that have RuntimeCall users are being added into a vector to create dependencies from preceding
        // reference instructions
        for (auto &user : inst->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst->IsRuntimeCall()) {
                ssWithRuntimeCall_.push_back(inst);
                break;
            }
        }
    }
    // 1. SafePoint also has this flag
    // 2. GC triggered inside can poison loaded reference value
    if (inst->IsRuntimeCall()) {
        for (auto mem : loads_) {
            if (mem->GetType() == DataType::REFERENCE || mem->GetType() == DataType::ANY) {
                AddDep(prio, inst, mem, 1U, lastBarrier);
            }
        }
    }
    // We have to "restore" BoundsCheckI -> LoadArrayI dependency
    if (inst->GetOpcode() == Opcode::BoundsCheckI) {
        ProcessSpecialBoundsCheckI(inst, prio, lastBarrier);
    }
    special_.push_back(inst);
}

void Scheduler::ScheduleBarrierInst(Inst **inst)
{
    sched_.push_back((*inst));
    if ((*inst)->WithGluedInsts()) {
        auto next = (*inst)->GetNext();
        auto nnext = next->GetNext();
        ASSERT(next->GetOpcode() == Opcode::LoadPairPart && nnext->GetOpcode() == Opcode::LoadPairPart);
        sched_.push_back(next);
        sched_.push_back(nnext);
        *inst = nnext;
        for (auto pair : deps_.at(next)) {
            uint32_t numDeps = numDeps_[pair.first];
            ASSERT(numDeps > 0);
            numDeps--;
            numDeps_[pair.first] = numDeps;
        }
        for (auto pair : deps_.at(nnext)) {
            uint32_t numDeps = numDeps_[pair.first];
            ASSERT(numDeps > 0);
            numDeps--;
            numDeps_[pair.first] = numDeps;
        }
    }
}

// Rearranges instructions in the basic block using list scheduling algorithm.
bool Scheduler::ScheduleBasicBlock(BasicBlock *bb)
{
    COMPILER_LOG(DEBUG, SCHEDULER) << "Schedule BB " << bb->GetId();
    if (!BuildAllDeps(bb)) {
        return false;
    }

    // Schedule intervals between barriers
    uint32_t cycle = 0;
    uint32_t numInst = 0;
    Inst *first = nullptr;
    for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
        bool barrier = inst->IsBarrier();
        numInst++;
        inst->ClearMarkers();
        if (first == nullptr) {
            first = inst;
        }
        if (barrier || inst == bb->GetLastInst()) {
            Inst *last = nullptr;
            if (barrier) {
                last = inst->GetPrev();
                numInst--;
            } else {
                last = inst;
            }
            if (numInst > 1) {
                cycle += ScheduleInstsBetweenBarriers(first, last);
            } else if (numInst == 1) {
                ASSERT(first->GetOpcode() != Opcode::LoadPairPart);
                sched_.push_back(first);
                cycle++;
            }
            if (barrier) {
                ASSERT(inst->GetOpcode() != Opcode::LoadPairPart);
                ScheduleBarrierInst(&inst);
                cycle++;
            }
            numInst = 0;
            first = nullptr;
        }
    }
    return FinalizeBB(bb, cycle);
}

bool Scheduler::FinalizeBB(BasicBlock *bb, uint32_t cycle)
{
    // Rearrange instructions in basic block according to schedule
    bool result = false;
    bool hasPrev = false;
    uint32_t prev;
    for (auto inst : sched_) {
        auto cur = old_[inst];
        if (hasPrev && prev - 1 != cur) {
            result = true;
        }
        prev = cur;
        hasPrev = true;

        bb->EraseInst(inst);
        bb->AppendInst(inst);
    }

    if (result) {
        COMPILER_LOG(DEBUG, SCHEDULER) << "Stats for block " << bb->GetId() << ": old cycles = " << oprev_
                                       << ", num barriers = " << numBarriers_ << ", critical path = " << maxPrio_
                                       << ", scheduled = " << cycle;
        GetGraph()->GetEventWriter().EventScheduler(bb->GetId(), bb->GetGuestPc(), oprev_, numBarriers_, maxPrio_,
                                                    cycle);
    }

    Cleanup();
    return result;
}

void Scheduler::Cleanup()
{
    sched_.clear();
    loads_.clear();
    stores_.clear();
    special_.clear();
    old_.clear();
    ocycle_.clear();
    numDeps_.clear();
    prio_.clear();
    deps_.clear();
    ssWithRuntimeCall_.clear();
}

// Rearranges instructions between [first..last] inclusive, none of them are barriers.
uint32_t Scheduler::ScheduleInstsBetweenBarriers(Inst *first, Inst *last)
{
    COMPILER_LOG(DEBUG, SCHEDULER) << "SchedBetween " << first->GetId() << " and " << last->GetId();

    // Compare function for 'waiting' queue
    auto cmpAsap = [this](Inst *left, Inst *right) {
        return asap_[left] > asap_[right] || (asap_[left] == asap_[right] && old_[left] < old_[right]);
    };
    // Queue of instructions, which dependencies are scheduled already, but they are still not finished yet
    SchedulerPriorityQueue waiting(cmpAsap, GetGraph()->GetLocalAllocator()->Adapter());

    // Compare function for 'ready' queue
    auto cmpPrio = [this](Inst *left, Inst *right) {
        return prio_[left] < prio_[right] || (prio_[left] == prio_[right] && old_[left] < old_[right]);
    };
    // Queue of ready instructions
    SchedulerPriorityQueue ready(cmpPrio, GetGraph()->GetLocalAllocator()->Adapter());

    // Initialization, add leafs into 'waiting' queue
    uint32_t numInst = 0;
    for (auto inst = first; inst != last->GetNext(); inst = inst->GetNext()) {
        asap_.insert({inst, 1U});
        if (numDeps_[inst] == 0) {
            COMPILER_LOG(DEBUG, SCHEDULER) << "Queue wait add " << inst->GetId();
            waiting.push(inst);
        }
        numInst++;
    }

    // Scheduling
    uint32_t cycle = 1;
    while (numInst > 0) {
        if (ready.empty()) {
            ASSERT(!waiting.empty());
            uint32_t nearest = asap_[waiting.top()];
            // Skipping cycles where we can't schedule any instruction
            if (nearest > cycle) {
                cycle = nearest;
            }
        }

        // Move from 'waiting' to 'ready'
        while (!waiting.empty()) {
            Inst *soonest = waiting.top();
            if (asap_[soonest] <= cycle) {
                waiting.pop();
                COMPILER_LOG(DEBUG, SCHEDULER) << "Queue ready moving " << soonest->GetId();
                ready.push(soonest);
            } else {
                break;
            }
        }
        ASSERT(!ready.empty());

        // Schedule top 'ready' instruction (together with glued, when necessary)
        numInst -= SchedWithGlued(ready.top(), &waiting, cycle++);
        ready.pop();
    }

    // Cleanup
    asap_.clear();
    return cycle;
}

uint32_t Scheduler::SchedWithGlued(Inst *inst, SchedulerPriorityQueue *waiting, uint32_t cycle)
{
    uint32_t amount = 0;
    // Compare function for 'now' queue
    auto cmpOld = [this](Inst *left, Inst *right) { return old_[left] < old_[right]; };
    // Queue of instructions to schedule at current cycle
    SchedulerPriorityQueue now(cmpOld, GetGraph()->GetLocalAllocator()->Adapter());
    // Add inst into 'now'
    ASSERT(now.empty());
    now.push(inst);

    // Add glued instructions into 'now'
    if (inst->WithGluedInsts()) {
        for (auto &userItem : inst->GetUsers()) {
            auto user = userItem.GetInst();
            ASSERT(user->GetOpcode() == Opcode::LoadPairPart);
            now.push(user);
        }
    }

    [[maybe_unused]] constexpr auto MAX_NOW_SIZE = 3;
    ASSERT(now.size() <= MAX_NOW_SIZE);

    // Schedule them
    while (!now.empty()) {
        auto cur = now.top();
        now.pop();
        COMPILER_LOG(DEBUG, SCHEDULER) << "Scheduling " << cur->GetId() << " at cycle " << cycle;

        // Adjust all dependent instructions
        for (auto pair : deps_.at(cur)) {
            // Adjust asap
            uint32_t asap = asap_[pair.first];
            asap = std::max(asap, cycle + pair.second);
            asap_[pair.first] = asap;

            // Adjust numDeps
            uint32_t numDeps = numDeps_[pair.first];
            ASSERT(numDeps > 0);
            numDeps--;
            numDeps_[pair.first] = numDeps;

            // Glued instructions scheduled immediately, so they should not go into queue
            if (numDeps == 0 && pair.first->GetOpcode() != Opcode::LoadPairPart) {
                COMPILER_LOG(DEBUG, SCHEDULER) << "Queue wait add " << pair.first->GetId();
                waiting->push(pair.first);
            }
        }

        // Add into schedule
        sched_.push_back(cur);
        amount++;
    }

    return amount;
}
}  // namespace ark::compiler
