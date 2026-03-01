# Branch Elimination
## Overview 

`Branch Elimination` searches condition statements which result is known at compile-time and removes not reachable branches.

## Rationality

Reduce number of instructions and simplify control-flow.

## Dependence 

* DominatorsTree
* LoopAnalysis
* Reverse Post Order (RPO)

## Algorithm

`Branch Elimination` optimization searches `if-true` blocks with resolvable conditional instruction.
Condition can be resolved in the following ways:
- Condition is constant:
```
             T           F
           /---[ 1 > 0 ]---\
           |               |
           |               |
           v               v 
                    can be eliminated
```
- Condition is dominated by the equal one condition with the same inputs and the only one successor of the dominant reaches dominated condition

```
             T           F
           /---[ a > b ]---\
           |               | 
           |         T     v     F
          ...      /---[ a > b ]---\
           |       |               |
           v       v               v
              can be eliminated

             T           F
           /---[ a > b ]---\
           |               | 
          ...              |
           |               |
           \-------------->|               
                     T     v     F
                   /---[ a > b ]---\
                   |               |
                   v               v
            can't be eliminated - reachable from both successors 
```
To resolve the condition result by the dominant one we use table, keeps the result of the condition in the row if the condition in the column is true:
```
    /*           CC_EQ    CC_NE    CC_LT    CC_LE    CC_GT    CC_GE    CC_B     CC_BE    CC_A     CC_AE */
    /* CC_EQ */ {true,    false,   false,   nullopt, false,   nullopt, false,   nullopt, false,   nullopt},
    /* CC_NE */ {false,   true,    true,    nullopt, true,    nullopt, true,    nullopt, true,    nullopt},
    /* CC_LT */ {false,   nullopt, true,    nullopt, false,   false,   nullopt, nullopt, nullopt, nullopt},
    /* CC_LE */ {true,    nullopt, true,    true,    false,   nullopt, nullopt, nullopt, nullopt, nullopt},
    /* CC_GT */ {false,   nullopt, false,   false,   true,    nullopt, nullopt, nullopt, nullopt, nullopt},
    /* CC_GE */ {true,    nullopt, false,   nullopt, true,    true,    nullopt, nullopt, nullopt, nullopt},
    /* CC_B  */ {false,   nullopt, nullopt, nullopt, nullopt, nullopt, true,    nullopt, false,   false},
    /* CC_BE */ {true,    nullopt, nullopt, nullopt, nullopt, nullopt, true,    true,    false,   nullopt},
    /* CC_A  */ {false,   nullopt, nullopt, nullopt, nullopt, nullopt, false,   false,   true,    nullopt},
    /* CC_AE */ {true,    nullopt, nullopt, nullopt, nullopt, nullopt, false,   nullopt, true,    true},
```
## Pseudocode

TBD

## Examples

```cpp
              [0]
            T  |  F
         /----[2]----\
         |           |
         v        T  v  F
        [3]    /----[4]----\
         |     |           |
         |    [5]         [6]
         |     |           |
         v     v           |
       [exit]<-------------/


    GRAPH(graph) {
        PARAMETER(0, 0).u64();
        PARAMETER(1, 1).u64();
        PARAMETER(2, 2).u64();

        BASIC_BLOCK(2, 3, 4) {
            INST(19, Opcode::Compare).b().CC(CC_EQ).Inputs(0, 1);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(19);
        }
        BASIC_BLOCK(3, 7) {
            INST(5, Opcode::Add).u64().Inputs(0, 1);
            INST(6, Opcode::Add).u64().Inputs(5, 2);
        }
        BASIC_BLOCK(4, 5, 6) {
            INST(8, Opcode::SaveState).Inputs(0, 1, 2).SrcVregs({0, 1, 2});
            INST(9, Opcode::Compare).b().CC(CC_NE).Inputs(0, 1);
            INST(10, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(9);
        }
        BASIC_BLOCK(5, 7) {
            INST(11, Opcode::Sub).u64().Inputs(0, 1);
            INST(12, Opcode::Sub).u64().Inputs(11, 2);
        }
        BASIC_BLOCK(6, 7) {
            INST(14, Opcode::Mul).u64().Inputs(0, 1);
            INST(15, Opcode::Mul).u64().Inputs(14, 2);
        }
        BASIC_BLOCK(7, -1) {
            INST(17, Opcode::Phi).u64().Inputs(6, 12, 15);
            INST(18, Opcode::Return).u64().Inputs(17);
        }
    }

```
We reaches basic block `4` when the condition `PARAMETER(0) == PARAMETER(1)` is false, so that the result of instruction `INST(9)` always true.
No we can eliminate basic block `6` and conditional statement form basic block `4`:

```
              [0]
            T  |  F
         /----[2]----\
         |           |
         v           v 
        [3]         [4]
         |           |
         |          [5]
         |           |
         v           |
       [exit]<-------/
```
## Links

Source code:  
[branch_elimination.cpp](../optimizer/optimizations/branch_elimination.cpp)  
[branch_elimination.h](../optimizer/optimizations/branch_elimination.h)

Tests:  
[branch_elimination_test.cpp](../tests/branch_elimination_test.cpp)