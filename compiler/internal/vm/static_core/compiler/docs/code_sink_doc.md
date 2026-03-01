# Code Sink
## Overview 

The optimization moves instructions into successor blocks, when possible, so that they are not executed on paths where their results are not needed.

## Rationality

This optimization allows to avoid execution of statements that are not used on execution path. This should speedup the execution.

Motivational example:
```
BB 3
    0.i32  Parameter      arg 0 -> (v5, v8, v7)
succs: [bb 0]

BB 0  preds: [bb 3]
    8.i32  AddI           v0, 0x1 -> (v6)
    5.     IfImm GT i32   v0, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 0]
    6.i32  Return         v8

BB 1  preds: [bb 0]
    7.i32  Return         v0
```

In this example `v8` is used only in one branch however it is always executed. The code sinking optimization suggests to move `v8` into `BB 2`.

## Dependence 

* AliasAnalysis
* DominatorsTree
* LoopAnalysis
* Reverse Post Order (RPO)

## Algorithm

The iterative approach is used. On each iteration the optimization tries to sink each instruction to one of its immediately dominated blocks. It is possible if all users of the instruction is dominated by a block that the instruction is sunk into. Instructions in a basic block are iterated in reverse order to decrease the number of iterations. Iterating finishes when no instruction was sunk.

Instructions that cannot sink:

* Instructions allocating memory
* Control flow instructions
* Instructions that can throw an exception
* Barrier instructions (calls, monitors, volatile, SafePoints, etc.)
* Store instructions
* Load instructions if they dominate in scope of current basic block:
    * an aliased store instruction
    * a Monitor instruction
    * a volatile store instruction

To determine which load instruction can be sunk we keep a list of store instructions that have been met so far (we are iterating in reverse order; therefore, when we meet load instruction, we have already collected all stores after this load and can easily check on aliases).

Blocks that instruction cannot be sunk into:
* Blocks that do not dominate all users of the instruction
* Loads cannot be sunk into blocks with more than one predecessors (because other predecessors might have aliased stores)
* Do not sunk instructions into loops

## Pseudocode

```
void CodeSink::RunImpl() {
    // Iteratively sink instructions.  On each iteration an instruction can be
    // sunk to its basic block dominatee.  Iterate sinking until no changes
    // happens.
    bool changed = true;
    while(changed) {
        changed = false;
        for (auto block : GetGraph()->GetBlocksRPO()) {
            bool barriered = false;
            ArenaVector<Inst *> stores;
            for (auto inst : block->InstsSafeReverse()) {
                 barriered |= inst->IsMonitor() || (inst->IsStore && inst->IsVolatile());
                 candidate = SinkInstruction(inst, &stores, barriered);
                 if (candidate != nullptr) {
                     block->EraseInst(inst);
                     candidate->PrependInst(inst);
                     changed = true;
                 }
            }
        }  
    }
}

BasicBlock *CodeSink::SinkInstruction(Inst *inst, ArenaVector<Inst *> *stores, bool barriered) {
    // Save stores to be sure we do not sink a load instruction that may be aliased
    if (inst->IsStore()) {
        stores->push_back(inst);
        return nullptr;
    }
    // Check that instruction can be sunk
    if (inst->IsAllocation() || inst->IsControlFlow() || inst->CanThrow() || inst->IsBarrier()) {
        return nullptr;
    }
    if (inst->IsLoad()) {
        // Do not sink over monitor
        // Do not sink over volatile store
        if (barriered) {
            return nullptr;
        }
        for (auto store : *stores) {
            if (GetGraph()->CheckInstAlias(inst, store) != AliasType::NO_ALIAS) {
                return nullptr;
            }
        }
    }

    // Iterate over dominated blocks
    for (auto cand : inst->GetBasicBlock()->GetDominatedBlocks()) {
        if (IsAcceptableTarget(inst, cand)) {
            return cand;
        }
    }
    return nullptr;
}

bool CodeSink::IsAcceptableTarget(Inst *inst, BasicBlock *candidate) {
    BasicBlock *block = inst->GetBasicBlock();
    Loop *loop = block->GetLoop();
    Loop *cand_loop = candidate->GetLoop();
    if (candidate->GetPredsBlocks().size() > 1) {
        // Do not sink loads across a critical edge there may be stores in other code paths.
        if (inst->IsLoad()) {
            return false;
        }
        // Do not sink into loops
        if (loop != cand_loop) {
            return false;
        }
    }

    // Check that all uses are dominated by the candidate
    for (auto &user : inst->GetUsers()) {
        Inst *uinst = user.GetInst();
        if (!candidate->IsDominate(uinst->GetBasicBlock())) {
            return false;
        }
    }
    return true;
}
```

## Examples
### Regular sinking
```
BB 2  preds: [bb 0]
    5.i32  Add                        v3, v2 -> (v15)
    8.i64  LoadObject 243             v0 -> (v9, v13)
    9.b    Compare NE i64             v8, v4 -> (v10)
   10.     IfImm NE b                 v9, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2]
   13.i64  StoreObject 243            v1, v8
succs: [bb 3]

BB 3  preds: [bb 2, bb 4]
   15.i32  Return                     v5
succs: [bb 1]
```
Sink arithmetic operation `v5` but do not `v8` because `BB 3` has several predecessors.
```
BB 2  preds: [bb 0]
    8.i64  LoadObject 243             v0 -> (v9, v13)
    9.b    Compare NE i64             v8, v4 -> (v10)
   10.     IfImm NE b                 v9, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2]
   13.i64  StoreObject 243            v1, v8
succs: [bb 3]

BB 3  preds: [bb 2, bb 4]
    5.i32  Add                        v3, v2 -> (v15)
   15.i32  Return                     v5
succs: [bb 1]
```
### Loop Sinking
```
BB 2  preds: [bb 0]                     
    6.i64  Add                        v1, v5 -> (v21)
succs: [bb 3]

BB 3  preds: [bb 2, bb 3]
prop: head, loop 1
  10p.i64  Phi                        v4(bb2), v22(bb3) -> (v23, v22, v20)
   20.i64  LoadArray                  v2, v10p -> (v21)
   21.i64  Add                        v20, v6 -> (v26, v22)
   22.i64  Add                        v21, v10p -> (v10p, v26)
   23.i32  Add                        v10p, v3 -> (v24)
   26.i64  Add                        v21, v22 -> (v27)
   24.b    Compare LT i32             v23, v0 -> (v25)
   25.     IfImm NE b                 v24, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 3]
   27.i64  Return                     v26
succs: [bb 1]                                                             
```
Sinking `v6` into loop and `v26` out of loop
```
BB 2  preds: [bb 0]
    6.i64  Add                        v1, v5 -> (v21)
succs: [bb 3]

BB 3  preds: [bb 2, bb 3]
prop: head, loop 1
  10p.i64  Phi                        v4(bb2), v22(bb3) -> (v23, v20, v22)
   20.i64  LoadArray                  v2, v10p -> (v21)
   21.i64  Add                        v20, v6 -> (v22, v26)
   22.i64  Add                        v21, v10p -> (v10p, v26)
   23.i32  Add                        v10p, v3 -> (v24)
   24.b    Compare LT i32             v23, v0 -> (v25)
   25.     IfImm NE b                 v24, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 3]
   26.i64  Add                        v21, v22 -> (v27)
   27.i64  Return                     v26
succs: [bb 1]

```
## Links

Source code:   
[code_sink.cpp](../optimizer/optimizations/code_sink.cpp)  
[code_sink.h](../optimizer/optimizations/code_sink.h)

Tests:  
[code_sink_test.cpp](../tests/code_sink_test.cpp)
