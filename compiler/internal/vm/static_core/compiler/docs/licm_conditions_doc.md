# Loop-invariant code motion for conditions chains

## Overview
`LICM` moves instructions outside the body of a loop. But complex conditions are encoded as basic blocks cascades (chains) and are not target of `LICM` pass. `LICM conditions` moves such condition chains outside of a loop.

## Rationality
Conditions which have been hoisted out of a loop are executed less often, providing a speedup

## Dependence
* Loop Analysis;
* Dominators Tree;
* LICM;

## Algorithm
Condition chain is graph which looks like
```
          | |
          v v
          [A]------\
           |       |
           |       v
           |<-----[B]
           |       |
           v       v
       -->[S0]    [S1]<--
           |       |
           v       v
```
Basic blocks A & B is a condition chain.
Each basic block in the chain has S0 successor which is called `multiple_predecessor_successor' (all chain basic blocks are predecessors).
Only last basic block in chain (B) has S1 successor which is called `single_predecessor_successor`.
Both S0 and S1 successors can have predeccessors which are not part of the chain.

`LICM conditions` visits loops from inner to outer, skipping irreducible and OSR loops. In each loop it finds condition chains, which can be hoisted:
* all IfImm's inputs should dominate loop's pre-header;
* Phis in condition chain successors must allow hoisting of condition chain basic blocks.

Then selected condition chains are removed from loop and inserted after loop pre-header.

## Examples
Loop 1 contains condition chain BB4, BB6.
```
BB 0
prop: start, bc: 0x00000000
    0.b    Parameter                  arg 0 -> (v13)
    1.b    Parameter                  arg 1 -> (v14)
    2.i64  Constant                   0x64 -> (v7)
    3.i64  Constant                   0x0 -> (v5p, v6p)
    4.i64  Constant                   0x1 -> (v12)
succs: [bb 14]

BB 14  preds: [bb 0]
prop: prehead, bc: 0x00000000
succs: [bb 2]

BB 2  preds: [bb 14, bb 8]
prop: head, loop 1, depth 1, bc: 0x00000000
   5p.i64  Phi                        v3(bb14), v11p(bb8) -> (v9, v10, v15)
   6p.i64  Phi                        v3(bb14), v12(bb8) -> (v7, v9, v10, v12)
    7.b    Compare GE i64             v6p, v2 -> (v8)
    8.     IfImm NE b                 v7, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2]
prop: loop 1, depth 1, bc: 0x00000000
   13.     IfImm EQ b                 v0, 0x0
succs: [bb 5, bb 6]

BB 6  preds: [bb 4]
prop: loop 1, depth 1, bc: 0x00000000
   14.     IfImm EQ b                 v1, 0x0
succs: [bb 5, bb 7]

BB 7  preds: [bb 6]
prop: loop 1, depth 1, bc: 0x00000000
   10.i64  Sub                        v5p, v6p -> (v11p)
succs: [bb 8]

BB 5  preds: [bb 4, bb 6]
prop: loop 1, depth 1, bc: 0x00000000
    9.i64  Add                        v5p, v6p -> (v11p)
succs: [bb 8]

BB 8  preds: [bb 5, bb 7]
prop: loop 1, depth 1, bc: 0x00000000
  11p.i64  Phi                        v9(bb5), v10(bb7) -> (v5p)
   12.i64  Add                        v6p, v4 -> (v6p)
succs: [bb 2]

BB 3  preds: [bb 2]
prop: bc: 0x00000000
   15.i64  Return                     v5p
succs: [bb 1]

BB 1  preds: [bb 3]
prop: end, bc: 0x00000000
```

BB4 and BB6 are hoisted and replaced with singe block in the loop.
```
BB 0
prop: start, bc: 0x00000000
    0.b    Parameter                  arg 0 -> (v13)
    1.b    Parameter                  arg 1 -> (v14)
    2.i64  Constant                   0x64 -> (v7)
    3.i64  Constant                   0x0 -> (v16p, v5p, v6p)
    4.i64  Constant                   0x1 -> (v16p, v16p, v12)
succs: [bb 14]

BB 14  preds: [bb 0]
prop: prehead, bc: 0x00000000
succs: [bb 4]

BB 4  preds: [bb 14]
prop: loop 1, depth 1, bc: 0x00000000
   13.     IfImm EQ b                 v0, 0x0
succs: [bb 16, bb 6]

BB 6  preds: [bb 4]
prop: loop 1, depth 1, bc: 0x00000000
   14.     IfImm EQ b                 v1, 0x0
succs: [bb 16, bb 17]

BB 17  preds: [bb 6]
succs: [bb 16]

BB 16  preds: [bb 4, bb 6, bb 17]
  16p.b    Phi                        v4(bb4), v4(bb6), v3(bb17) -> (v17)
succs: [bb 2]

BB 2  preds: [bb 16, bb 8]
prop: head, loop 1, depth 1, bc: 0x00000000
   5p.i64  Phi                        v3(bb16), v11p(bb8) -> (v9, v10, v15)
   6p.i64  Phi                        v3(bb16), v12(bb8) -> (v7, v9, v10, v12)
    7.b    Compare GE i64             v6p, v2 -> (v8)
    8.     IfImm NE b                 v7, 0x0
succs: [bb 3, bb 15]

BB 15  preds: [bb 2]
   17.     IfImm NE b                 v16p, 0x0                                                        bc: 0x00000000
succs: [bb 5, bb 7]

BB 7  preds: [bb 15]
prop: loop 1, depth 1, bc: 0x00000000
   10.i64  Sub                        v5p, v6p -> (v11p)
succs: [bb 8]

BB 5  preds: [bb 15]
prop: loop 1, depth 1, bc: 0x00000000
    9.i64  Add                        v5p, v6p -> (v11p)
succs: [bb 8]

BB 8  preds: [bb 5, bb 7]
prop: loop 1, depth 1, bc: 0x00000000
  11p.i64  Phi                        v9(bb5), v10(bb7) -> (v5p)
   12.i64  Add                        v6p, v4 -> (v6p)
succs: [bb 2]

BB 3  preds: [bb 2]
prop: bc: 0x00000000
   15.i64  Return                     v5p
succs: [bb 1]

BB 1  preds: [bb 3]
prop: end, bc: 0x00000000
```
More examples in unit tests.

## Links
Source code:
[licm_conditions.cpp](../optimizer/optimizations/licm_conditions.cpp)
[licm_conditions.h](../optimizer/optimizations/licm_conditions.h)

Tests:
[licm_conditions_test.cpp](../tests/licm_conditions_test.cpp)
