# SaveState Bridges

## Overview
This is a tool that can be used in optimizations that can break correct state of SaveStates.

`SaveStateBridges` has several use cases:
1. Fix `SaveStates` for all object in BasicBlock
2. Fix `SaveStates` for `source` instruction **ONLY** on path from `source` instructions to `target` instruction.

## Rationality
Some optimisations can break correct state of `SaveStates` and if in this case GC is triggered between incorrect SaveState and usage then we will lost object - we will not find out about his movement and will load him at the wrong address.

## Dependence
We need to be sure that the `source` instruction dominates the `target` instruction.

## Algorithm
We bypass the graph in the opposite direction from the `target` instruction to the `source` and we are looking for SS that need to be fixed. We always can do it, because we sure, that the `source` instruction dominates `target`.

## Usage
Create **`SaveStateBridgesBuilder`** in place where during all time of the pass object will be live. For example, in `.h` file of pass. Functions below are class methods.

**`SearchAndCreateMissingObjInSaveState`** includes `SearchMissingObjInSaveStates` and, if it is required, `CreateBridgeInSS`. It inserts `source` instruction into `SaveStates` on path in each SaveState between `source` and `target` instructions to save the object in case GC is triggered on this path.

**`FixSaveStatesInBB`** fixes all SaveStates in the BasicBlock, if necessary. Searches for the use of objects (reference) and their definition, and on this path enters the object into the SaveState inputs if it is not there. Delete object from the SaveState, if instruction does not dominate this input. Requires a BasicBlock and works within its boundaries. (Despite this, it takes into account the definition and usage outside of this block.)

**`SearchMissingObjInSaveStates`** is using for search SaveState on path to `target`, which don't have `source` instruction in input. Return `ArenaVector<Inst *> *` all of these SaveState. If is empty, bridges are not needed. As an *optional* parameters takes `stop_search`, which restricts the search for SaveState when traversing backwards from `target`, and `target_block` (for the case when we need to build bridges until the end of an empty block, so `target` is `nullptr`).

**`CreateBridgeInSS`** is using for append `source` as a bridge in all SaveStates from `ArenaVector<Inst *> *` received above.

**`DumpBridges`** write in your `std::ostream` all bridges which need add for this `source` instruction.

## Simple example of work without any optimization

### Before:

Receiving and using is at a distance, and the object is not recorded in the intermediate SaveState. This is an incorrect graph, because after SaveState v4 or SaveState v7 GC can be triggered. So obj `1.ref` can be deleted or moved, but the pointer will not change. As a result, in instruction v10 we can use the object at an **invalid address**.
```
    1.ref  Intrinsic.GET        (v10)
    ...
    4.     SaveState            ...
    ...
    7.     SaveState            ...
    ...
    10.    Intrinsic.USE        v1
```

### After `SaveState Bridges`:
Here the tool corrected SaveState thus restored the object's safety.

```
    1.ref  Intrinsic.GET        (v10)
    ...
    4.     SaveState            v1(bridge), ...
    ...
    7.     SaveState            v1(bridge), ...
    ...
    10.    Intrinsic.USE        v1
```

## Example in VN

Instrinsic below with clear flag `NO_CSE`.

Before VN:

```
    0.ref  Intrinsic.GET        (v1)   <==|
    1.void Intrinsic.USE        v0        |
    2.     SaveState            -> ...    |
    3.ref  Intrinsic.GET        (v4)   <==| Inst v3 is equal inst v0,
    4.void Intrinsic.USE        v3        | they return the same reference
```

After VN with bridges + Cleanup:
```
    0.ref  Intrinsic.GET        (v4, v2, v1)
    1.void Intrinsic.USE        v0
    2.     SaveState            v0(bridge) -> ...  <==| Added bridge on way
                                                      | to target instruction
                                        <==| VN delete inst v3
    4.void Intrinsic.USE        v0
```

## Links

Source code:
[analysis.h](../optimizer/ir/analysis.h)
[analysis.cpp](../optimizer/ir/analysis.cpp)

Tests:
[Tests for GVN](../tests/vn_test.cpp)
[Tests for Scheduler](../tests/scheduler_test.cpp)
