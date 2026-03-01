# Rationale for Bytecode

## Introduction

This document sets up some context about bytecode design principles and provides rationales for
bytecode design in Panda Runtime.

## Bytecode basics

Before discussing bytecode per se, let's take a look at an over-simplified picture of a real
hardware running a program.

There is a central processing unit (CPU) that reads commands (or _instructions_) from
somewhere in memory and executes corresponding _operations_ on operation's arguments,
also known as operation's _operands_. Operands may be _registers_ (very fast "variables" located
directly on the CPU) or _memory_ (some locations in computer's RAM). An important subset of memory
operands are _stack operands_ that reside in a special data structure called _stack_. The program
must maintain the stack in the correct state during runtime because exactly this data structure
is used for storing local variables along with function arguments and doing function calls.

In real world, different CPU manufacturers provide different sets of commands for their devices –
or, in other words, different CPUs have different _instruction set architectures_. This means
that the number and purpose of registers differs, too. Some nuances of working with stack may also
vary across CPUs and/or different operating systems.

Here comes the bytecode. Simply said, it is an attempt to build an abstract CPU on top of real
ones. A program written for such abstract CPU can be run on any real hardware with the help of a
special program called _interpreter_. The goal of the interpreter is to read our unified _virtual_
commands (or bytecode) and execute them. Of course, this implies additional performance overhead
making interpretation slower than _native code execution_. In return, we get the ability to
abstract from CPU limitations and run our program wherever our interpreter runs. Tooling
(debugger, profilers, etc.) is also unified, as well as the ecosystem for managing libraries,
frameworks, etc.

Although bytecode represents some abstraction, it mirrors all the mentioned concepts from the
hardware world: the terms "operations", "operands", "registers" and "stack" have the same meaning.
In case there is a chance for ambiguity, the terms "virtual registers" and "virtual stack" are used
to distinguish between an abstract system and the hardware.

Just as real CPUs can expose different instruction set architectures, there is no universal way of
building bytecode. Following sections explain advantages and disadvantages of various approaches.

## Encoding operands

One very important question is how an operation refers to its operands.

In _stack-based_ approach, operands are implicitly encoded in the operation, which results in
following code:

```
.function foo(arg1, arg2)
    push_arg1 ; copy the first argument to the top of the stack
    push_arg2 ; copy the second argument to the top of stack
    add       ; remove two top-most values from the stack, add them and put the result at the top
    ; at this point, the top of the stack contains arg1 + arg2
    ...
.end
```

In _register-based approach_, operands are explicitly encoded in the operation, which results in
following code:

```
.function foo(arg1, arg2)
    add vreg0, arg1, arg2 ; vreg0 = arg1 + arg21
    ; at this point, virtual register 0 contains arg1 + arg2
    ...
.end
```

This example demonstrates a fundamental difference between two approaches. Stack-based approach
operates with smaller instructions. Indeed, each instruction `push_arg1`, `push_arg1`, and `add`
can be represented with a single byte, while register-based `add reg_dst, reg_src1, reg_src2` may
require up to 4 bytes to encode.

At the same time, to execute a stack-based addition we need to run 3 instructions compared to
just a single register-based instruction. Since the interpreter has an extra work to do to read
each bytecode instruction, execute it and move to the next one, running more instruction results in
more _dispatch overhead_. Which means that the stack-based bytecode is slower by nature.

According to our experiment, uncompressed register-based bytecode can be reduced by ~26%
if substituted by a stack-based analogue. At the same time, performance becomes 10%-40% worse
(depending on the benchmark).

Since bytecode interpretation is a required program execution mode for Panda, performance of the
interpreter is very important, that's why
**Panda uses register-based instruction set architecture**.

However, to address the issue of compactness, two main tweaks are used:

* Implicitly addressed accumulator register.
* Variable size of instructions with frequent instructions are encoded to be smaller.

According to our research, these tweaks will allow to reduce the size of uncompressed bytecode by
~20% compared to pure register-based bytecode.

### Implicitly addressed accumulator register

Panda bytecode has a dedicated register called _accumulator_, which is addressed implicitly
by some bytecodes. With this tweak, our example can be rewritten as follows:

```
.function foo(arg1, arg2)
    adda arg1, arg2 ; acc = arg1 + arg21
    ; at this point, accumulator register contains arg1 + arg2
    ...
.end
```

With this approach, we are no longer required to encode destination register, it is "hardcoded" to
be an accumulator register. Having an implicitly addressed accumulator register de facto borrows
some "stack-based'ness" into an otherwise register-based instruction set in attempt to make the
encoding more compact.

In an ideal case, accumulator register may safe us ~25% of size. But it needs to be used carefully:

* Sometimes you might want to write directly into virtual register. e.g. for register moves (that
  are popular) and for increment/decrement instructions (when loop variable is only read in a loop
  body forming a separate def-use chain, i.e. in the majority of loops.
* You don't need to pass object reference in accumulator in the object call. Usually objects live
  longer than accumulator value (otherwise calls will be accompanied with moves from and to
  accumulator, reducing performance and increasing encoding size).
* The same goes with object and array loads and stores.

To address the risk of producing inefficient bytecode with redundant moves from and to
accumulator, a simple optimizer will be introduced as a part of the toolchain.

Finally, using accumulator allows getting rid of the instructions for writing the result to the register,
which also saves us encoding space and improves performance

### Variable size of instructions

Let's take a closer look at `adda arg1, arg2`. Assume that arguments map to virtual registers on
the virtual stack as follows:

```
+--------------+----------------------+
| accumulator  |                      |
| virt. reg. 0 | some local variable  |
| virt. reg. 1 | some local variable  |
| virt. reg. 2 | some temporary value |
| virt. reg. 3 | some temporary value |
| virt. reg. 4 | arg1                 |
| virt. reg. 5 | arg2                 |
+--------------+----------------------+
```

It easy to see that to address virtual registers 4 and 5 we need just 3 bits which allows to encode
the instruction as follows:

```
|<-       8 bits       ->|<- 4 bits ->|<- 4 bits ->|
|     operation code     |   vreg 1   |   vreg 2   |
```

This trick gives us just `1 + 0.5 + 0.5 = 2` bytes for a single instructions, which get us closer
to the stack-based approach. Of course, if virtual registers have large numbers that do no fit
into 4 bits, we have to use a wider encoding:

```
|<-       8 bits       ->|<-       8 bits       ->|<-       8 bits       ->|
|     operation code     |         vreg 1         |         vreg 2         |
```

How to make sure that we benefit from the shorter encoding most of the time? An observation shows
that most of operations inside a function happen on local and/or temporary variables, while
function arguments participate as operands in a fewer number of cases. With that in mind, let's map
function arguments to virtual registers with larger numbers reserving smaller ones for local
and/or variables.

Please note also that we don't need "full-range" versions for all instructions. In case some
instruction lacks a wide-range form, we can prepare operands for it with moves that have all
needed forms. Thus we save on opcode space without losing in encoding size (on average).

With such approach, we can carefully introduce various "overloads" for instruction when it could
be beneficial. For example, we have three types of instructions for integer-sized arithmetic
(acc-reg-reg, acc-reg, acc-imm) and integer-based jumps, but not for floating-point arithmetic
(which is rare) and which is supposed to have only acc-reg form. Another good candidates for
overloads are calls (different number of operands) and calls are the most popular instructions in
applications (thus we again save encoding space).

## Handling various data types

Another important question is how bytecode is supposed to handle various data types. Back to our
`adda ...` instruction, what are types of its operands?

One option is to make the operation _statically typed_, i.e. specify explicitly that it works only
with say 64-bit integers. In this case, if we want to add two double-precision floating point
numbers and store the result into accumulator, we will need a dedicated `adda_d ...`, etc.

Another option is to make the operation _dynamically typed_, i.e. specify that `adda ...` handles
all kinds of addition (for short and long integers, for signed and unsigned integers, for
single- and double-precision numbers, etc.).

The first approach bloats the instruction set, but keeps the semantics of each instruction simple
and compact. The second approach keeps the instruction set small, but bloats the semantics of
each instruction.

It may seem that the dynamically typed approach is better for dynamically typed languages, but it
is true only if the platform is **not** supposed to support multiple languages.
Consider a simple example: what is the result of the expression `4 + "2"` in JavaScript and, say,
Python? In JavaScript, it evaluates to the string `"42"`, while Python forbids adding a string to
a number without an explicit type cast. This means that if we would like to run these two languages
on the same platform with the same bytecode, we would have to handle both JavaScript-style addition
and Python-style addition within a single instruction, which would eventually lead us to an
unmaintainable bytecode.

Thus, as we are required to support multiple languages (both statically and dynamically typed),
**Panda uses statically-typed bytecode**.

There may be a concern: Does a statically typed bytecode forbid us to support a dynamically typed
language? No, it does not. In practice, it is always possible to compile a dynamically typed
language to some statically typed instruction set: after all, all native hardware instructions
sets are "statically-typed" in our terminology.

There may be another concern: Does a statically-typed bytecode imply statically-typed registers?
I.e. does it mean that if `adda reg1, reg2` operates only on 64-bit integers, registers `reg1`
and `reg2` **must** hold only integer values throughout the function? Fortunately, the answer is
no, they must not, virtual registers may hold value of different types (just as hardware registers,
which do not distinguish between integers and pointers on many platforms). The key constraint is
that once a value of a certain type is store into a virtual register, all operations on that value
must be of this very type, unless the virtual register is redefined. Language compilers and
bytecode verifiers take the responsibility to control this invariant.
