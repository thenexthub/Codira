# Memory Coalescing
## Overview 

The optimization is based on the fact that some architectures (`AArch64` particularly) support simultaneous load and store operations for consecutive addresses instead of several separate operations.

## Rationality

Replacing two memory operations with one generally reduces the number of long latency memory instructions.

| Code | Regular | Optimized |
| ------ | ------ | ------|
| `num[0] * num[1];` | `ldr     x1, [x0]` <br> `ldr     x0, [x0, 8]` <br> `mul     x0, x1, x0` | `ldp     x1, x0, [x0]` <br> `mul     x0, x1, x0` |

## Dependence 

* DominatorsTree
* LoopAnalyzer
* AliasAnalysis
* Reverse Post Order (RPO)

## Assumptions

The optimization was implemented for arrays' accesses for `AArch64` architecture.

Array accesses cannot be volatile.

`AArch64` has `32`-bit and `64`-bit versions of coalescing operations – `ldp` and `stp`. As a result it is possible only for following Panda's types: `INT32`, `UINT32`, `INT64`, `UINT64`, `REFERENCE`.

## Algorithm

To implement such kind of optimization the extra support from IR is required

### IR Support

The following actions are required

* Separate instructions that will represent coalesced memory accesses from regular accesses.
* Handle multiple output from one instruction in terms of SSA

The case with a coalesced store is quite straightforward: having two consecutive stores we replace them by one instruction that accepts index and two values to store.

| Consecutive Stores | Coalesced Store |
| --- | --- |
| `248.i64  StoreArrayI  v2, 0x0, v53` <br> `250.i64  StoreArrayI  v2, 0x1, v53` | `251.i64 StoreArrayPairI v2, 0x0, v53, v53` |

The problem occurs with a coalesced load because a load instruction of multiple values produces multiple assignment that is not a part of SSA form. By this reason, we need additional pseudo instructions as `LoadPairPart` to divide multiple values into single values. The type of `LoadArrayPair` instruction corresponds to the type of a single element, so there is an assumption that `LoadArrayPair` loads only multiple values of the same type.

| Consecutive Loads | Coalesced Load |
| --- | --- |
| `58.i64  LoadArrayI  v2, 0x0 -> (v37)` <br> `61.i64  LoadArrayI  v2, 0x1 -> (v43)`  | `62.i64  LoadArrayPairI v2, 0x0 -> (v63, v64)` <br> `63.i64  LoadPairPart  v62, 0x0 -> (v37)` <br> `64.i64  LoadPairPart  v62, 0x1 -> (v43)`  |

### Transformation

The optimization tries to coalesce memory operations in a scope of a basic block. It needs that two consecutive memory operations are placed near each other without intermediate instructions. By this reason we need to find a place to sunk an upper memory operation and to hoist the lower according to reordering rules.

During hoisting and sinking of memory operations we use rules for memory instruction scheduling: do not move over monitors, calls, save states, save points and etc.

Memory coalescing was implemented for array accesses. We process instructions of basic block in order. To find accesses of consecutive memory addresses we keep a queue of candidates. Each instruction that may be coalesced is inserted into this queue. A candidate is marked as invalid in the following conditions:
 * it has been paired already
 * store candidates are invalid if SaveState instruction has been met
 * all candidates are invalid if a barrier is met: calls, control flow instructions, monitors, exceptions, intrinsic and etc.

To track indices we use basic implementation of scalar evolution that allows to track how variables evolves: basic value (variable or constant), difference from the basic value (if basic value is a variable) and evolution (if basic value is a variable incremented on each iteration of a loop). It is a simple graph bypass by collecting assignments including Phi evolutions (supported only addition and subtraction).

Processing each instruction in basic block we do the following:
 1) If the instruction cannot be coalesced.
     1) If the instruction is a barrier – invalidate all candidates.
     2) If the instruction is a SaveState – invalidate all store candidates.
     3) If the instruction is a memory operation – add it as invalid candidate.
 2) If we can't determine anything about index variable, we add this instruction as a candidate and move on next instruction
 3) Iterate candidates in backward order
     2) If a candidate is invalid **or** candidate cannot be coalesced with the instruction **or** both refer to different objects **or** we have no information about candidate index, move on next candidate
     3) If indices differs by one and there is a place to sunk the candidate instruction and hoist the currently processing instruction – add this candidate and the instruction as a pair for coalescing and invalidate both.
 4) Add the instruction as a candidate.

Finally, we replace collected pairs by coalesced instructions.

To find a place for candidate and current instruction:
 1) find the lowest position the candidate can be sunk
 2) find the highest position the instruction can be hoisted
 3) The place can be any between highest and lowest position. If the intersection is empty, coalescing is not possible.

## Pseudocode

```
void MemoryCoalescing::RunImpl() {
    VariableAnalysis variables(GetGraph());
    for (auto block : GetGraph()->GetBlocksRPO()) {
        for (auto inst : block.Insts()) {
            if (IsArrayAccess(inst)) {
                HandleArrayAccess(inst, variables);
            } else if (inst->IsMemory()) {
                inst->SetMarker(mrk_invalid_);
                candidates.push_back(inst);
            } else if (inst->IsBarrier()) {
                // Remove all candidates -- do not move anything across barriers
                candidates.clear();
            }
        }
        // Work in scope of basic block
        candidates.clear();
    }

    for (auto pair : pairs) {
        // Replace a pair of instructions by a coalesced instruction
        ReplacePair(pair);
    }
}

void HandleArrayAccess(Inst *inst, VariableAnalysis &vars) {
    Inst *obj = inst->GetObject();
    Inst *idx = inst->GetIndex();
    // If we don't know anything about index, do nothing
    if (!vars.IsAnalyzed(idx)) {
        candidates.push_back(inst);
        return;
    }
    // Last candidates more likely to be coalesced
    for (auto iter = candidates.rbegin(); iter != candidates.rend(); iter++) {
        auto cand = *iter;
        // Skip not interesting candidates: invalid and that cannot be coalesced with current inst
        if (cand->IsMarked(invalid) || cand->GetOpcode() != inst->GetOpcode()) {
            continue;
        }

        Inst *cand_obj = cand->GetObject();
        auto cand_idx = cand->GetIndex();
        // We need to have info about candidate's index and array objects must alias each other
        if (!vars.IsAnalyzed(cand_idx) || obj.IsAlias(cand_obj) != MUST_ALIAS) {
            continue;
        }
        // If both indices differs by one
        Inst *position = FindBetterPlace(cand, inst);
        if (poisition && vars.DiffersByConst(idx, cand_idx, 1)) {
            pairs.push_back({cand, inst, position});
            cand->SetMarker(invalid);
            inst->SetMarket(invalid);
        }
    }

    candidates.push_back(inst);
}
```

## Examples

### Loads and Stores with immediate indices
Before optimization
```
BB 0
prop: start
    0.i64  Constant                   0x2a -> (v3)
succs: [bb 2]

BB 2  preds: [bb 0]
    3.ref  NewArray 77                v0 -> (v42, v41)
   41.     SaveState                  v3(vr7) -> (v42)
   42.ref  NullCheck                  v3, v41 -> (v225, v229, v227, v230)
  225.i64  LoadArrayI                 v42, 0x0 -> (v51)
  227.i64  LoadArrayI                 v42, 0x1 -> (v51)
   51.i64  Add                        v225, v227 -> (v229, v40, v230)
  229.i64  StoreArrayI                v42, 0x0, v51
  230.i64  StoreArrayI                v42, 0x1, v51
   40.i64  Return                     v51
succs: [bb 1]
``` 
After optimization
```
BB 0
prop: start
    0.i64  Constant                   0x2a -> (v3)
succs: [bb 2]

BB 2  preds: [bb 0]
    3.ref  NewArray 77                v0 -> (v41, v42)
   41.     SaveState                  v3(vr7) -> (v42)
   42.ref  NullCheck                  v3, v41 -> (v231, v234)
  231.i64  LoadArrayPairI             v42, 0x0 -> (v232, v233)
  232.i64  LoadPairPart               v231, 0x0 -> (v51)
  233.i64  LoadPairPart               v231, 0x1 -> (v51)
   51.i64  Add                        v232, v233 -> (v234, v234, v40)
  234.i64  StoreArrayPairI            v42, 0x0, v51, v51
   40.i64  Return                     v51
succs: [bb 1]
```
### Coalescing inside loop
Before optimization
```
BB 2  preds: [bb 0]
    3.i32  LenArray                   v0 -> (v35)
succs: [bb 3]

BB 3  preds: [bb 2, bb 3]
prop: head, loop 1
   6p.i32  Phi                        v4(bb2), v34(bb3) -> (v33, v17, v34)
   7p.i32  Phi                        v5(bb2), v24(bb3) -> (v24, v17)
   8p.i32  Phi                        v5(bb2), v25(bb3) -> (v25, v23, v24)
   17.i32  StoreArray                 v0, v6p, v7p
   33.i32  AddI                       v6p, 0x1 -> (v23)
   23.i32  StoreArray                 v0, v33, v8p
   24.i32  Add                        v7p, v8p -> (v7p, v25)
   25.i32  Add                        v8p, v24 -> (v8p)
   34.i32  AddI                       v6p, 0x2 -> (v6p, v35)
   35.     If LT i32                  v34, v3
succs: [bb 3, bb 4]

BB 4  preds: [bb 3]
   29.void ReturnVoid
succs: [bb 1]
```
After optimization
```
BB 2  preds: [bb 0]
    3.i32  LenArray                   v0 -> (v35)
succs: [bb 3]

BB 3  preds: [bb 2, bb 3]
prop: head, loop 1
   6p.i32  Phi                        v4(bb2), v34(bb3) -> (v33, v36, v34)
   7p.i32  Phi                        v5(bb2), v24(bb3) -> (v36, v24)
   8p.i32  Phi                        v5(bb2), v25(bb3) -> (v36, v24, v25)
   33.i32  AddI                       v6p, 0x1
   36.i32  StoreArrayPair             v0, v6p, v7p, v8p
   24.i32  Add                        v7p, v8p -> (v7p, v25)
   25.i32  Add                        v8p, v24 -> (v8p)
   34.i32  AddI                       v6p, 0x2 -> (v6p, v35)
   35.     If LT i32                  v34, v3
succs: [bb 3, bb 4]

BB 4  preds: [bb 3]
   29.void ReturnVoid
succs: [bb 1]
```

## Options

| Option | Description | Default value |
| --- | --- | --- |
| `--compiler-memory-coalescing` | Enables optimization | `true` |
| `--compiler-memory-coalescing-objects` | Allows coalescing of operations with `ref`s | `true` |
| `--compiler-memory-coalescing-aligned` | Coalesces only aligned accesses (starting with even indices e.g. 0-1, 4-5 etc.) | `false` |

## Links

Source code:   
[memory_coalescing.cpp](../optimizer/optimizations/memory_coalescing.cpp)  
[memory_coalescing.h](../optimizer/optimizations/memory_coalescing.h)

Tests:  
[memory_coalescing_test.cpp](../tests/memory_coalescing_test.cpp)