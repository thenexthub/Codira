# StringFlatCheck instruction insertion

## Overview
Many string intrinsics work with flat string only.
For such intrinsics inserts StringFlatCheck IR instruction between String input and specific intrinsic.
If several intrinsics use the same argument, moves corresponding StringFlatCheck to the common
dominating basic block if it has SaveState.

## Rationality
Avoid going to slowpath if flattening is performed in IRTOC implementation.

## Dependence
* Loop Analysis;
* Dominators Tree;

## Algorithm
1. For each intrinsics argument which requires flattening try to find existing StringFlatCheck user and
try to move it to common dominating block. If StringFlatCheck is not found or it cannot be moved
insert new StringFlatCheck instruction.
2. Update users of original input to use StringFlatCheck instruction.

## Examples
```
BB 0
prop: start, bc: 0x00000000
hotness=0
    0.ref  Parameter                  arg 0 -> (v45, v40, v77, v9)
    1.i32  Parameter                  arg 1 -> (v10)
    2.i32  Parameter                  arg 2
succs: [bb 2]

BB 2  preds: [bb 0]
prop: bc: 0x00000000
hotness=0
    40.     SaveState                  v0(vr2), inlining_depth=0 -> (v41, v42)
    41.void CallStatic.Inlined 4294967295 v40
    8.     SaveState                  v0(vr2), caller=41, inlining_depth=1 -> (v9)
    9.ref  NullCheck                  v0, v77 -> (v43)
    45.     SaveState                  v0(vr2), caller=41, inlining_depth=1 -> (v43)
    43.ref  StringFlatCheck            v9, v45 -> (v44, v10)
    44.     SaveState                  v43(vr2), caller=41, inlining_depth=1 -> (v10)
    10.ref  Intrinsic.StdCoreStringRepeat v43, v1, v44 -> (v11)
    42.     ReturnInlined              v40
    11.ref  Return                     v10
succs: [bb 1]
```
