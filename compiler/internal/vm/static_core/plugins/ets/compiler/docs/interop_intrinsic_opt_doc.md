# Interop Intrinsic Optimization
## Overview 

Before the optimization, each call to Ecmascript method is enclosed by its own local scope (see [js_interop_inst_builder.cpp](../optimizer/ir_builder/js_interop/js_interop_inst_builder.cpp)). Arguments of the method are converted from ArkTS type (primitive or object, including JSValue) to local values (we call this **wrap**), and returned local value is converted to ArkTS JSValue, which may be cast to primitive or other object type later (we call conversion from local to primitve, JSValue or object of other class **unwrap**).

We try to reduce number of local scopes by merging them and then optimize wrap/unwrap intrinsics.

## Rationality

Elimination of scope creations and destructions reduces time needed for GC to remove local objects. Also the following optimizations (also made in this pass) are possible only for instructions inside the **same local scope**:

* Elimination of `unwrap+wrap` pairs (transform `local value->JSValue->local value` is redundant)
* Redundancy elimination for `wrap` intrinsics (hoisting them to more optimal positions and removing dominated duplicates)

## Dependence 

* Dominator tree
* LoopAnalysis
* Reverse Post Order (RPO)

## Algorithm

### Scope merging

The process preserves the following invariants:
* number of scope creations on any execution path is not increased *in most cases*
* nested scopes are not created
* not inlined calls to other ArkTS methods are not moved into local scopes (because this may potentially lead to many live local objects)


Scope merging is done in 3 stages, moving from more local to more global transforms. Number of objects in one scope is approximately limited by the value of the option `--compiler-interop-scope-object-limit`. If we hit the limit during the first 2 stages, we do not run the third one.

0. Exit if method has no interop calls; otherwise, if option `--compiler-interop-try-single-scope` is enabled and there are no calls to non-interop methods, enclose the method into one scope and remove other scope creations/destructions.

1. Try to merge local scopes inside each basic block. We iterate block instructions and track current scope status (whether we are inside scope, after scope end or there were no scopes yet) and number of local objects in last scope (to check the limit).

2. Consider a graph `G` where vertices correspond to block starts not contained in scopes, and vertices for bb1 and bb2 are connected by a (directed) edge if bb1 and bb2 are connected in CFG and there are no local scopes in bb1. Consider a (weakly) connected component `S` in this graph. `S` is separated from `V(G)\S` by scope switches (scope start or scope ends). If we remove these scope switches, blocks from `S` will be moved into some local scope, and scopes which were its neighbours will become the same scope. Note that the new scope may have several starts and several ends, but in each execution path exactly one scope start (end) will be used to enter (exit) this scope.

    To track number of objects in the newly created scopes, we use disjoint set union (DSU) with this number saved for each unique scope. We use a greedy approach and simply do not merge `S` with neighbouring scopes if estimated local object count will exceed `scope-object-limit`.

3. Having a scope which may start at several `CreateScope` intrinsics is bad for further optimization. Having several scopes which do not intersect or dominate each other, but use `wrap`s of the same values, is bad too. So we try to minimize number of scope creations while preserving our invariants.

    Here (and later) we use some ideas from general `partial-redundancy elimination` algorithms. We call a value **anticipated** (**partially anticipated**, respectively) in some block if it is computed on **each** (**any**) path to the end block. We call a value **available** in some block if it is computed on **each** path from the start block to it (the main case is when it is computed in some dominating block).

    NOTE: to be refined


### Find scopes for interop intrinsics

We traverse the CFG, tracking the current scope (i. e. last scope start instruction on the path from start block, remember that we don't create nested scopes) and remember scope for each interop intrinsic in `UnorderedMap`. After step `2` scope start for an instruction may be not unique (although we try to get rid of such cases on step `3`), so we recursively reset scope start to `nullptr` (meaning it is unknown) recursively when we see that different scope starts are possible for some block.

### Peepholes

Here we use scope starts computed above and optimize only pairs of instructions which are in the same scope.

* remove `unwrap+wrap` pairs:

```
v1.ptr Call ... -> v2 # local
v2.ref Intrinsic.CompilerConvertLocalToJSValue v1 -> v3
v3.ptr Intrinsic.CompilerConvertJSValueToLocal v1 -> v4
===>
v1.ptr Call ... -> v2, v4 # local
v2.ref Intrinsic.CompilerConvertLocalToJSValue v1
```

* remove `JSValue` creations when `JSValue` is always used after cast to the same ArkTS primitive:

```
v1.ptr Call ... -> v2 # local
v2.ref Intrinsic.CompilerConvertLocalToJSValue v1 -> v3
v3.f64 Intrinsic.JSRuntimeGetValueDouble v2 -> v5
v4.f64 Intrinsic.JSRuntimeGetValueDouble v2 -> v6
===>
v1.ptr Call ... -> v2 # local
v2.ref Intrinsic.CompilerConvertLocalToJSValue v1
v3.f64 Intrinsic.JSRuntimeConvertLocalToF64 v1 -> v5, v6
```

`JSValue`s may be used in not optimized bytecode, so we remove them only if they are not used in `SaveState`s with potential deoptimizations. (Restoring `JSValue`s by corresponding primitive values during deoptimization is not implemented).


### Redundancy elimination

For each tuple `(intrinsic id, intrinsic input, scope start)` (where `scope start` is unique) we try to hoist corresponding intrinsics (let `v` be the value of them) somehow and eliminate ones that can be replaced with others. We call `boundary` the instruction that should dominate our intrinsic, i. e. whether `scope start` or `intrinsic input` (one of the two will always dominate the other, boundary is the upper one). We use some heuristics for hoisting:
* hoist instruction to position of lower loop depth if it is partially anticipated there
* hoist instruction to some position if it is fully anticipated there and there is a path to end block where it occurs more than once

To reduce number of processed blocks, we traverse only the blocks where `v` is partially anticipated, and `boundary` is available.