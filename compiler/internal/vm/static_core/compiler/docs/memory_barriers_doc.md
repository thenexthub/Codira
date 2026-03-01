# Optimize memory barriers

## Overview 

We need to encode barriers after the instructions NewArray, NewObject, NewMultiArray so that if the created objects are used in another thread, the initialization is fully completed.
We can remove the barrier if we prove that the created object cannot be passed to another thread before the next barrier. 
This can happen if we save the object to memory or pass it to another method

## Rationality

Reducing the number of instructions and speed up execution. 

## Dependence 

RPO analysis

## Algorithm 1: Merging Memory barriers (Implemented in both ArkIR and LLVM IR)

There is instruction flag `MEM_BARRIER`. The flag is set to `true` for the instructions NewObject, NewArray and NewMultiArray.  
The pass `OptimizeMemoryBarriers` try remove the flag(set false) from the instruction.  
We pass through all instructions in PRO order. If the instruction has flag `MEM_BARRIER` we add the instruction in special vector `barriers_insts_`.
If we visit an instruction that can pass an object to another thread(Store instruction, Call instruction e.t.c) we check the instruction inputs.  
If the instruction has input from the `barriers_insts_`, we call function `MergeBarriers`.  
The function set `false` for the flag `MEM_BARRIER`, exclude last instruction from the vector. 
So we will only set the barrier in the last instruction before potentially passing the created objects to another thread

The function `MergeBarriers` also is called at end of the basic block.

Codegen checks the flag `MEM_BARRIER` for the instructions NewObject, NewArray and NewMultiArray and encode memory barrier if the flag `true`

## Pseudocode for Algorithm 1

```
bool OptimizeMemoryBarriers::RunImpl()
{
    barriers_insts.clear();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
                barriers_insts.push_back(inst);
            }
            if (InstCanMoveObjectInAnotherthread(inst) && InstHasBarrierInput(inst, barriers_insts)) {
                MergeBarriers(barriers_insts);
            }
        }
        MergeBarriers(barriers_insts);
    }
    return true;
}

void MemoryBarriersVisitor::MergeBarriers(InstVector& barriers_insts)
{
    if (barriers_insts.empty()) {
        return;
    }
    auto last_barrier_inst = barriers_insts.back();
    for (auto inst : barriers_insts) {
        inst->ClearFlag(inst_flags::MEM_BARRIER);
    }
    last_barrier_inst->SetFlag(inst_flags::MEM_BARRIER);
    barriers_insts.clear();
}
```

## Examples for Algorithm 1

```
BB 0
prop: start
    0.i64  Constant                   0x2a -> (v6, v3, v1, v2, v8, v11)
succs: [bb 2]

BB 2  preds: [bb 0]
    1.     SaveState                  v0(vr0) -> (v2)
    2.ref  NewArray 1                 v0, v1 -> (v6, v3, v8, v11, v12)
    3.     SaveState                  v0(vr0), v2(vr1) -> (v5, v4)
    4.ref  LoadAndInitClass 'A'       v3 -> (v5)
    5.ref  NewObject 2                v4, v3 -> (v6, v8, v11, v12)
    6.     SaveState                  v0(vr0), v2(vr1), v5(vr2) -> (v7, v12)
    7.void CallStatic 3               v6
    8.     SaveState                  v0(vr0), v2(vr1), v5(vr2) -> (v9, v10)
    9.ref  LoadAndInitClass 'B'       v8 -> (v10)
   10.ref  NewObject 4                v9, v8 -> (v11, v13)
   11.     SaveState                  v0(vr0), v2(vr1), v5(vr2), v10(vr3)
   12.i64  CallVirtual 5              v2, v5, v6
   13.ref  Return                     v10
succs: [bb 1]

BB 1  preds: [bb 2]
prop: end
```

Instructions `2.ref  NewArray`, `5.ref  NewObject` and `10.ref  NewObject` have flag `MEM_BARRIER` by default.
`7.void CallStatic` don't have the instructions  `2.ref  NewArray`, `5.ref  NewObject` as inputs. 
So the pass `OptimizeMemoryBarriers` will remove the flag from these instructions and skip in `10.ref  NewObject`.  

## Algorithm 2: Memory Barrier Sinking (Implemented only in LLVM IR)  
Algorithm 1 calls `MergeBarriers` at the end to place memory barriers for allocations after last use of newly allocated object. This memory barrier is not always needed.  
Consider a path `BasicBlock1 -> BasicBlock2`. We call such a path to be `Protected` when all newly allocated objects at the end of `BasicBlock1` are protected by at least one memory barrier before any use of those newly allocated objects.  
This optimization is closely related to whether the paths from a predecessor to successors are protected or not. Currently, the optimization only covers the case where the predecessor block's terminator is a `br` instruction with 2 successors. The optimiztion is:
- If both successors paths are protected, then we may safely delete the memory barrier at the end of predecessor.
- If one successor path is protected, and the other successor only have one predecessor, then we may sink the memory barrier from the end of predecessor to the beginning of successor.

## Examples for Algorithm 2

### Protected Path
```
BB1:
    NewObject o1

BB2:
    NewObject o2
    MemoryBarrier
    func(o1, o2)

BB3:
    func(o1)
```
`BB1 -> BB2` is a protected path, because there is a `MemoryBarrier` between the allocation `NewObject o1` and the call `func(o1, o2)`. In contrast, `BB1 -> BB3` is not a protected path because `o1` is not protected by a `MemoryBarrier` before `func(o1)`.

### Memory Barriers Sinking
```
Before optimization:
BB1:
	NewObject o1
    MemoryBarrier
    go to BB2 or BB3 based on some condition
    
BB2:
	NewObject o2
    MemoryBarrier
    store v0, o2
    ...

BB3:
	call func(o1)

After optimization:
BB1:
	NewObject o1
    go to BB2 or BB3 based on some condition

BB2:
	NewObject o2
    MemoryBarrier
    store v0, o2
    ...

BB3:
	MemoryBarrier
	call func(o1)
```

## Links

Source code:   
[memory_barriers.cpp (ArkIR)](../optimizer/optimizations/memory_barriers.cpp)  
[memory_barriers.h (ArkIR)](../optimizer/optimizations/memory_barriers.h)  
[mem_barriers.cpp (LLVM IR)](../../libllvmbackend/transforms/passes/mem_barriers.cpp)  
[mem_barriers.h (LLVM IR)](../../libllvmbackend/transforms/passes/mem_barriers.h)  

Tests:  
[memory_barriers_test.cpp (ArkIR)](../tests/memory_barriers_test.cpp)  
[llvmaot-dmb-sinking.pa (LLVM IR)](../../tests/checked/llvmaot-dmb-sinking.pa)