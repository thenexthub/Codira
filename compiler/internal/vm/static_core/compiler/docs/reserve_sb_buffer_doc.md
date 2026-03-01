# Reserve String Builder internal buffer

## Overview

StringBuilder class implemented to store parts of a string being constructed in an internal buffer of Objects. Initially it is allocated to store INITIAL_BUF_SIZE objects. During StringBuilder object usage, the buffer needs to be reallocated several times to accommodate all the pieces fed.

## Rationality

Sometimes, it is possible to calculate nessesary space to store all the intermediate parts in advance. In this case, we initialize internal buffer with the size equal to the number of append instructions counted to prevent buffer reallocation.

## Dependence

* BoundsAnalysis
* Inlining
* LoopAnalysis
* DominatorsTree

## Algorithm

Example:
```TS
let sb = new StringBuilder()
sb.append(0)
for (let i = 1; i < 63; ++i)
    sb.append(i)
sb.append(63)
let output = sb.toString()
```
Since the compiler can count number of `StringBuilder::append()`-calls in between `constructor` and `toString()`-call (64 in this case), we can initialize StringBuilder internal buffer size to 64, and skip a couple of reallocations.

## Pseudocode

```C#
function ReserveStringBuilderBuffer(graph: Graph)
    foreach block in graph (in RPO)
        foreach instance being StringBuilder instance in block
            let count be number of append instructions of instance
            if count is valid
                replace initial StringBuilder.buf size constant to count
```

Validity of `count`:
 - Be greater than zero
 - Be less than or equal to maximum array size calculated to fit in TLAB
 - Be not equal INVALID_COUNT value, indicating that number of append instructions is uncountable


## Examples

ETS function example:
```TS
function nums(): String {
    let sb = new StringBuilder()
    sb.append("|")
    for (let i = 0; i < 62; ++i)
        sb.append(i)
    sb.append("|")
    let output = sb.toString()
}
```

IR before transformation:

(Save state and null check instructions are skipped for simplicity)
```
Method: std.core.String ETSGLOBAL::nums()

BB 4
prop: start
    2.i64  Constant                   0x3e
    3.i64  Constant                   0x0
   23.i64  Constant                   0x1
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead
   37.ref  LoadImmediate(class: std.core.StringBuilder)
    6.ref  NewObject 11622            v37, ss
    7.void CallStatic.Inlined 541254 std.core.StringBuilder::<ctor> ss
   46.i32  LoadStatic 541156 std.core.StringBuilder.INITIAL_BUF_SIZE v37
    9.ref  LoadImmediate(class: [Lstd/core/Object;)
   48.i32  NegativeCheck              v46, ss
   50.ref  NewArray 30087             v9, v48, ss
   ...
```
Notice, NewArray instruction second argument is StringBuilder.INITIAL_BUF_SIZE

IR after transformation:
```
Method: std.core.String ETSGLOBAL::nums()

BB 4
prop: start, bc
    2.i64  Constant                   0x3e
    3.i64  Constant                   0x0
   23.i64  Constant                   0x1
  118.i64  Constant                   0x40
  141.i64  Constant                   0x2
  142.i64  Constant                   0x4
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead
   37.ref  LoadImmediate(class: std.core.StringBuilder)
    6.ref  NewObject 11622            v37, ss
    7.void CallStatic.Inlined 541254 std.core.StringBuilder::<ctor> ss
    9.ref  LoadImmediate(class: [Lstd/core/Object;)
   50.ref  NewArray 30087             v9, v118, ss
```
Notice, NewArray instruction second argument is now Constant 0x40

## Links

* Implementation
    * [simplify_string_builder.h](../optimizer/optimizations/simplify_string_builder.h)
    * [simplify_string_builder.cpp](../optimizer/optimizations/simplify_string_builder.cpp)
* Tests
    * [ets_string_builder_reserve.ets](../../plugins/ets/tests/checked/ets_string_builder_reserve.ets)
