# Constant Folding
## Overview 
**Constant folding** - optimization which calculate instructions with constant inputs in compile time.
## Rationality

Reducing the number of instructions.

## Dependence 
 * RPO analyze.
## Algorithm
Visit all instructions in PRO order.

If instruction have constant inputs, new constant is calculate and input of user of original instruction is replaced on new constant.

Constant folding is called in Peephole optimization.
## Pseudocode
```
for (auto inst: All insts in RPO order) {
   if (inputs is constants) {
      Calculate new value.
      Putting new constant in inputs of inst users.
   }
}
```
## Examples
Before constant folding:
```
1.i64 Constant 1 -> v3
2.i64 Constant 2 -> v3

3.i64 Add v1, v2 -> v4 // is redundant, can calculate in compile time
4.i64 Return v3
```
After constant folding:
```
1.i64 Constant 1
2.i64 Constant 2
5.i64 Constant 3 -> v4

3.i64 Add v1, v2
4.i64 Return v5
```

## Links

Source code:   
[constant_folding.cpp](../optimizer/optimizations/const_folding.cpp)  
[constant_folding.h](../optimizer/optimizations/const_folding.h)  

Tests:  
[constant_folding_test.cpp](../tests/const_folding_test.cpp)