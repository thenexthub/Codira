# Loop-invariant code motion (LICM)

## Overview
`LICM` moves instructions outside the body of a loop without affecting the program semantics

## Rationality
Instructions which have been hoisted out of a loop are executed less often, providing a speedup

## Dependence
* Loop Analysis;
* Dominators Tree;

## Algorithm 
`LICM` visits loops from inner to outer, skipping irreducible and OSR loops. In each loop it selects instructions, which can be hoisted:
* all instruction's inputs should dominate loop's pre-header or they are selected to hoist; 
* instruction must dominate all loop exits;

Then selected instructions are moved to the loop pre-header
## Pseudocode
TBD
## Examples
```
BB 0
prop: start
   12.i64  Constant                   0x1 -> (v3p, v13, v13, v5)
    1.i64  Constant                   0xa -> (v4p)
    2.i64  Constant                   0x14 -> (v10)
succs: [bb 2]

BB 2  preds: [bb 6, bb 3]
prop: head, loop 1
   3p.u64  Phi                        v12(bb6), v7(bb3) -> (v7, v10)
   4p.u64  Phi                        v1(bb6), v8(bb3) -> (v5, v7, v8)
    5.b    Compare EQ u64             v4p, v12 -> (v6)
    6.     IfImm NE b                 v5, 0x0
succs: [bb 3, bb 4]

BB 3  preds: [bb 2]
prop: loop 1
    7.u64  Mul                        v3p, v4p -> (v3p)
   13.u64  Mul                        v12, v12 -> (v8)
    8.u64  Sub                        v4p, v13 -> (v4p)
succs: [bb 2]


BB 4  preds: [bb 2]
   10.u64  Add                        v2, v3p
   11.     ReturnVoid
succs: [bb 1]
```
`LICM` hoists `13.u64 Mul` instruction:

```
BB 0
prop: start
   12.i64  Constant                   0x1 -> (v3p, v13, v13, v5)
    1.i64  Constant                   0xa -> (v4p)
    2.i64  Constant                   0x14 -> (v10)
succs: [bb 6]

BB 6  preds: [bb 0]
   13.u64  Mul                        v12, v12 -> (v8)
succs: [bb 2]

BB 2  preds: [bb 6, bb 3]
prop: head, loop 1
   3p.u64  Phi                        v12(bb6), v7(bb3) -> (v7, v10)
   4p.u64  Phi                        v1(bb6), v8(bb3) -> (v5, v7, v8)
    5.b    Compare EQ u64             v4p, v12 -> (v6)
    6.     IfImm NE b                 v5, 0x0
succs: [bb 3, bb 4]

BB 3  preds: [bb 2]
prop: loop 1
    7.u64  Mul                        v3p, v4p -> (v3p)
    8.u64  Sub                        v4p, v13 -> (v4p)
succs: [bb 2]

BB 4  preds: [bb 2]
   10.u64  Add                        v2, v3p
   11.     ReturnVoid
succs: [bb 1]

```

## Links
Source code:   
[licm.cpp](../optimizer/optimizations/licm.cpp)  
[licm.h](../optimizer/optimizations/licm.h)  

Tests:  
[licm_test.cpp](../tests/licm_test.cpp)  