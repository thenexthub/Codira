# Object Type Check Elimination
## Overview
**Object Type Check Elimination** - optimization which try to reduce number of IsInstance/CheckCast instructions.

## Rationality
Reduce number of instructions and remove unnecessary data-flow dependencies.

## Dependences
* RPO
* ObjectTypePropagation

## Algorithm
Visit `IsInstance` and `CheckCast` instructions in RPO order and try to elimiate them.

### IsInstance

`IsInstance` is replaced by 1 if an object in input can be cast to the resolved type, else replaced by 0. 'null' object is not an instance of any class.
`IsInstanceVisitor` also used in `Peephole` optimizations.

### CheckCast

If an object in input can't be cast to the resolved type `CheckCast` is replaced by deoptimize, else removed. 'null' object reference can be cast to every type.
`CheckCastVisitor` also used in `CheckCast` optimizations.

## Pseudocode
    TODO

## Examples

```
.record A {}
.record B <extends=A> {}
.record C {}

...
    newobj v0, B
    lda.obj v0
    isinstance A // will replaced by 1
    newobj v0, C
    lda.obj v0
    isinstance A // will replaced by 0
...
    newobj v0, B
    lda.obj v0
    checkcast A // will removed
    checkcast C // will replaced by deoptimze
```

## Links
Source code:
[object_type_check_elimination.h](../optimizer/optimizations/object_type_check_elimination.h)
[object_type_check_elimination.cpp](../optimizer/optimizations/object_type_check_elimination.cpp)

Tests:
[isinstance_elimination_test.cpp](../../tests/checked/isinstance_elimination_test.pa)
[checkcast_elimination_test.cpp](../../tests/checked/checkcast_elimination_test.pa)
