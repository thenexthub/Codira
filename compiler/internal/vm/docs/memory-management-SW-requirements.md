# 1. INTRODUCTION

The main components of Panda memory management:
* Allocators
* GC

Allocators has two purposes:
1. Allocations for the application
1. Allocations for the internal usage by Runtime(Allocations for compiler purposes, for GC internal structures etc)

Garbage Collector:
Garbage collector(GC) automatically recycles memory that it can prove will never be used again.
GC used to recycle memory allocated as result of application work(objects, compiled code etc).

# 2. OVERALL DESCRIPTION

## Allocator

### Alocator Types
- Bump pointer allocator
- Arena Allocator (objects can be deallocated at once(list of arenas, almost at once - O(number of arenas in the list)))
- Freelist allocator
- TLAB
- Run of slots allocator
- Code allocator

### Spaces

- Code space (executable)
- Compiler Internal Space(linked list of arenas)
- Internal memory space for non-compiler part of runtime (including GC internals)
- Object space
- Non-moving space(space for non-movable objects)

## GC

- Concurrent generational GC (optional - we can disable generational mode)
- GC for classes (optional)
- GC for code cache(optional)

Reference processor.

High level requirements for GC:
- acceptable latency (max pause) for good user experience
- acceptable throughput
- acceptable footprint size

# 3. KEY FEATURES OF MEMORY MANAGEMENT SYSTEM

Configurable flexible setup "Set of allocators + GC"
We can use profile to choose MM configuration for application (for example: we can choose non-compacting GC with freelist allocators for some application if we get acceptable metrics for this)


# 4. INTERACTION WITH OTHER COMPONENTS

## Allocator

- allocator API for runtime
- TLAB API for compiler and interpreter
- interfaces for tools?

## GC

- Safepoints
- Reference storage and additional interfaces for MM <-> JNI interaction
- Barriers API for compiler and interpreter
- Possibility to change VRegs for any frame in the stack
- interfaces for tools?

# 5. ADDITIONAL REQUIREMENTS

- Memory management flexible enough to work with multiple languages.
- provide various statistics data

