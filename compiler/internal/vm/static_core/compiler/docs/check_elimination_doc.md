# Checks Elimination
## Overview
**Checks Elimination** - optimization which try to reduce number of checks(NullCheck, ZeroCheck, NegativeCheck, BoundsCheck).

## Rationality
Reduce number of instructions and removes unnecessary data-flow dependencies.

## Dependences
* RPO
* BoundsAnalysis
* DomTree
* LoopAnalysis

## Algorithm
Visit all check instructions in RPO order and check specific rules.
If one of the rules is applicable, the instruction is deleted.

Instruction deleted in two steps:
1. Check instruction inputs connects to check users.
2. Check instruction replace on `NOP` instruction.

### Rules
#### All checks
All the same checks that are dominated by the current one are deleted and consecutive checks deleted.
#### NullCheck: 
1. NullChecks with allocation instruction input is deleted.
2. If based on bounds analysis input of NullCheck is more then 0, check is deleted.

Loop invariant NullChecks replaced by `DeoptimiseIf` instruction before loop.

#### ZeroCheck:
If based on bounds analysis input of ZeroCheck isn't equal than 0, check is deleted.
#### NegativeCheck: 
If based on bounds analysis input of NegativeCheck is more then 0, check is deleted.
#### BoundsChecks: 
If based on bounds analysis input of BoundsCheck is more or equal than 0 and less than length of array, check is deleted.

If BoundsCheck isn't deleted, it insert in special structure `NotFullyRedundantBoundsCheck`.
```
// parent_index->{Vector<bound_check>, max_val, min_val}
using GroupedBoundsChecks = ArenaUnorderedMap<Inst*, std::tuple<ArenaVector<Inst*>, int64_t, int64_t>>;
// loop->len_array->GroupedBoundsChecks
using NotFullyRedundantBoundsCheck = ArenaDoubleUnorderedMap<Loop*, Inst*, GroupedBoundsChecks>;
```
For example, for this method:
```
int Foo(array a, int index) {
  BoundCheck(len_array(a), index); // id = 1
  BoundCheck(len_array(a), index+1); // id = 2
  BoundCheck(len_array(a), index-2); // id = 3
  return a[index] + a[index+1] + a[index-2];
}
```
NotFullyRedundantBoundsCheck will be filled in this way:
```
Root_loop->len_array(a)-> index -> {{BoundsChecks 1,2,3}, max_val = 1, min_val = -2}
```

For countable loops witch index in `NotFullyRedundantBoundsCheck`, algorithm try to replace boundschecks by `DeoptimiseIf` instructions before loop.

For example, loop:
```
for ( int i = 0; i < x; i++) {
  BoundCheck(i);
  a[i] = 0;
}
```
will be transormed to
```
deoptimizeIf(x >= len_array(a));
for ( int i = 0; i < x; i++) {
  a[i] = 0;
}

```

For another BoundsCheck instructions in `NotFullyRedundantBoundsCheck` algorithm try to replace more then 2 grouped bounds checks by `DeoptimiseIf`.
For example, this method:
```
int Foo(array a, int index) {
  BoundCheck(len_array(a), index); // id = 1
  BoundCheck(len_array(a), index+1); // id = 2
  BoundCheck(len_array(a), index-2); // id = 3
  return a[index] + a[index+1] + a[index-2];
}
```
will be transformed to:
```
int Foo(array a, int index) {
  deoptimizeIf(index-2 < 0);
  deoptimizeIf(index+1 >= len_array(a));
  return a[index] + a[index+1] + a[index-2];
}
```

## Pseudocode
  TODO

## Examples

Before Checks Elimination:
```
1.ref Parameter  -> v3, v4
2.i64 Constant 1 -> v5, v6

3.ref NullCheck v1 -> v7
4.ref NullCheck v1 -> v8
5.i32 ZeroCheck v2 -> v9
6.i32 NegativeCheck v2 -> v10 
```
After Checks Elimination:
```
1.ref Parameter  -> v3
2.i64 Constant 1 -> v9, v10

3.ref NullCheck v1 -> v7, v8
4.ref NOP
5.i32 NOP
6.i32 NOP
```

## Links
Source code:   
[checks_elimination.h](../optimizer/optimizations/checks_elimination.h)  
[checks_elimination.cpp](../optimizer/optimizations/checks_elimination.cpp)

Tests:  
[checks_elimination_test.cpp](../tests/checks_elimination_test.cpp)