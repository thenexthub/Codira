# Redundant Loop Elimination
## Overview 
**Redundant Loop Elimination(RLE)** - optimization which find and remove useless loops.
## Rationality
Reducing number of basic blocks and instructions.
## Dependence 
* Loop analysis
## Algorithm
Visit loops in LRN order (first children, then parent).
For each loop check that:
* RLE is applied for all children loops.
* Loop doesn't contain instructions with side effect (ex. call instructions).
* Loop doesn't contain instructions with users out of the loop.

If all checks are true then loop is removing:
1. Loop pre-header connect with loop outer block.
2. Loop inner blocks disconnect from graph. 
## Pseudocode
```
LoopVisitLRN(Loop* loop) {
    for (auto inner_loop : loop->GetInnerLoops()) {
       LoopVisitLRN(inner_loop);
    }
    if (Check(loop)) {
       Remove(loop);
    }
}
```
## Examples
Before RLE:
```
BB 0
prop: start
    0.i64  Constant                   0x0 -> (v4p)                                                                
    1.i64  Constant                   0x1 -> (v10)                                                                
    2.i64  Constant                   0xa -> (v5)                                                                
succs: [bb 3]

BB 3  preds: [bb 0, bb 4]
prop: head, loop 1
   4p.i32  Phi                        v0(bb8), v10(bb4) -> (v5, v10)                                                                
    5.b    Compare LT i32             v4p, v2 -> (v6)                                                                
    6.     IfImm NE b                 v5, 0x0                                                                
succs: [bb 4, bb 5]

BB 5  preds: [bb 3]
   12.     ReturnVoid                                                                                 
succs: [bb 1]

BB 1  preds: [bb 5]
prop: end

BB 4  preds: [bb 3]
prop: loop 1
   10.i32  Add                        v4p, v1 -> (v4p)                                                                
succs: [bb 3]
```
After RLE:
```
BB 0
prop: start
    0.i64  Constant                   0x0                                                                
    1.i64  Constant                   0x1                                                                
    2.i64  Constant                   0xa                                                                
succs: [bb 5]

BB 5  preds: [bb 0]
   12.     ReturnVoid                                                                                 
succs: [bb 1]

BB 1  preds: [bb 5]
prop: end
```
## Links

Source code:   
[redundant_loop_elimination.cpp](../optimizer/optimizations/redundant_loop_elimination.cpp)  
[redundant_loop_elimination.h](../optimizer/optimizations/redundant_loop_elimination.h)  

Tests:  
[redundant_loop_elimination_test.cpp](../tests/redundant_loop_elimination_test.cpp)