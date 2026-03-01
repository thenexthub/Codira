# Move constants closer to usage

## Overview

Previously compiler defined all constants in a start block. It leads to several performance issues:
- if some of the constants are used in a branch that is not reachable during particular execution then there is no need to execute code initializing these constants;
- since constants have relatively large live intervals register allocator may spill them to stack;
- constants residing in registers may cause excessive caller registers load/stores around method calls.

## Rationality

Avoid unnecessary constants definition and excessive caller registers load/stores around method calls. Reduce live interval to help to register allocator.

## Algorithm

Before register allocation for each constant try to move it closer to usage:
- If there is no Phi instruction among users and all users are located in the same block move constant just before first user
- Otherwise move constant to closest common dominating block

In both cases avoid moving into loop, find closest dominating block outside loop.

## Examples

IR before optimization

```
BB 3
prop: start
    0.i32  Parameter                  arg 0 -> (v150, v149, v148, v147, v146, v145, v144, v143, v142, v141, v140, v139, v138, v137, v136, v135, v134, v133, v132, v131, v130, v129, v128, v127, v126, v125, v124, v123, v122, v121, v120, v119, v118, v117, v116, v3, v5)
PUSH r1 -> s0
    6.i64  Constant                   0xa -> r0 (v5)
    8.i64  Constant                   0x14 -> r2 (v116)
...
// many constants definitions
...
   52.i64  Constant                   0x3e -> r27 (v138)
  188.     SpillFill                  PUSH r27 -> s1
...
// causes many SpillFills
...
   76.i64  Constant                   0x5d -> r27 (v150)
  200.     SpillFill                  PUSH r27 -> s13
succs: [bb 0]

BB 0  preds: [bb 3]
prop: bc: 0x00000000
  151.     SpillFill                  POP s0 -> r27
    3.     IfImm LE i32               v0(r27), 0x5                                                     bc: 0x00000004
succs: [bb 1, bb 2]

BB 2  preds: [bb 0]
prop: bc: 0x00000006
  114.i32  ReturnI                    0x5                                                              bc: 0x00000008
succs: [bb 4]

BB 1  preds: [bb 0]
prop: bc: 0x00000009
  152.     SpillFill                  POP s0 -> r27
    5.i32  Mul                        v0(r27), v6(r0) -> r0 (v116)                                     bc: 0x0000000b
  153.     SpillFill                  POP s0 -> r27
  116.i32  MAdd                       v0(r27), v8(r2), v5(r0) -> r0 (v117)                             bc: 0x000000e3
  154.     SpillFill                  POP s0 -> r27
  117.i32  MAdd                       v0(r27), v10(r3), v116(r0) -> r0 (v118)                          bc: 0x000000e9
...
// many SpillFills while use constants
...
  155.     SpillFill                  POP s0 -> r27
  150.i32  MAdd                       v0(r27), v76(r26), v149(r0) -> r0 (v112)                         bc: 0x000001af
  112.i32  Return                     v150(r0)                                                         bc: 0x000001b5
succs: [bb 4]

BB 4  preds: [bb 2, bb 1]
prop: end
```

IR after optimization

```
BB 3
prop: start
    0.i32  Parameter                  arg 0 -> (v150, v149, v148, v147, v146, v145, v144, v143, v142, v141, v140, v139, v138, v137, v136, v135, v134, v133, v132, v131, v130, v129, v128, v127, v126, v125, v124, v123, v122, v121, v120, v119, v118, v117, v116, v3, v5)
MOVE r1 -> r1
succs: [bb 0]

BB 0  preds: [bb 3]
prop: bc: 0x00000000
    3.     IfImm LE i32               v0(r1), 0x5                                                      bc: 0x00000004
succs: [bb 1, bb 2]

BB 2  preds: [bb 0]
prop: bc: 0x00000006
  114.i32  ReturnI                    0x5                                                              bc: 0x00000008
succs: [bb 4]

BB 1  preds: [bb 0]
prop: bc: 0x00000009
    6.i64  Constant                   0xa -> r0 (v5)
    5.i32  Mul                        v0(r1), v6(r0) -> r0 (v116)                                      bc: 0x0000000b
    8.i64  Constant                   0x14 -> r2 (v116)
  116.i32  MAdd                       v0(r1), v8(r2), v5(r0) -> r0 (v117)                              bc: 0x000000e3
...
// constants are defined just before usage, no SpillFills
...
   76.i64  Constant                   0x5d -> r2 (v150)
  150.i32  MAdd                       v0(r1), v76(r2), v149(r0) -> r0 (v112)                           bc: 0x000001af
  112.i32  Return                     v150(r0)                                                         bc: 0x000001b5
succs: [bb 4]

BB 4  preds: [bb 2, bb 1]
prop: end
```
