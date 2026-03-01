# Deoptimize Elimination
## Overview
**Deoptimize Elimination** - optimization which try to reduce number of DeoptimizeIf instructions.

## Rationality
Reduce number of instructions and removes unnecessary data-flow dependencies.

## Dependences
* RPO
* DomTree

## Algorithm
Visit `DeoptimizeIf` and `SaveStateDeoptimize` instructions in RPO order and check specific rules.

### Rules
#### SaveStateDeoptimize
If `SaveStateDeoptimize` didn't have users, this instruction is replaced by `NOP`.

#### DeoptimizeIf
If input of `DeoptimizeIf` is constant:
* Constant is equal 0 -> `DeoptimizeIf` is replaced by `NOP`.
* Other constant -> `DeoptimizeIf` is replaced by `Deoptimize` instruction.


If input is `IsMustDeoptimize`:
    Run search recursively from current block to start block.
    We can remove guard (`IsMustDeoptimize` +  `DeoptimizeIf`), if guard is met in all ways and there should be no call instructions between current guard and found guards.

For another inputs, algorithm try to replaced by `NOP` all `DeoptimizeIf` instruction which is dominated by current instruction and have same input.

## Pseudocode
    TODO

## Examples
Before Deoptimize Elimination:
```
1. Constant 0
2. Constant 1

8. SaveStateDeoptimize 
3. DeoptimizeIf v1, v8
4. DeoptimizeIf v2, v8

9. SaveStateDeoptimize 
5. Some condition
6. DeoptimizeIf v5, v9

10. SaveStateDeoptimize 
7.  DeoptimizeIf v5, v10
```

After:
```
1. Constant 0
2. Constant 1

8. SaveStateDeoptimize 
3. NOP
4. Deoptimize v8

9. SaveStateDeoptimize 
5. Some condition
6. DeoptimizeIf v5, v9

10. NOP 
7.  NOP
```

## Links
Source code:   
[deoptimize_elimination.h](../optimizer/optimizations/deoptimize_elimination.h)  
[deoptimize_elimination.cpp](../optimizer/optimizations/deoptimize_elimination.cpp)

Tests:  
[deoptimize_elimination_test.cpp](../tests/deoptimize_elimination_test.cpp)