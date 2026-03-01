# PLT Resolvers

AOT compiler mode mainly described in [aot.md](../../docs/aot.md), please read it first.

## Brief SlowPath idea description

JIT/AOT compiler has a `SlowPath` mechanism. It is used for some opcodes where a call to runtime is required conditionally,
but not always.
During code generation so-called `SlowPath` code is created, and we put it into a special cold code block at the end of the function.
Unique `SlowPath` blob is generated for each place it is called, and as it contains saving registers and setting up of so-called
`BoundaryFrame` for stack walker, it's code is much longer than few runtime-call-related instructions mentioned in the section above.

## Code size issue

Speaking about AOT mode, for opcodes like `CallStatic`, `CallVirtual`, and opcodes related to `Class` resolving such
`SlowPath` also can be used, as we can cache gathered Method or Class pointer into a slot in GOT table (in `.aot_got` section).
The problem is that such a `SlowPath` would be actually required only once when we first time reach appropriate `method Id`
or `class Id`. So, in order to reduce code size in AOT mode, more tricky solution with PLT Resolvers is used.

## Static Call Resolver

For each pair of File (input for `ark_aot` compiler) and callee `method Id` (`panda_file::File::EntityId`) three
consecutive slots are reserved in PLT-GOT table. `FirstSlot` is filled during AOT file creation and contains `method Id`.
`SecondSlot` is filled during AOT file loading into runtime and contains `PLT CallStatic Resolver` address.
`ThirdSlot` would actually store `Method pointer` after resolving, but during AOT file loading it is initialized
to address of `SecondSlot`, subtracted by `GetCompiledEntryPointOffset` value.

During calls, first parameter is always a callee `Method pointer`, so the trick from previous paragraph allows to have 
fully transparent resolver for code generation. Lets see `arm64` example (`GetCompiledEntryPointOffset` is 56 = 7 * 8, all function
parameters are already in proper registers):

```
========= .aot_got ========
; Somewhere in PLT-GOT table
 . . .
-YY-16: FirstSlot - method Id
-YY-08: SecondSlot - PLT CallStatic Resolver
-YY-00: ThirdSlot - address of (-YY-08-56)  <--------------
 . . .                                                    |
; start of entrypoint table                               |
-NN: address of handler 0, NN = N * 8                     |
 . . .                                                    |
-16: address of handler N-1                               |
-08: address of handler N                                 |
========== .text ==========                               |
00:                                                       |
 . . .                                                    |
XX+00: adr x0, #-(YY+XX)   ; Put to the x0 address of ThirdSlot ; before resolve   ; after resolve
XX+04: ldr x0, [x0]        ; Load value stored in ThirdSlot     ; (&FirstSlot)-48  ; Method Pointer
XX+08: ldr x30, [x0, #56]  ; Load EntryPoint                    ; SecondSlot value ; Executable code
XX+12: blr x30             ; Call                               ; Call Resolver    ; Call Method
 . . .
```

`PLT CallStatic Resolver` after saving all registers to the stack and `BoundaryFrame` generation, have `(&FirstSlot)-48`
value in `x0`, so it may load `ldr x1, [x0, #48]` to get `method Id` from `FirstSlot`.
Caller `Method pointer` could be extracted (into `x0`) directly from Caller's CFrame, so,
having this two values in `x0` and `x1` it just call `GetCalleeMethod` to gather `Method pointer`.

When we have `Method pointer`, it is stored into `ThirdSlot`, allow to load proper executable address, and goes as first
parameter in actual method call. Jump by register value operation is used instead of call to return back directly into code,
not the resolver.

## Virtual Call Resolver

For each pair of File (input for `ark_aot` compiler) and callee `method Id` (`panda_file::File::EntityId`) two consecutive
slots are reserved in PLT-GOT table. `FirstSlot` is filled during AOT file creation and contains `method Id`.
`SecondSlot` is filled with zero and after resolving it stores `VTable index` incremented by 1.

```
========= .aot_got ========
; Somewhere in PLT-GOT table
 . . .
-YY-08: FirstSlot - method Id
-YY-00: SecondSlot, zero or (index+1) <---------------------------
 . . .                                                           |
; start of entrypoint table                                      |
-NN: address of handler 0, NN = N * 8                            |
 . . .                                                           |
-16: address of handler N-1                                      |
-08: address of handler N                                        |
========== .text ==========                                      |
00:                                                              |
 . . .                                                           |
; CallVirtual opcode (register allocator used x5 for Class ptr)  |
XX+00: adr x16, #-(YY+XX)        ; Put to the x16 address of SecondSlot
XX+04: ldr w17, [x16]            ; Load value from SecondSlot
XX+08: cbnz w17, #16             ; Jump to XX+24 when non-zero
XX+16: ldr x28, [#CALL_VIRTUAL_RESOLVER] ; Load VirtualCall Resolver address
XX+20: blr x30                   ; Call Resolver, x16 is like a "parameter" and "return value"
XX+24: ldr w16, [x5, #4]         ; Get Class pointer into x16
XX+28: add w16, w16, w17, lsl #3 ; x16 = Class+(index+1)*8
XX+32: ldr w16, [x16, #160]      ; Load Method from VTable (compensating index+1, as VTable start offset is 168)
 . . . ; Check IsAbstract
 . . . ; Save caller-saved registers
 . . . ; Set call parameters
ZZ+00: mov x0, x16               ; x0 = Method address
ZZ+04: ldr x30, [x0, #56]        ; Executable code address
ZZ+08: blr x30                   ; Call
 . . .
```

Unlike CallStatic, there is no way to use default parameter registers to send/receive values into resolver.
Thus for `PLT CallVirtual Resolver` convention is the following - first `Encoder` temporary register
(`x16` for `arm64` or `r12` for `x86_84`) is a parameter with `SecondSlot` address and also the same register
works as "return value"

`PLT CallVirtual Resolver` loads `method Id` from `FirstSlot` using address `x16-8`,
takes caller `Method pointer` from previous frame and calls `GetCalleeMethod` entrypoint.
Having `Method pointer` it is easy to load `VTable index` value.
Resolver returns `index+1` value using `x16`, and don't call any other functions like `PLT CallStatic Resolver` do.
Control is returned back into code instead.

## Class and InitClass Resolvers

For each pair of File (input for `ark_aot` compiler) and `class Id` (`panda_file::File::EntityId`) which needs to be resolved
three consecutive slots are reserved in PLT-GOT table. `FirstSlot` is filled during AOT file creation and contains `class Id`.
`SecondSlot` and `ThirdSlot` are filled with zeroes and after resolving they both store `Class pointer`, but have different meaning.
When `SecondSlot` in non-zero it means that `Class` is known to be in `Initialized` state already.

```
========= .aot_got ========
; Somewhere in PLT-GOT table
 . . .
-YY-16: FirstSlot - class Id
-YY-08: SecondSlot, zero or "Inialized Class" pointer <-----------
-YY-00: ThirdSlot, zero or Class pointer                         |
 . . .                                                           |
; start of entrypoint table                                      |
-NN: address of handler 0, NN = N * 8                            |
 . . .                                                           |
-16: address of handler N-1                                      |
-08: address of handler N                                        |
========== .text ==========                                      |
00:                                                              |
 . . .                                                           |
; Shared resolved slow path for PLT resolver                     |
YY+00: ldr x17, x28, [CLASS_INIT_RESOLVER] ; Load InitClass Resolver address
YY+04: br  x17                             ; Jump to resolver, x16 works like a "parameter" and "return value"
 . . .                                                           |
; LoadAndInitClass opcode (w7 register allocated for result)     |
XX+00: adr x16, #-(YY+8+XX)      ; Put to the x16 address of SecondSlot
XX+04: ldr w7, [x16]             ; Load value from SecondSlot
XX+08: cbnz w7, #20              ; Jump to XX+28 when non-zero
XX+12: bl YY - (XX+08)           ; Call shared slow path for PLT resolver, x16 works like a "parameter" and "return value"
XX+16: mov w7, w16               ; Class should be in w7
XX+20: ... ; run next opcode
 . . .
```

For class-related resolvers convention is the following - first `Encoder` temporary register
(`x16` for `arm64` or `r12` for `x86_84`) is a parameter with Slot address, and it is also used as "return value".

`PLT InitClass Resolver` loads `class Id` from `FirstSlot` using address `x16-8`,
takes caller `Method pointer` from previous frame and calls `InitializeClassById` entrypoint.
It stores gathered `Class pointer` into `ThirdSlot`, and also does the same for `SecondSlot` but under condition.
The condition is whether `Class` state is `Initialized`, as returning from `InitializeClassById` entrypoint in some corner
cases can happen when `Class` is yet only in `Initializing` state.

`PLT Class Resolver` receives `x16` addressing `ThirdSlot`, so it loads `class Id` from `FirstSlot` using address `x16-16`.
Another entrypoint is called here - `ResolveClass`. Gathered `Class pointer` value is stored into `ThirdSlot` only.

Both Resolvers returns `Class pointer` value using `x16` back into code.

## Resolver Encoding

As all 4 resolvers have a lot of similar parts, their generation in implemented in one method - `EncodePltHelper`.
Moreover, it is placed in platform-independent file `code_generator/target/target.cpp`, although there are actually several
differences in what's happening for `arm64` and `x86_64`.

Main difference between two supported platforms is a main temporary register to use in Resolver.
For `arm64` we use `LR` register (`x30`), and for `x86_64` third `Encoder` temporary - `r14` is used.

One more issue is that first `Encoder` temporary register (`x16` for `arm64` or `r12` for `x86_84`) used as parameter
in 3 Resolvers (all but CallStatic) is actually a caller-saved for `arm64`, but callee-saved for `x86`, leading to some
difference.

Lets briefly discuss all steps which happen consecutively in any Resolver:
* **Save LR and FP register to stack.**
On `arm64` is is just a one `stp x29, x30, [sp, #-16]` instruction,while on `x86` caller return address is already
on stack, so we load it into temporary (we need it for `BoundaryFrame`), and push `rbp` to the stack.

* **Create BoundaryFrame.**
It actually copies the `SlowPath` behavior of usual `BoundaryFrame` class constructor, but with one special trick:
for 3 out of 4 Resolvers (all but CallStatic) "return address" and "previous frame" values which are already on stack
(see previous step) directly became the upper part of `BoundaryFrame` stack part.

* **Save caller-saved registers.**
In CallStatic resolver we prepare place on the stack and save registers there. In three other Resolvers caller-saved
registers are saved directly into appropriate places in previous CFrame.
Stack pointer is temporarily manually adjusted in this case to allow `SaveCallerRegisters` function to do it's job.
Moreover, for `arm64` we manually add `x16` to live registers set.

* **Prepare parameters for Runtime Call.**
This step is described above separately in each resolver description.

* **Save callee-saved registers.**
Adjust stack pointer (second time for `CallStatic` Resolver, and the only time for other) and
call `SaveRegisters` two times - for float and scalar registers.  

* **Make a Runtime Call.**
This step is done using `MakeCallAot` function with properly calculated offset. Resolvers are placed after all functions in
AOT file, but distance to `.aot_got` section can be calculated in the same way like for usual code generation.

* **Load callee-saved registers.**
Reverse what was done two steps above - `LoadRegisters` for float and scalar registers, then adjust the stack pointer back.

* **Restore previous Frame.**
Works similar to `BoundaryFrame` class destructor.

* **Process gathered result.**
First, `arm64` non-`CallStatic` Resolvers need to manually restore `x16` from the place it was saved.
On `x86_64` this step is not required, as `r12` appears to be callee-saved register and is restored already.
Main logic of this step is described above separately in each resolver description.

* **Load caller-saved registers.**
Registers are loaded in the same manner they were saved. So, in CallStatic we have to adjust stack pointer after loading,
while in other Resolvers it is temporarily manually adjusted to previous frame before calling `LoadCallerRegisters` function.

* **Restore LR and FP.**
Nothing special, symmetric to the very first step.

* **Leave Resolver.**
Jump to the callee Method in `CallStatic` Resolver, and do a usual "return" in others.
