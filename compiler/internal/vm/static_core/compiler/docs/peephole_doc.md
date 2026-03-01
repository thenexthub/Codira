# Peepholes
## Overview 
**Peepholes** - optimizations which try to apply some small rules for instruction.
## Rationality

Reducing the number of instructions.

## Dependence 

* RPO analyze.

## Algorithm

Visit all instructions in PRO order.

For instruction tried to apply some rules.

Peephole not remove instruction, it's only replace it on another and replace users on new instruction.

### Rules
This some of the rules:
* Constant folding
* Partial constant instruction replace by constant (a *= 0 -> a = 0)
* Putting constant input on second place for commutative instructions (ex. Add, Mul, ...)
* Grouping instructions (ex. b=a+2, c=b-4 -> c=a-2)
* Remove redundant instructions (ex. b=a+0, b=a&1)
* Replace instructions for equal but more cheap (ex. a*=4 - > a<<=2, b*=-1 -> b = -b )
* De Morgan rules for `and` and `or` instructions.
*...
## Pseudocode
```
for (auto inst: All insts in RPO order) {
   try to apply rules
}
```
## Examples
Before peepholes:
```
1.i64 Parameter  -> v6      // x
2.i64 Parameter  -> v8      // y
3.i64 Constant -1 -> v10    // -1
4.i64 Constant 1 -> v6, v7  // 1
5.i64 Constant 2 -> v8      // 2

6. Add v1, v4 -> v7         // x+1
7. Add v6, v4 -> v9         // (x+1)+1
8. Mul v2, v5 -> v9         // y*2
9. Mul v7, v8 -> v10        // ((x+1)+1)*(y*2)
10. Mul v9, v3 -> v11       // (((x+1)+1)*(y*2))*-1
11. Return v10
```
After peepholes:
```
1.i64 Parameter  -> v6, v12  // x
2.i64 Parameter  -> v8, v13  // y
3.i64 Constant -1 -> v10     // -1
4.i64 Constant 1 -> v6, v7   // 1
5.i64 Constant 2 -> v8, v12  // 2

// (x+1)+1 -> x+2
6. Add v1, v4                // x+1
7. Add v6, v4                // (x+1)+1
12.Add v1, v5 -> v9          // x+2

//y*2 -> y + y
8. Mul v2, v5                // y*2
13.Add v2, v2 -> v9          // y+y

9. Mul v12, v13 -> v14       // (x+2)*(y+y)

// z*-1 -> -z
10.Mul v9, v3                // (x+2)*(y+y)*(-1)
14.Neg v9 -> v11             // -((x+2)*(y+y))

11.Return v14
```
## Links
[constant folding](constant_folding_doc.md)  

Source code:   
[peepholes.cpp](../optimizer/optimizations/peepholes.cpp)  
[peepholes.h](../optimizer/optimizations/peepholes.h)  

Tests:  
[peepholes_test.cpp](../tests/peepholes_test.cpp)

