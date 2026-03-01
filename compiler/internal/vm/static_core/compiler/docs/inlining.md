
# Inlining optimization

## Overview

Inlining optimization replaces a method call site with the body of the called method.

Inlining optimizations supports two types of call instructions.

- CallStatic - call static methods
- CallVirtual - call virtual methods

Inlining of these instructions has two main difference: method resolving and guards.

Resolving of the CallStatic method is quite simple, since target method is explicitly determined by the call instruction
itself. Thus, no guards is needed as well.

CallVirtual instruction also contains method ID, that should be called, but it also contains input object(receiver) that
is instance of some class, that, in turn, determines which implementation of the method should be called.

To devirtualize virtual calls, i.e. determine target method in compile time, we use following techniques:
- Static devirtualization
- Devirtualization with Class Hierarchy Analysis
- Inline caches

### Static devirtualization

Receiver is determined via ObjectTypePropagation analysis, that propagates statically known types down to the users.
For example:
```
    newobj v0, A
    call.virt A.foo, v0
```
here, we know receiver class (A) at compile time and we can inline A.foo method without any speculation.

### Devirtualization with Class Hierarchy Analysis

We use dynamic Class Hierarchy Analysis, because Panda Runtime allows dynamic class loading.

Class Hierarchy Analysis(CHA) is a runtime analysis that is invoked every time when a new class is loaded. The result of
the analysis is a flag in Method class, that indicates that method has a single implementation.

```
.record Test2 {}
.record A {}
.record B <extends=A> {}

.function i32 A.func(A a0) {
    ldai 1
    return
}
.function i32 B.func(B a0) {
    ldai 0
    return
}

# Actually, here is static devirtualization makes a work, we use this assembly for simplicity and clarity.
.function i32 Test2.main() {
    newobj v0, A  # CHA set SingleImplementation for A.foo method
    call.virt A.foo, v0
    newobj v0, B  # CHA set SingleImplementation for B.foo method and reset it for A.foo
    call.virt A.foo, v0
    return
}
```

Since Panda VM allows instantiating a classes with abstract methods, we can't rely on the fact that abstract classes are
never instantiated. Thus we can't apply the rule: abstract method does not break Single Implementation flag of the
parent method.

We should protect all devirtualized call sites with CHA by special guards. In short, these guards check that
devirtualized method still has Single Implementation flag.

CHA has dependency map, where key is a devirtualized method and value is a list of methods that employ this
devirtualized method, f.e. inline. Once method lost Single Implementation flag, CHA searches this method in the
dependency map and reset compilation code in the dependent methods.

But dependent methods (methods that inline devirtualized method) may already being executed and resetting the
compilation code in Method class is not enough. Thus, we walk over call stack of the all threads and search frames with
dependent methods. Then we set special `ShouldDeoptimize` flag in the frame of these dependent methods. This flag is
exactly that CHA guards check. When method execution reaches this guard, it reads flag and see that method is no longer
valid and deoptimize itself.

CHA guard comprises two instructions:
- `IsMustDeoptimize` - checks `ShouldDeoptimize` flag in the frame, return true if it is set.
- `DeoptimizeIf` - deoptimize method if input condition is true.

Disassembly of the CHA guard:
```
# [inst]    60.b    IsMustDeoptimize           r7 (v61)
  00a0: ldrb w7, [sp, #592]
  00a4: and w7, w7, #0x1 
# [inst]    61.     DeoptimizeIf               v60(r7), v14
  00c0: cbnz w7, #+0x3f4 (addr 0x400316e8fc)
```


### Inline caches

Not implemented yet.

## Inlining algorithm

After target method is determined, Inlining optimization call IrBuilderInliningAnalysis that check each bytecode
instruction of the inlining method is suiteble for inline.

If method is suitable, the Inliner creates new graph and runs IrBuilder for it. After that it embeds inlined graph into
the current graph.

_Pseudo code for the inlined graph embedding:_
```python
# Check bytecode is suitable for inlining
if not inlined_method.bytecode.include_only_suitable_insts:
    return false

# Build graph for inlined method. Pass current SaveState instruction, created for call_inst.
# This SaveState will be set as aditional input for all SaveState instructions in the inlined graph.
inlined_graph = IrBuilder.run(inlined_method, current_save_state)

# Split block by call instruction
[first_bb, second_bb] = curr_block.split_on(call_inst)

# Replace inlined graph incoming dataflow edges
if call_inst.has_inputs:
    for input in call_inst.inputs:
        input.replace_succ(call_inst, inlined_graph.start_block.param_for(input))

# Replace inlined graph outcoming dataflow edges
call_inst.replace_succs(inlined_graph.return_inst.input)
inlined_graph.remove(inlined_graph.return_inst)

# Move constants of inlined graph to the current graph
for constant in inlined_graph.constants:
    exisitng_constant = current_graph.get_constant(constant.value)
    if not exisitng_constant:
        current_graph.append_constant(constant)
    else:
        for succ : constant.succs:
            succ.replace_pred(constant, exisitng_constant)

# Connect inlined graph as successor of the first part of split block
inlined_graph.start_block.succ.set_pred(first_bb)

# Move all predecessors of inlined end block to second part of split block
for pred in inlined_graph.end_block:
    pred.replace_succ(inlined_graph.end_block, second_bb)

```

## Dependencies

- IrBuilder
- ObjectTypePropagation

## Links

[Inlining optimization source code](../optimizer/optimizations/inlining.cpp)

[Class Hierarchy Analysis source code](../../runtime/cha.cpp)
