# Loop Unswitch

## Overview
`Loop Unswitch` optimization moves a conditional inside a loop outside of it by duplicating the loop's body, and placing a version of it inside each of the if and else clauses of the conditional.

## Rationality
Reduce branches count in the loop's body. Each of new loops can be separately optimized.

## Dependence
* Loop Analysis
* Dominators Tree
* LICM
* LICM Conditions

## Algorithm
`Loop Unswitch` starts with original loop and iterates on each new loop (which was created while unswitching) until max level of unswitching or instructions limit is reached. 
On each iteration:
* find branch instruction where loop can be unswitched
* clone loop
* move branch instruction outside a loop
* original branch instructions are replaced with constant conditions corresponding `true` and `false` branches in original loop

Optimization settings:
* Max loop unswitch level
* Max loop unswitch instructions

## Examples
Replace
```
for (i = 0; i < 100; i++) {
  x[i] += y[i];
  if (w)
    y[i] = 0;
}
```
with
```
if (w) {
  for (i = 0; i < 100; i++) {
    x[i] += y[i];
    if (true)
      y[i] = 0;
  }
} else {
  for (i = 0; i < 100; i++) {
    x[i] += y[i];
    if (false)
      y[i] = 0;
  }
}
```
Constant conditions should be eliminated with `Branch elimination` optimization.

## Links
Source code:
[loop_unswitch.cpp](../optimizer/optimizations/loop_unswitch.cpp)
[loop_unswitch.h](../optimizer/optimizations/loop_unswitch.h)

Tests:
[loop_unswitch_test.cpp](../tests/loop_unswitch_test.cpp)
