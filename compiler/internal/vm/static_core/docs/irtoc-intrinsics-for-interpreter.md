# Using IrToc implementations of the intrinsics in the interpreter

## Introduction

A method of a class or a global function can be declared as an intrinsic.
During the runtime initialization the initialization routine parses the
list of methods known as intrinsics and adds a reference to the c++
implementation to the method description. Each time while executing the
bytecode if the interpreter encounters a call to such a method it invokes
the c++ function associated with method's compiled code information.

Apart from the c++ implementations there are also (target) optimized IrToc
implementations for some (having considerable performance impact ) intrinsics.
Having the interpeter use these IrToc intrinsics implementations is
beneficial for the interpreter performance and might reduce the overall
amount of the c++ code.

## The issues

Originally IrToc functions were intended to by used by the compiled code
and were themselves compiled in so called `FastPath` mode. The interpreter
on the other hand uses native ABI with an imposed condition that the first
argument in the argument list is always a MethodPtr. So to invoke a regular
c++ code using this convention the arguments list has to be "shifted left".
This is done by means of the special bridge.
The current IrToc intrinsics implementations are using so called `FastPath`
calling convention that is incompatible with that of the interpreter, so,
we cannot use them directly and there is no bridge for such a mode transition.        

## The design of the solution

The special `FastPathPlus` mode allows to compile a single function into
two versions: `FastPath` to be used with the compiled code and `NativePlus`
to be used with the interpreter. The name of the `NativePlus` version
is suffixed with "NativePlus".

The function compiled in the `NativePlus` mode uses the same calling
convention as the interpreter. Since the first `MethodPtr` argument
has no meaning in the IrToc context it will be ignored. This approach,
however, allows us to call the IrToc implementation directly from the
I2C bridge. Strictly speaking it will be called via bridge selector
but the bridge selector will call the function itself not the bridge.

If a `NativePlus` IrToc implementation has to call another `NativePlus`
implementation it has to manually pass a fake first argument to the callee.
The regular c++ code can be called using the "natural" arguments list.

The bridge selectors generation is guided by the presence of "irtoc_impl"
field of the intrinsic definition. If the field is present the selector
will use the field's value directly otherwise the `impl` field will be
used and the CompiledAbi bridge will be generated for it. The IrToc
implementation is used only for the targets that are listed in the
`codegen_arch`.

## The limitations

If an IrToc intrinsic calls other functions the `FastPath` and the
`NativePlus` modes imply different approaches, so, callsite have to be
guarded by a condition checking the mode. This means that non-leaf
functions cannot generally use the same IrToc code intact via the
`FastPathPlus` macro-mode.

`SlowPath` intrinsic is not currently supported for the `NativePlus` mode
and if there such a call exists in the `FastPath` mode it has to be
changed to a regular `Call(...).Method()` or to a `TailCall` if appropriate.

While `TailCall` intrinsic is supported sometimes a part of the functionality
for a function is implemented directly in the codegen. In this case the
usage of the `TailCall` is not possible as the codegen functionality is
unavailable to the interpreter. So, the regular call is to be used instead
and the callsite has to be properly changed.

There is no builtin checks of which mode is used to create a function
the user is about to call, that is, whether it needs a dummy first
argument, so, the user has to be aware in what mode the function
that is to be called was compiled.

## The typical workflow

### Leaf functions

For a leaf function it is enough to just replace the `FastPath` mode
of the function with the `FastPathPlus` mode. This will make the irtoc
machinery to generate two versions of the function with different code
generators and using different calling conventions.

### function(:foo, ..., mode: [FastPathPlus]) => {foo(...), fooNativePlus(...)}


### Non-leaf functions

For non-leaf functions it is generally more complicated unless the user
needs to tail call a regular c++ function (or functions).

If an original function is a c++ function called via `SlowPath` the code
has to be changed to use either `Call` or `TailCall`. Generally speaking the
`TailCall` is preferred unless a different function is to be called or
some additional work is to be done on return from the function.

Calling a function compiled with the native ABI

```
if cgmode == :FastPath
  Intrinsic(:SLOW_PATH_ENTRY, arg1, arg2).AddImm(offset).MethodAsImm(name).Terminator.ptr
else # NativePlus
  Intrinsic(:TAIL_CALL, arg1, arg2).AddImm(offset).MethodAsImm(name).Terminator.ptr

```
calling a function compiled with the `NativePlus` ABI

```
if cgmode == :FastPath
  Intrinsic(:SLOW_PATH_ENTRY, arg1, arg2).AddImm(offset).MethodAsImm(name).Terminator.ptr
else # NativePlus
  Intrinsic(:TAIL_CALL, 0, arg1, arg2).AddImm(offset).MethodAsImm(name + 'NativePlus').Terminator.ptr

```

However, the cases when a function is supposed to call another function
compiled in the `NativePlus` mode are rare (`IndexOf` and `IndexOfAfter`
is an example), so, while keeping this in mind the user are not expected
to encounter such situations too often.
