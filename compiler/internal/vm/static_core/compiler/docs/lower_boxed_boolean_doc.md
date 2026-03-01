# Lower Boxed Boolean
## Overview
**Lower Boxed Boolean** is a scalar replacement optimization pass that detects usage of boxed boolean constants (`Boolean.TRUE`, `Boolean.FALSE`) and replaces them with equivalent primitive operations. This eliminates unnecessary boxing/unboxing and allows for further optimizations such as dead code elimination and block pruning.

## Rationality
This pass:
- Replaces `LoadStatic Boolean.TRUE/FALSE` with constant primitives (1/0).
- Eliminates intermediate Phi nodes for boxed booleans by creating primitive versions.
- Prunes redundant branches checking nullability (`Compare ... NullPtr`).
- Removes unused exception-handling paths.

## Dependencies
- RPO (Reverse Post-Order block traversal).
- Runtime metadata (to identify `Boolean.TRUE/FALSE` fields).

## Algorithm
### Traversal lgorithm
- Uses `VisitGraph()` to traverse all instructions.
- Tracks visited instructions to avoid reprocessing.
- Looks for `LoadObject`:
  - Checks if the load accesses `Boolean.value` field via runtime metadata.
  - Processes input instruction.
  - If input is reducible, replaces `LoadObject` users with constant.
- Looks for `Compare`:
  - Identifies comparisons against `NullPtr`.
  - Processes input instruction.
  - If comparing a reducible value, replaces comparison with constant `false`.

### Input instruction processing
- For `LoadStatic`
  - Gets boolean value via runtime (`1` for `TRUE`, `0` for `FALSE`).
  - Creates constant replacement.
- For `Phi`
  - Verifies all inputs are reducible.
  - Clones `Phi` with primitive type and inputs.
  - Preserves `NO_NULLPTR` flag

## Pseudocode
```
for block in RPO:
    for inst in block:
        if inst is LoadObject:
            if inst.input is LoadStatic(Boolean.TRUE/FALSE):
                replace LoadObject with constant 1/0
            elif inst.input is Phi:
                if all Phi inputs are reducible:
                    clone Phi with primitive constants

        if inst is Compare:
            if inst.input(0) is Phi and inst.input(1) is NullPtr:
              if result feeds into IfImm (throw):
                patch IfImm to always fall through
```

## Examples
Before pass:
```
BB 1:
  v1 = LoadStatic Boolean.TRUE
  ...
BB 2:
  v2 = LoadStatic Boolean.FALSE
  ...
BB 3:
  v3 = Phi(v1, v2)
  v4 = Compare v3, NullPtr
  IfImm v4, 0x0
BB 4:
  Throw
BB 5:
  v5 = LoadObject Boolean.value from v3
  IfImm v5, 0x0
```

After pass:
```
BB 1:
  c1 = Constant 1
BB 2:
  c0 = Constant 0
BB 3:
  v3 = Phi(c1, c0)
BB 5:
  IfImm v3, 0x0
```

## Links
Source code:
 - [lower_boxed_boolean.h](../optimizer/optimizations/lower_boxed_boolean.h)
 - [lower_boxed_boolean.cpp](../optimizer/optimizations/lower_boxed_boolean.cpp)
Test:
 - [lower_boxed_boolean.ets](../../plugins/ets/tests/checked/lower_boxed_boolean.ets)
