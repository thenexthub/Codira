# Try-catch blocks IR building

## Overview 

Consider the following pandasm code:

```
.function void RuntimeException.init(RuntimeException a0) <ctor, external>

.function i32 SimpleTryCatch.main() <static> {
        try_begin_label_0: call.short SimpleTryCatch.foo
        sta v0
        try_end_label_0: lda v0
        return
        handler_begin_label_0_0: movi v0, 0x0
        lda v0
        return
        handler_begin_label_0_1: movi v0, 0x1
        lda v0
        return

.catch RuntimeException, try_begin_label_0, try_end_label_0, handler_begin_label_0_0
.catch Exception, try_begin_label_0, try_end_label_0, handler_begin_label_0_1
}

.function i32 SimpleTryCatch.foo() <static> {
        initobj.short RuntimeException.init
        sta.obj v0
        throw v0
}
```
IR for this `main` method is:

```
            /-----------\
            | Start bb  |
            \-----------/
                |
                v
            /-----------\
            | Try-begin |---------->\------------------>\
            \-----------/           |                   |
                |                   |                   |
                v                   |                   |
        /---------------\           |                   |
        | return foo()  |           |                   |
        \---------------/           |                   |
                |                   |                   |
                v                   |                   |
        /-----------\               |                   |
        |  Try-end  |-------------->\------------------>\
        \-----------/               |                   |
                |                   |                   |
                |                   v                   v
                |               /-----------\       /-----------\
                |               |  return 0 |       |  return 1 |
                |               \-----------/       \-----------/
                |                   |                   |
                |                   v                   |
                |               /-----------\           |
                \-------------->|  End bb   |<----------/
                                \-----------/
```                                
`Try-begin` and `Try-end` - are try boundaries basic blocks. Both of them have one normal control-flow successor and N catch-handlers successors (N is equal 2 in the example: try-block has 2 catch-handlers in the method).

Edges form `Try-end` block to the catch-handlers are needed for correct linear order of basic blocks, since program flow can be jumped to the catch-handler after each throwable instruction, placed between try boundaries.

## IR with unreachable try-end

Consider the following pandasm code:

```
.record E {}

.function void foo() {
    return.void
}

.function u1 main() {
    movi v0, 0x0
    mov v2, v0
try_begin:
    movi v0, 0x2
    call.short foo
    mov.obj v3, v0
    inci v2, 0x1
    jmp try_begin
try_end:
    lda v2
    return

.catch E, try_begin, try_end, try_end
}
```
IR for this `main` method is:

```
                /-----------\
                | Start bb  |
                \-----------/
                      |
                      v
                /-----------\
          /---> | TryBegin  |----------\
          |     \-----------/          |
          |           |                |
          |           v                |
          |        /-----\             |
          |        | Try |             |
          |        \-----/             |
          |           |                |
          |           v                v
          |      /--------\        /-------\
          \----  | TryEnd |------> | Catch |
                 \--------/        \-------/
                                       |
                                       v
                                  /--------\
                                  | End bb |
                                  \--------/
```

## Exceptions' meta-info in the IR

`CatchPhi` - pseudo instruction which is added to the `Catch-begin` basic block. Each `CatchPhi` corresponds to the bytecode's virtual register and its inputs are values of these virtual registers in the points where throwable instructions are placed. `CatchPhi` contains vector of throwable instructions in the same order as related inputs.

In the next example `CatchPhi` has 3 inputs: `a0`, `a1`, `a2` and contains vector of 3 throwable instructions: `call foo`, `call foo1`, `call foo2`:

```
try_begin:
    movi v0, a0
    call foo
    movi v0, a1
    call foo1
    movi v0, a2
    call foo2
try_end:
    jmp label
...
```

`Try` - pseudo instruction which is added to the `Try-begin` basic block. It contains pointer to the `Try-end` and maps exceptions types to the catch-handlers basic blocks. 

In the example from overview section `Try` instruction provides information that `Try-begin`'s second successor is a handler for `RuntimeException` and the third successor is a handler for `Exception`.

## Graph special data-structures for exceptions info

- vector of try-begin blocks in order they are declared in the bytecode;
- map from each throwable instruction the corresponding catch-handlers;

## Try-catch blocks processing in the RegAlloc for bytecode-optimizer

Currently the folowing algorithm is implemented: 

- RegAlloc reserves unique dst-register for each catch-phi in the graph;
- Before each throwable instruction RegAlloc adds move from related catch-phi's input to the reserved catch-phi's dst-register;
