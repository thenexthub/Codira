# IfMerging
## Overview

`IfMerging` searches condition statements results of which are known from previous conditions and removes these conditions,
linking their branches to corresponding input blocks.

## Rationality

Reduce number of instructions and simplify control-flow.

## Dependence

* DominatorsTree
* Reverse Post Order (RPO)

## Algorithm

`IfMerging` optimization searches `if` blocks with resolvable conditional instruction.
Condition can be resolved in the following ways:

1. Condition is comparison of Phi instruction with constant inputs and some constant. In this case
for each input block of Phi instruction the branch of condition can be resolved, and If instruction can
be removed.

Before:
```
      ...    ...     ....
       |c1    |c2     |c3
       |      |       |
       \------+-------/
              |
    T         v           F
  /--[Phi(c1,c2,c3) == c1]--\
  |                         |
  v                         v
 [2]                       [3]
```

After:
```
      ...    ...     ....
       |c1    |c2     |c3
       |      |       |
       |      \---+---/
       |          |
       v          v
      [2]    [Phi(c2, c3)]

```

2. Condition is dominated by an equivalent or inverted condition, and branches of the dominating
condition don't intersect. In this case branches of the second condition can be connected to
corresponding branches of the first condition, and the second If instruction can be removed.

Before:
```
               T          F
           /--[a is true?]--\
           |                |
          ...              ...
           |                |
           \--------------->|
                     T      v     F
                   /--[a is true?]--\
                   |                |
                  [2]              [3]
                   |                |
                   v                v
```

After:
```
             T            F
           /--[a is true?]--\
           |                |
          ...              ...
           |                |
           v                v
          [2]              [3]
           |                |
           v                v
```

During the optimization dominators in dominators tree are adjusted so that they are still dominators,
but not necessary immediate.

## Pseudocode

```
for (auto bb : GetGraph()->GetVectorBlocks()) {
    if (TryMergeEquivalentIfs(block) || TrySimplifyConstantPhi(block)) {
        is_applied_ = true;
    }
}

bool TryMergeEquivalentIfs(BasicBlock *bb) {
    auto dom = bb->GetDominator();
    if ((dom and bb end with same Ifs) && (true and false branches of dom meet only in bb)) {
        remove If from bb
        split bb into two blocks
    }
    return is_applied;
}

bool IfMerging::TrySimplifyConstantPhi(BasicBlock *bb)
{
    auto is_applied = false;
    while (TryRemoveConstantPhiIf(bb)) {
        is_applied = true;
    }
    return is_applied;
}

bool TryRemoveConstantPhiIf(BasicBlock *bb) {
    auto if = bb->GetLastInst();
    auto [phi, cnst] = if->GetInputs();
    find phi inputs for which compare with cnst gives true and link their blocks to new branch
    return is_applied;
}
```

## Examples

Before:
```
BB 0
prop: start, bc: 0x00000000
    0.u64  Parameter                  arg 0 -> (v3)
    1.i64  Constant                   0x0 -> (v9p, v3, v6, v12)
    2.i64  Constant                   0x1 -> (v9p, v8, v10, v13)
succs: [bb 2]

BB 2  preds: [bb 0]
prop: bc: 0x00000000
    3.b    Compare EQ u64             v0, v1 -> (v4)
    4.     IfImm NE b                 v3, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2]
prop: bc: 0x00000000
    7.     SaveState                   -> (v8)
    8.void CallStatic 4294967295       v2, v7
succs: [bb 5]

BB 3  preds: [bb 2]
prop: bc: 0x00000000
    5.     SaveState                   -> (v6)
    6.void CallStatic 4294967295       v1, v5
succs: [bb 5]

BB 5  preds: [bb 3, bb 4]
prop: bc: 0x00000000
   9p.u64  Phi                        v1(bb3), v2(bb4) -> (v10)
   10.b    Compare EQ u64             v9p, v2 -> (v11)
   11.     IfImm NE b                 v10, 0x0
succs: [bb 6, bb 7]

BB 7  preds: [bb 5]
prop: bc: 0x00000000
   13.b    Return                     v2
succs: [bb 1]

BB 6  preds: [bb 5]
prop: bc: 0x00000000
   12.b    Return                     v1
succs: [bb 1]
```

After `IfMerging` and `Cleanup`:
```
BB 0
prop: start, bc: 0x00000000
    0.u64  Parameter                  arg 0 -> (v3)
    1.i64  Constant                   0x0 -> (v3, v6, v12)
    2.i64  Constant                   0x1 -> (v8, v13)
succs: [bb 2]

BB 2  preds: [bb 0]
prop: bc: 0x00000000
    3.b    Compare EQ u64             v0, v1 -> (v4)
    4.     IfImm NE b                 v3, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2]
prop: bc: 0x00000000
    7.     SaveState                   -> (v8)
    8.void CallStatic 4294967295       v2, v7
   12.b    Return                     v1
succs: [bb 1]

BB 3  preds: [bb 2]
prop: bc: 0x00000000
    5.     SaveState                   -> (v6)
    6.void CallStatic 4294967295       v1, v5
   13.b    Return                     v2
succs: [bb 1]
```

## Links

Source code:
[if_merging.cpp](../optimizer/optimizations/if_merging.cpp)
[if_merging.h](../optimizer/optimizations/if_merging.h)

Tests:
[if_merging_test.cpp](../tests/if_merging_test.cpp)
