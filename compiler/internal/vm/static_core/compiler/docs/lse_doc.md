# Load Store Elimination
## Overview 

The idea of optimization is to delete store instructions that store a value to memory that has been already written as well as delete load instructions that attempt to load a value that has been loaded earlier. 

## Rationality

Elimination of load and store instructions generally reduces the number of long latency memory instructions. It should be stated that the optimization increases the register pressure and as a result can increase the amount of required spills/fills.

## Dependence 

* AliasAnalysis
* LoopAnalysis
* Reverse Post Order (RPO)

## Algorithm

Algorithm needs to know that two memory instructions access the same memory address.  This can be done using alias analysis that accepts two memory instructions and is able to say about these accesses the following
*  `MUST_ALIAS` if the instructions definitely access the same memory address
*  `NO_ALIAS` if the instructions definitely access different memory addresses.
*  `MAY_ALIAS` if analysis can't say with confidence whether the instructions access the same memory address or different (or probably they may access the same address only under some conditions but under others – not)

The main structure for the algorithm is a heap representation in a form of 

`"Memory Instruction" -> "Value stored at location pointed by instruction"`

The heap is a representation of the real heap during execution. Each instruction that works with memory operates with a particular memory address. E.g. store instructions write values on the heap, load instructions read values from the heap. But if a load instruction tries to read a value that was written by store instruction above, this load can be eliminated because we already know the value without actual reading. The heap structure helps to keep track of values that memory instruction operates with (store instruction -- write, load instructions -- read).

Each basic block has its own heap. The initial value of a heap for a basic block is a merged heap from its predecessors. The result of heap merge is an intersected set of heap values on the same addresses. The initial value for entry basic block is empty heap. The empty heap is an initial value for loop headers as well because back edges can rewrite the heap values.

A dependence on predecessors needs us to traverse basic blocks in reverse post order so that for each basic block we already visited all it's predecessors (empty heap of one of predecessor is a back edge).

Once a heap is initialized for a basic block we iterate over instructions and update heap by applying the following rules to current instruction:

- if the instruction is a store and a stored value is equal to value from heap for this store then this store can be eliminated.
- if the instruction is a store and a value from heap for this store is absent or differs from a new stored value then the new stored value is written into heap.  The values of memory instructions that `MUST_ALIAS` this store are updated as well.  All values in the heap that `MAY_ALIAS` this store instruction are invalidated.
- if the instruction is a load and there is a value from the heap for this load then this load can be eliminated.
- if the instruction is a load and there is no value from the heap for this load then we update heap value for this load with the result of this load.  All instructions that `MUST_ALIAS` this load updated as well.
- If the instruction can invoke GC then all references on the heap that aren't mentioned in corresponded SaveState should be invalidated.
- if the instruction is a volatile load then the whole heap is cleared.
- if the instruction is a call then the whole heap is cleared.

All instructions that can be eliminated are recorded in a separate list for eliminated instructions and erase them at the end of optimization.

Loops have back edges by this reason we cannot eliminate anything in single traversal because further memory operations may update heap using back edge. Therefore an initial heap for loop header is empty. But still there is a case when we can use collected heap of loop preheader (e.g. if we can guarantee that memory access is read-only inside loop).

The typical example is the following (in pandasm):

```
.record T {
    i32[] array <static>
}

.function i32 T.foo() {
    movi v2, 0       #res
    movi v0, 0       #i
    ldobj.obj v1, T.array
    ldarr v1
    sta v3
loop:
    jge v3, loop_exit
    lda v0
    ldobj.obj v1, T.array
    ldarr v1
    add2 v2
    sta v2
    inci v0, 1
    lda v0
    jmp loop
loop_exit:   
    lda v2
    return 
}
```

It will be translated in the following code:

```
BB 2  preds: [bb 0]
    3.ref  LoadObject 242             v0 -> (v6)
    6.i32  LenArray                   v3 -> (v30, v16)
   30.i32  Cmp                        v7, v6 -> (v31)
   31.b    Compare GE i32             v30, v7 -> (v32)
   32.     IfImm NE b                 v31, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2, bb 4]
prop: head, loop 1
  10p.i32  Phi                        v7(bb2), v27(bb4) -> (v25, v27)
  11p.i32  Phi                        v7(bb2), v26(bb4) -> (v26)
   20.ref  LoadObject 242             v0 -> (v25)
   25.i32  LoadArray                  v20, v10p -> (v26)
   26.i32  Add                        v25, v11p -> (v11p, v35p)
   27.i32  Add                        v10p, v28 -> (v10p, v16)
   16.b    Compare GE i32             v27, v6 -> (v17)
   17.     IfImm NE b                 v16, 0x0
succs: [bb 3, bb 4]

BB 3  preds: [bb 2, bb 4]
  35p.i32  Phi                        v7(bb2), v26(bb4) -> (v29)
   29.i32  Return                     v35p
succs: [bb 1]
```

Here is `v20` inside loop that can be eliminated due to `v3`. However we can't record `v20` to list of loads that will be eliminated because further in the loop we can have a store instruction like the following:

```
v28.ref StoreObject 242 v0, v40
```

To handle this, each loop has its list of memory accesses that was loaded before loop (heap of preheader). We call these memory accesses as phi-candidates. Each phi-candidate has its own list of aliased memory accesses that will be collected further. While traversing a loop we check aliasing of each memory access inside the loop with phi-candidates.  If a memory access is `MAY_ALIAS`ed or `MUST_ALIAS`ed we put this memory access to the corresponding lists of phi-candidates.  Irreducible loops do not have preheader so we ignore them for optimization.

Finally, we iterate over all memory accesses collected in lists for each phi-candidate. If there is at least one store among them, we drop this phi-candidate because it is overwritten somewhere inside the loop. If there are only load instructions among them, then we can replace memory accesses inside the loop by values obtained from outside the loop (replace only `MUST_ALIAS`ed accesses).

The following rules are added to the list above to handle loops:

- visiting any memory access inside a reducible loop we check the aliasing with phi-candidates of this loop and all outer loop and record aliased ones to a corresponding candidate.
- all phi-candidates of a loop (and of all outer loops of this loop) are invalidated if any of instructions that clear heap have been met in this loop.

After iterating the whole graph the following actions are done
- iterate over phi-candidates with aliased accesses.
  * If any of aliased accesses for a candidate is a store, we do nothing.
  * If among aliased accesses only loads, add `MUST_ALIAS`ed load to elimination list.
- erase all instructions from elimination list

## Pseudocode

```
bool LSE::RunImpl() {
    Heap heap;
    PhiCands phis;

    LSEVisitor visitor(GetGraph(), &heap, &phis);
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsLoopHeader()) {
            // Put heap[block->GetLoop()->GetPreheader()] values to phis[block->GetLoop()]
            // heap[block] is empty
            MergeHeapValuesForLoop(block, &heap, &phis);
        } else {
            // Put in heap[block] values that are the same in block's predecessors
            MergeHeapValuesForBlock(block, &heap, &mustrix);
        }

        for (auto inst : block->Insts()) {
            if (IsHeapInvalidatingInst(inst)) {
                heap[block].clear();
                auto loop = block->GetLoop();
                while (!loop->IsRoot()) {
                    phis.at(block->GetLoop()).clear();
                    loop = loop->GetOuterLoop();
                }
            } else if (IsGCInst(inst)) {
                SaveStateInst *ss = inst->GetSaveState();
                if (inst->GetOpcode() == Opcode::SafePoint) {
                    ss = inst->CastToSafePoint();
                }
                // Invalidate references not mentioned in SaveState
                visitor.InvalidateRefs(block, ss);
            } else if (inst->IsLoad()) {
                visitor.VisitLoad(inst);
            } else if (inst->IsStore()) {
                visitor.VisitStore(inst, inst->GetStoredValue());
            }
        }
    }
    // Append instructions, that can be eliminated due to loop preheader heap values, to elimination list 
    visitor.FinalizeLoops();

    // Erase collected instructions
    for (auto elim : visitor.GetEliminations()) {
        DeleteInstruction(elim.inst, elim.value); // Replace elim.inst with elim.value
    }
}

void LSEVisitor::VisitStore(Inst *inst, Inst *val) {
    BasicBlock *block = inst->GetBasicBlock();
    auto &block_heap = heap_.at(block);
    // If a value stored on the heap is already there, we eliminate this store instruction
    auto mem = HeapHasEqaulValue(inst, val);
    if (mem != nullptr && LSE::CanEliminateInstruction(inst)) {
        eliminations_[inst] = block_heap[mem];
        return;
    }

    // If this store MUST_ALIAS any inst from phis[block], it prohibits any replacements of this phi candidate
    UpdatePhis(inst);

    // Erase all aliased values, because they may be overwritten
    for (auto heap_iter : block_heap) {
        if (inst.CheckAlias(heap_iter) != NO_ALIAS) {
          block_heap.erase(heap_iter);
        }
    }

    // Record a stored value to heap[block]
    block_heap[inst] = {inst, val};
}

void LSEVisitor::VisitLoad(Inst *inst) {
    BasicBlock *block = inst->GetBasicBlock();
    auto &block_heap = heap_.at(block);
    // We already know the value on the heap -> eliminate inst
    auto mem = HeapHasValue(inst);
    if (mem != nullptr && LSE::CanEliminateInstruction(inst))
        eliminations_[inst] = block_heap[mem]; // Store the value to replace instruction with it
        return;
    }

    // If this load MUST_ALIAS any inst from phis[block] it can be further replaced with value outside the loop
    UpdatePhis(inst);

    // Record loaded value to heap[block] and update all MUST_ALIASes
    block_heap[inst] = {inst, inst};
}
```

## Examples
### Loading stored value in `if` block
Before:
```
BB 2  preds: [bb 0]
    7.i32  StoreArray                 v0, v2, v1
    9.b    Compare GT i32             v1, v8 -> (v10)
   10.     IfImm NE b                 v9, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2]
   15.i32  LoadArray                  v0, v2 -> (v21)
   20.i32  LoadArray                  v0, v2 -> (v21)
   21.i32  Add                        v15, v20 -> (v22p)
succs: [bb 3]

BB 3  preds: [bb 2, bb 4]
  22p.i32  Phi                        v1(bb2), v21(bb4) -> (v23)
   23.i32  Return                     v22p
succs: [bb 1]
```
Here we can see that instruction `v15` and `v20` load a values stored by `v7`. As a result we can substitute these loads with stored value.

After:
```
BB 2  preds: [bb 0]
    7.i32  StoreArray                 v0, v2, v1
    9.b    Compare GT i32             v1, v8 -> (v10)
   10.     IfImm NE b                 v9, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2]
   21.i32  Add                        v1, v1 -> (v22p)
succs: [bb 3]

BB 3  preds: [bb 2, bb 4]
  22p.i32  Phi                        v1(bb2), v21(bb4) -> (v23)
   23.i32  Return                     v22p
succs: [bb 1]
```
### Object access elimination in loop
In this example we load array from object and sum its elements.
```
BB 2  preds: [bb 0]
    3.ref  LoadObject 242             v0 -> (v6)
    6.i32  LenArray                   v3 -> (v30, v16)
   30.i32  Cmp                        v7, v6 -> (v31)
   31.b    Compare GE i32             v30, v7 -> (v32)
   32.     IfImm NE b                 v31, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2, bb 4]
prop: head, loop 1
  10p.i32  Phi                        v7(bb2), v27(bb4) -> (v25, v27)
  11p.i32  Phi                        v7(bb2), v26(bb4) -> (v26)
   20.ref  LoadObject 242             v0 -> (v25)
   25.i32  LoadArray                  v20, v10p -> (v26)
   26.i32  Add                        v25, v11p -> (v11p, v35p)
   27.i32  Add                        v10p, v28 -> (v10p, v16)
   16.b    Compare GE i32             v27, v6 -> (v17)
   17.     IfImm NE b                 v16, 0x0
succs: [bb 3, bb 4]
```
Until the `LoadObject` accesses a volatile field, we can eliminate `v20` inside the loop and use `v3` instead.
```
BB 2  preds: [bb 0]
    3.ref  LoadObject 242             v0 -> (v6, v25)
    6.i32  LenArray                   v3 -> (v30, v16)
   30.i32  Cmp                        v7, v6 -> (v31)
   31.b    Compare GE i32             v30, v7 -> (v32)
   32.     IfImm NE b                 v31, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2, bb 4]
prop: head, loop 1
  10p.i32  Phi                        v7(bb2), v27(bb4) -> (v25, v27)
  11p.i32  Phi                        v7(bb2), v26(bb4) -> (v26)
   25.i32  LoadArray                  v3, v10p -> (v26)
   26.i32  Add                        v25, v11p -> (v11p, v35p)
   27.i32  Add                        v10p, v28 -> (v10p, v16)
   16.b    Compare GE i32             v27, v6 -> (v17)
   17.     IfImm NE b                 v16, 0x0
succs: [bb 3, bb 4]
```
## Links

Source code:  
[lse.cpp](../optimizer/optimizations/lse.cpp)
[lse.h](../optimizer/optimizations/lse.h)  

Tests:
[lse_test.cpp](../tests/lse_test.cpp)
