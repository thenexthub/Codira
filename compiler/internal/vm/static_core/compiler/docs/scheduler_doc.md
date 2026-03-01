# Instruction scheduling
## Overview 

Rearrange adjacent instructions for better performance.

## Rationality

When instructions are executed on CPU they may stall the processor pipeline when input registers are not ready yet, because they are written by one of the previous instructions. Scheduling allows to reduce the amount of such stalls in pipeline.

## Dependence 

* Dead Code Elimination(DCE)
* Remove Empty Blocks
* Remove Linear blocks
* Reverse Post Order(RPO)

## Algorithm

Current decisions/limitations:
 * Scheduler pass is placed immediately before register allocation
 * It rearranges instructions only inside the basic block, but not between them
 * No liveness analysis, only calculating dependencies using barrier/users/alias information
 * No CPU pipeline/resource modeling, only having dependency costs
 * Forward list scheduling algorithm with standard critical-path-based priority

For each basic block we first scan instructions in reverse order marking barriers and calculating the dependencies.
Together with dependencies we calculate priority as a longest (critical) path to leaf instructions in basic block dependency graph.

Than we schedule each interval between barriers using the following algorithm.
There are two priority queues, `waiting` and `ready`. `ready` queue is sorted based on priority calculated previously, while `waiting` queue is based on so-called `ASAP` (as soon as possible) values. In initialization, `ready` is empty and `waiting` contains all leaf instructions (without incoming dependencies), their `ASAP` is 1.

`ASAP` value for each instruction is changed only before it enters the `waiting` queue, and remains unchanged since that time.
Algorithm starts from tick `cycle` 1. If `ready` queue is empty we look through "soonest" instruction from `waiting` queue and if we need to skip some ticks without scheduling any instruction we have to adjust `cycle` value.
Next, we move all already available instructions (`ASAP` <= `cycle`) from `waiting` queue into `ready` queue.

Finally, we extract top instruction from `ready` queue and add it into new schedule. At this moment we adjust `ASAP` value for all dependent instructions and add some of them (which depend only on already scheduled instructions) into `waiting` queue. 

## Pseudocode

```c++
Scheduler::RunImpl() {
    for (auto bb : GetGraph()->GetBlocksRPO())
        ScheduleBasicBlock(bb);
}

// Dependency helper function
void Scheduler::AddDep(uint32_t* prio, Inst* from, Inst* to, uint32_t latency, Inst* barrier) {
    // Update instruction priority - "how high instruction is in dependency tree"
    *prio = std::max(*prio, latency + prio_[to]);
    // Do not add cross-barrier dependencies into deps_
    if (barrier == nullptr || old_[to] > old_[barrier]) {
        if (deps_.at(from).count(to) == 1) {
            uint32_t old_latency = deps_.at(from).at(to);
            if (old_latency >= latency)
                return;
        } else
            num_deps_[to]++;
        deps_.at(from)[to] = latency;
    }
}

// Rearranges instructions in the basic block using list scheduling algorithm.
Scheduler::ScheduleBasicBlock(BasicBlock* bb) {
    // Calculate priority and dependencies
    uint32_t num_inst = 0;
    Inst* last_barrier = nullptr;

    for (auto inst : bb->InstsSafeReverse()) {
        uint32_t prio = 0;
        old_.insert({inst, num_inst++});
        num_deps_.insert({inst, 0U});
        deps_.emplace(inst, GetGraph()->GetLocalAllocator()->Adapter());

        // Dependency to the barrier
        if (last_barrier != nullptr)
            AddDep(&prio, inst, last_barrier, 1U, last_barrier);
        // Dependency from barrier
        if (barrier) {
            Inst* old_last_barrier = last_barrier;
            last_barrier = inst;
            num_barriers++;
            for (auto user = inst->GetNext(); user != old_last_barrier; user = user->GetNext())
                AddDep(&prio, inst, user, 1U, last_barrier);
        }

        // Users
        for (auto& user_item : inst->GetUsers()) {
            auto user = user_item.GetInst();
            AddDep(&prio, inst, user, inst_latency, last_barrier);
        }

        .... // Memory dependencies calculation
        ...  // CanThrow or SaveState can't be rearranged, and stores can't be moved over them

        prio_.insert({inst, prio});
    }

    // Schedule intervals between barriers
    uint32_t cycle = 0;
    num_inst = 0;
    Inst* first = nullptr;
    for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
        bool barrier = inst->IsBarrier();
        num_inst++;
        if (first == nullptr)
            first = inst;
        if (barrier || inst == bb->GetLastInst()) {
            Inst* last = nullptr;
            if (barrier) {
                last = inst->GetPrev();
                num_inst--;
            } else
                last = inst;
            if (num_inst > 1)
                cycle += ScheduleInstsBetweenBarriers(first, last);
            else if (num_inst == 1) {
                sched_.push_back(first);
                cycle++;
            }
            if (barrier) {
                sched_.push_back(inst);
                cycle++;
            }
            num_inst = 0;
            first = nullptr;
        }
    }
    ... // Here we rearrange instructions in basic block according to sched_
}

// Schedule instructions between [first..last] inclusive, none of them are barriers.
uint32_t Scheduler::ScheduleInstsBetweenBarriers(Inst* first, Inst* last) {
    // Compare function for 'waiting' queue
    auto cmp_asap = [this](Inst* left, Inst* right) {
        return asap_[left] > asap_[right] || (asap_[left] == asap_[right] && old_[left] < old_[right]);
    };
    // Queue of instructions, which dependencies are scheduled already, but they are still not finished yet
    std::priority_queue<Inst*, ArenaVector<Inst*>, decltype(cmp_asap)> waiting(
        cmp_asap, GetGraph()->GetLocalAllocator()->Adapter());

    // Compare function for 'ready' queue
    auto cmp_prio = [this](Inst* left, Inst* right) {
        return prio_[left] < prio_[right] || (prio_[left] == prio_[right] && old_[left] < old_[right]);
    };
    // Queue of ready instructions
    std::priority_queue<Inst*, ArenaVector<Inst*>, decltype(cmp_prio)> ready(
        cmp_prio, GetGraph()->GetLocalAllocator()->Adapter());

     // Initialization, add leafs into 'waiting' queue
    uint32_t num_inst = 0;
    for (auto inst = first; inst != last->GetNext(); inst = inst->GetNext()) {
        asap_.insert({inst, 1U});
        if (num_deps_[inst] == 0)
            waiting.push(inst);
        num_inst++;
    }
    // Scheduling
    uint32_t cycle = 1;
    while (num_inst > 0) {
        if (ready.empty()) {
            uint32_t nearest = asap_[waiting.top()];
            // Skipping cycles where we can't schedule any instruction
            if (nearest > cycle)
                cycle = nearest;
        }
        // Move from 'waiting' to 'ready'
        while (!waiting.empty()) {
            Inst* soonest = waiting.top();
            if (asap_[soonest] <= cycle) {
                waiting.pop();
                ready.push(soonest);
            } else
                break;
        }
        // Extract top 'ready' instruction
        auto cur = ready.top();
        ready.pop();
        // Adjust all dependent instructions
        for (auto pair : deps_.at(cur)) {
            // Adjust asap
            uint32_t asap = asap_[pair.first];
            asap = std::max(asap, cycle + pair.second);
            asap_[pair.first] = asap;
            // Adjust num_deps
            uint32_t num_deps = num_deps_[pair.first];
            num_deps--;
            num_deps_[pair.first] = num_deps;
            if (num_deps == 0 && pair.first->GetOpcode() != Opcode::LoadPairPart)
                waiting.push(pair.first);
        }
        // Add into schedule
        sched_.push_back(cur);
        num_inst--;
        cycle++;
    }
    asap_.clear();
    return cycle;
}
```

## Examples

IR Before optimization:
```
BB 0
prop: start
    0.i64  Constant                   0x2a -> (v8)                        
    1.i64  Constant                   0x2b -> (v8)                        
    2.i64  Constant                   0x2c -> (v9)                        
    3.i64  Constant                   0x2d -> (v9)                        
    4.i64  Constant                   0x2e -> (v11)                       
    5.i64  Constant                   0x2f -> (v11)                       
    6.i64  Constant                   0x30 -> (v12)                       
    7.i64  Constant                   0x31 -> (v12)                       
succs: [bb 2]

BB 2  preds: [bb 0]
    8.u64  Add                        v0, v1 -> (v10)                     
    9.u64  Add                        v2, v3 -> (v10)                     
   10.u64  Add                        v8, v9 -> (v14)                     
   11.u64  Add                        v4, v5 -> (v13)                     
   12.u64  Add                        v6, v7 -> (v13)                     
   13.u64  Add                        v11, v12 -> (v14)                   
   14.u64  Add                        v10, v13 -> (v15)                   
   15.u64  Return                     v14                                 
succs: [bb 1]

BB 1  preds: [bb 2]
prop: end
```

IR after optimization:
```
BB 0
prop: start
    0.i64  Constant                   0x2a -> (v8)                        
    1.i64  Constant                   0x2b -> (v8)                        
    2.i64  Constant                   0x2c -> (v9)                        
    3.i64  Constant                   0x2d -> (v9)                        
    4.i64  Constant                   0x2e -> (v11)                       
    5.i64  Constant                   0x2f -> (v11)                       
    6.i64  Constant                   0x30 -> (v12)                       
    7.i64  Constant                   0x31 -> (v12)                       
succs: [bb 2]

BB 2  preds: [bb 0]
    8.u64  Add                        v0, v1 -> (v10)                     
    9.u64  Add                        v2, v3 -> (v10)                     
   11.u64  Add                        v4, v5 -> (v13)                     
   12.u64  Add                        v6, v7 -> (v13)                     
   10.u64  Add                        v8, v9 -> (v14)                     
   13.u64  Add                        v11, v12 -> (v14)                   
   14.u64  Add                        v10, v13 -> (v15)                   
   15.u64  Return                     v14                                 
succs: [bb 1]

BB 1  preds: [bb 2]
prop: end
```

Instruction 10 was moved down.

## Links

Algorithm: [article](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.211.7673&rep=rep1&type=pdf)

Source code:
[scheduler.cpp](../optimizer/optimizations/scheduler.cpp)
[scheduler.h](../optimizer/optimizations/scheduler.h)

Tests:
[scheduler_test.cpp](../tests/scheduler_test.cpp)
