# Constant pool for interop strings


## Rationality

We have a `jscall` class for each ArkTS source file. Without this optimization, call to dynamic method (`dynamicObject.propA.foo(args..)`) is done as follows:

```
CallDynamic "propA.foo", dynamicObject, args...
// Execution:
// split "propA.foo" by "." in runtime
// create napi_strings for "propA" and "foo"
// prop1 = napi_get_property(dynamicObject, propA)
// ...
// methodRef = napi_get_property(propN, foo)
// call dynamic method: methodRef(propN, args...)
```

There's a performance problem that we split string in runtime and create `napi_string`s for each property (and method) name before each call.

## Solution

### ArkTS frontend part
 - enumerate all qualified names in the current file (see `ETSChecker::ResolveDynamicCallExpression` in [dynamic.cpp](../../../../../../ets_frontend/ets2panda/checker/ets/dynamic.cpp): `DynamicCallNames(isConstruct)->try_emplace(callNames.name, 0)`)
 - consider these qualified names as arrays of strings
 - concatenate the arrays into a single array and write it to the annotation of `jscall` class corresponding to the file (see `ETSEmitter::GenAnnotationDynamicCall` in [ETSemitter.cpp](../../../../../../ets_frontend/ets2panda/compiler/core/ETSemitter.cpp))
 - encode `CallDynamic("a.b.c", ...)` as `CallDynamic(qualifiedNameStart, qualifiedNameEnd, ...)` (see `LoadDynamicName` in [ETSCompiler.cpp](../../../../../../ets_frontend/ets2panda/compiler/core/ETSCompiler.cpp))

### Class init part
Now we can have several `jscall` classes, each of those has a separate indexing of strings in dynamic calls. We need to unite this indexings into a single one. To do so, when we *use for the first time* `jscall` class with `N` strings in its annotation:
 - current `qnameBufferSize_` (VM-global integer) is stored to a static field of this `jscall` class, and positions `[qnameBufferSize_, qnameBufferSize_ + N)` are assigned to this class (see `InitCallJSClass` in [call_js.cpp](../../runtime/interop_js/call/call_js.cpp))
 - `qnameBufferSize_` is atomically increased by `N` (see `AllocateSlotsInStringBuffer` in [interop_context.h](../../runtime/interop_js/interop_context.h))

And when we *use the jscall class for the first time in this napi_context*:
 - we have a **napi_array** of `napi_string`s for each napi_context. When `jscall` class is used, slots corresponding to it are populated with the `napi_string`s created from strings in `jscall` class annotation (see `ConstStringStorage::LoadDynamicCallClass` in [interop_context.cpp](../../runtime/interop_js/interop_context.cpp))

### Call part
To make a call `CallDynamic(qualifiedNameStart, qualifiedNameEnd, ...)` we:
 - load `classQualifiedNameStart` from the corresponding `jscall` class (see `GetClassQnameOffset` in [call_js.cpp](../../runtime/interop_js/call/call_js.cpp))
 - to retrieve `K`-th string from the qualified name, we load `(classQualifiedNameStart + qualifiedNameStart + K)`-th string from constant pool (which is a global `napi_array`) (see `JSRuntimeCallJSBase` in [call_js.cpp](../../runtime/interop_js/call/call_js.cpp))
