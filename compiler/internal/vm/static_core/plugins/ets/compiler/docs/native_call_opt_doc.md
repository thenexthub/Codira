# Native Call Optimization

## Overview

One can write native code in C/C++ languages in order to use some other native code (i.e. third-party native libraries). Also, the other reason may be getting good performance. The main goal of JIT/AOT compiler is to call these Native API methods as fast as possible. But calling conventions of Managed and Native methods are not the same. "Traditional" approach is to call Native methods like Managed methods through special assembly bridge, which does all the stuff of converting managed-to-native calling conventions. Unfortunately, this method is not good for calling Native methods as fast as possible. This optimization calls Native methods directly, in appropriate calling convention, without assembly bridges.

## Rationality

Reduce overhead of Managed-to-Native calling convention transformations in calling Native methods from JIT/AOT code. Currently, we have 3 Native calling modes:
1. @ani.unsafe.Direct: no objects allowed (in params (so, no virtual methods) / ret value), no coroutine state switch (managed-native and vice versa)
2. @ani.unsafe.Quick: objects are allowed, but there is no coroutine state switch
3. default (no annotation): objects are allowed, coroutine switches to native state (and back to managed after execution of method)

## Dependence

RPO analysis

## Algorithm

There is a set of IR instructions and Intrinsics to divide Native call into small several parts.

Intrinsics:
- CompilerGetNativeMethod - resolves addres of native method; deoptimizes, if native pointer is nullptr or if API version is unsupported
- CompilerGetNativeMethodManagedClass - loads class pointer from the resolved method, converts runtime method to managed method
- CompilerGetMethodNativePointer - loads native pointer from resolved method (this is an address which we should call)
- CompilerGetNativeApiEnv - loads native api environment from a coroutine
- CompilerBeginNativeMethod - calls special BeginNative entrypoint, prepares stack map for stack walker
- CompilerEndNativeMethodPrim / CompilerEndNativeMethodObj - calls special EndNative entrypoint
- CompilerCheckNativeException - checks native exception in coroutine; throws exception, if any

IR instructions:
- CallNative - calls native method directly, using resolved native pointer; handles return value (in case it needs to be wided)
- WrapObjectNative - this is a special "meta"-instruction, which accepts a ref-parameter and produces a pointer; this is a meta-insctruction, because it does not use a value of parameter, it uses its location instead (at the point of CallNative instruction).

For @ani.unsafe.Direct (1) methods we replace CallStatic of Native method with following IR:
```
CompilerGetNativeMethod
CompilerGetMethodNativePointer
CallNative
```

For @ani.unsafe.Quick and default (2) and (3) methods we replace CallStatic of Native method with following IR:
```
CompilerGetNativeMethod
CompilerGetNativeMethodManagedClass
CompilerGetMethodNativePointer
CompilerGetNativeApiEnv
CompilerBeginNativeMethod
WrapObjectNative (N times, depends on number of reference parameters)
CallNative
CompilerEndNativeMethodPrim / CompilerEndNativeMethodObj
CompilerCheckNativeException
```

## Examples

To be added

## Links

Compiler part (IR):
- [instructions.yaml](../../../../compiler/optimizer/ir/instructions.yaml)
- [inst.h](../../../../compiler/optimizer/ir/inst.h)
- [native_call_optimization.h](../../../../compiler/optimizer/optimizations/native_call_optimization.h)
- [native_call_optimization.cpp](../../../../compiler/optimizer/optimizations/native_call_optimization.cpp)

Codegen part (asm):
- [ets_codegen_extensions.h](../optimizer/ets_codegen_extensions.h)
- [ets_codegen_extensions.cpp](../optimizer/ets_codegen_extensions.cpp)

Runtime part (intrinsics/entrypoints):
- [compiler.cpp](../../../../runtime/compiler.cpp)
- [runtime.yaml](../../../../runtime/runtime.yaml)
- [ets_entrypoints.yaml](../../runtime/ets_entrypoints.yaml)
- [ets_entrypoints.cpp](../../runtime/ets_entrypoints.cpp)
