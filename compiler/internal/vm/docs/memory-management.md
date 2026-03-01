# Memory management and object layout. Design.

## Overview

Panda Runtime should be scalable onto different devices/OSes. So we need some abstraction level for the OS memory management.
For now, all targets suppose interaction with the user, so we have some limitations for the STW pause metric.
We have very limited memory resources for IoT target, so we should maximize efforts on reducing memory overhead(fragmentation and object header size).

The main components of Panda memory management and object model:
* [Allocators](#allocators)
* [GC](#gc)
* [Object header](#object-header)

Panda runtime works/interacts with these memory types:
* internal memory for runtime(ArenaAllocators for JIT, etc)
* application memory(i.e., memory for objects created by application)
* native memory via JNI/FFI
* memory for JITed code

![High-level design](./images/panda-mm-overview.png "Memory management high-level design")

There are several modes for memory management:
- base mode
  - allocators with some average metrics and profile-based configuration(if available)
  - some baseline GC with profile-based configuration(if available)
- performance
  - allocators with low allocation cost
  - low-pause/pauseless GC(for games) or GC with high throughput and acceptable STW pause (for not games)
- power-saving mode
  - energy-efficient allocators(if possible)
  - special thresholds to improve power efficiency, 

Mode are chosen at the startup time (we'll use profile info from cloud for that).

## Object header

Rationale see [here](memory-management-overview.md).

### Requirements

* Support all required features from Runtime
* Similar design for two different platforms - high-end and low-end
* Compact Object Header for low-end target

### Specification / Implementation

**Common ObjectHeader methods:**

* Get/Set Mark or Class word
* Get size of the object header and an object itself
* Get/Generate an object hash

**Methods, specific for Class word:**

* Get different object fields
* Return object type
* Verify object
* Is it a subclass or not, is it an array or not, etc.
* Get field address

**Methods, specific for Mark word:**

* Object locked/unlocked
* Marked for GC or not
* Monitor functions(get monitor, notify, notify all, wait)
* Forwarded or not

Mark word depends on configuration and can have different sizes and layout. So here all possible configurations:

128 bits object header for high-end devices(64 bits pointers):
```
|--------------------------------------------------------------------------------------|--------------------|
|                                   Object Header (128 bits)                           |        State       |
|-----------------------------------------------------|--------------------------------|--------------------|
|                 Mark Word (64 bits)                 |      Class Word (64 bits)      |                    |
|-----------------------------------------------------|--------------------------------|--------------------|
|               nothing:61          | GC:1 | state:00 |     OOP to metadata object     |       Unlock       |
|-----------------------------------------------------|--------------------------------|--------------------|
|    tId:29    |      Lcount:32     | GC:1 | state:00 |     OOP to metadata object     |  Lightweight Lock  |
|-----------------------------------------------------|--------------------------------|--------------------|
|               Monitor:61          | GC:1 | state:01 |     OOP to metadata object     |  Heavyweight Lock  |
|-----------------------------------------------------|--------------------------------|--------------------|
|                Hash:61            | GC:1 | state:10 |     OOP to metadata object     |       Hashed       |
|-----------------------------------------------------|--------------------------------|--------------------|
|           Forwarding address:62          | state:11 |     OOP to metadata object     |         GC         |
|-----------------------------------------------------|--------------------------------|--------------------|
```
64 bits object header for high-end devices(32 bits pointers):
```
|--------------------------------------------------------------------------------------|--------------------|
|                                   Object Header (64 bits)                            |        State       |
|-----------------------------------------------------|--------------------------------|--------------------|
|                 Mark Word (32 bits)                 |      Class Word (32 bits)      |                    |
|-----------------------------------------------------|--------------------------------|--------------------|
|               nothing:29          | GC:1 | state:00 |     OOP to metadata object     |       Unlock       |
|-----------------------------------------------------|--------------------------------|--------------------|
|    tId:13    |      Lcount:16     | GC:1 | state:00 |     OOP to metadata object     |  Lightweight Lock  |
|-----------------------------------------------------|--------------------------------|--------------------|
|               Monitor:29          | GC:1 | state:01 |     OOP to metadata object     |  Heavyweight Lock  |
|-----------------------------------------------------|--------------------------------|--------------------|
|                Hash:29            | GC:1 | state:10 |     OOP to metadata object     |       Hashed       |
|-----------------------------------------------------|--------------------------------|--------------------|
|           Forwarding address:30          | state:11 |     OOP to metadata object     |         GC         |
|-----------------------------------------------------|--------------------------------|--------------------|
```

However, we can also support such version of the object header(Hash is stored just after the object in memory if object was relocated):
```
|--------------------------------------------------------------------------------------|--------------------|
|                                   Object Header (64 bits)                            |        State       |
|-----------------------------------------------------|--------------------------------|--------------------|
|                 Mark Word (32 bits)                 |      Class Word (32 bits)      |                    |
|-----------------------------------------------------|--------------------------------|--------------------|
|        nothing:28        | Hash:1 | GC:1 | state:00 |     OOP to metadata object     |       Unlock       |
|-----------------------------------------------------|--------------------------------|--------------------|
|  tId:13  |   LCount:15   | Hash:1 | GC:1 | state:00 |     OOP to metadata object     |  Lightweight Lock  |
|-----------------------------------------------------|--------------------------------|--------------------|
|        Monitor:28        | Hash:1 | GC:1 | state:01 |     OOP to metadata object     |  Heavyweight Lock  |
|-----------------------------------------------------|--------------------------------|--------------------|
|   Forwarding address:28  | Hash:1 | GC:1 | state:11 |     OOP to metadata object     |         GC         |
|-----------------------------------------------------|--------------------------------|--------------------|
```
This scenario decreases the size of a Monitor instance, and we don't need to save Hash somewhere during Lightweight Lock too.
Unfortunately, it requires extra memory after GC moved the object (where the original hash value will be stored) and also required extra GC work.
But, this scenario will be useful if we have allocator and GC which decreases such a situation to a minimum.

32 bits object header for low-end devices:
```
|--------------------------------------------------------------------------------------|--------------------|
|                                   Object Header (32 bits)                            |        State       |
|-----------------------------------------------------|--------------------------------|--------------------|
|                 Mark Word (16 bits)                 |      Class Word (16 bits)      |                    |
|-----------------------------------------------------|--------------------------------|--------------------|
|               nothing:13          | GC:1 | state:00 |     OOP to metadata object     |       Unlock       |
|-----------------------------------------------------|--------------------------------|--------------------|
|    thread Id:7    | Lock Count:6  | GC:1 | state:00 |     OOP to metadata object     |  Lightweight Lock  |
|-----------------------------------------------------|--------------------------------|--------------------|
|               Monitor:13          | GC:1 | state:01 |     OOP to metadata object     |  Heavyweight Lock  |
|-----------------------------------------------------|--------------------------------|--------------------|
|                Hash:13            | GC:1 | state:10 |     OOP to metadata object     |       Hashed       |
|-----------------------------------------------------|--------------------------------|--------------------|
|         Forwarding address:14            | state:11 |     OOP to metadata object     |         GC         |
|-----------------------------------------------------|--------------------------------|--------------------|
```

States description:

Unlock - the object not locked.

Lightweight Lock - object locked by one thread.

Heavyweight Lock - we have competition for this object(few threads try to lock this object).

Hashed - the object has been hashed, and hash has been stored inside MarkWord.

GC - the object has been moved by GC. 

## String and array representation

Array:
```
+------------------------------------------------+
|             Object Header (64 bits)            |
|------------------------------------------------|
|                Length (32 bits)                |
|------------------------------------------------|
|                  Array payload                 |
+------------------------------------------------+
```
String:

If we don't use strings compressing, each string has this structure:
```
+------------------------------------------------+
|             Object Header (64 bits)            |
|------------------------------------------------|
|                Length (32 bits)                |
|------------------------------------------------|
|           String hash value (32 bits)          |
|------------------------------------------------|
|                  String payload                |
+------------------------------------------------+
```
If we use strings compressing, each string has this structure:
```
+------------------------------------------------+
|             Object Header (64 bits)            |
|------------------------------------------------|
|                Length (31 bits)                |
|------------------------------------------------|
|             Compressed bit (1 bit)             |
|------------------------------------------------|
|           String hash value (32 bits)          |
|------------------------------------------------|
|                  String payload                |
+------------------------------------------------+
```
If the compressed bit is 1, the string has a compressed payload - 8 bits for each element.

If the compressed bit is 0, the string has not been compressed - its payload consists of 16 bits elements.

One of the ideas about string representation is to use a hash state inside Mark Word as a container for string hash value (of course we should save object hash somewhere else if it is needed or should use string hash value as the object hash value).

String:
```
+------------------------------------------------+
| String Hash | GC bit (1 bit) | Status (2 bits) |    <--- Mark Word (32 bits)
|------------------------------------------------|
|              Class Word (32 bits)              |
|------------------------------------------------|
|                Length (32 bits)                |
|------------------------------------------------|
|                  String payload                |
+------------------------------------------------+
```

See research [here](./memory-management-overview.md#possible-string-objects-size-reduction).
About JS strings and arrays see [here](./memory-management-overview.md#js-strings-and-arrays).

## Allocators

Requirements:
- simple and effective allocator for JIT
  - no need to manual cleanup memory
  - efficient all at once deallocation to improve performance
- reasonable fragmentation
- scalable 
- support for pool extension and reduction(i.e., we can add another memory chunk to the allocator, and it can give it back to the global "pool" when it is empty)
- cache awareness

*(optional) power efficiency

All allocators should have these methods:
- method which allocates ```X``` bytes
- method which allocates ```X``` bytes with specified alignment
- method which frees allocated pointed by pointer memory(ArenaAllocator is an exception)

### Arena Allocator

It is a region-based allocator, i.e., all allocated in region/arena objects can be efficiently deallocated all at once.
Deallocation for the specific object doesn't have effect in this allocator.

JIT flow looks like this:
```
IR -> Optimizations -> Code
```

After code generation, all internal structures of JIT should be deleted.
So, if we can hold JIT memory usage at some reasonable level - Arena Allocator ideally fits JIT requirements for allocator.

### Code Allocator

Requirements:
- should allocate executable memory for JITed code

This allocator can be tuned to provide more performance.
For example, if we have some callgraph info, we can use it and allocate code for connected methods with a minimized potential cache-collision rate.

### Main allocator

Requirements:
- acceptable fragmentation
- acceptable allocation cost
- possibility to iterate over the heap
- scalable
desired:
- flexible allocation size list(required to support profile-guided allocation to improve fragmentation and power efficiency)

#### Implementation details

Each allocator works over some pool

Size classes(numbers just informational - they will be tuned after performance analysis):
- small(1b-4Kb)
- large(4Kb - 4Mb)
- humongous(4Mb - Inf)

Size-segregated algorithm used for small size class to reduce fragmentation. 
Small objects are joined in "runs"(not individual element for each size, but some "container" with X elements of the same size in it).
```
+--------------------------------------+-----------------+-----------------+-----+-----------------+
| header for run of objects with size X| obj with size X | free mem size X | ... | obj with size X |
+--------------------------------------+-----------------+-----------------+-----+-----------------+
```

Large objects are not joined in "runs".

Humongous objects can be allocated just by proxying requests to the OS(but keep reference to it somewhere) or by using special allocator.

_Note: below for non-embedded target_

Each thread maintains a cache for objects(at least for all objects with small size).
This should reduce overhead because of synchronization tasks.

Locking policy:
- locks should protect localized/categorized resources(for example one lock for each size in small size class)
- avoid holding locks during memory related system calls(mmap etc.)

#### Profile-guided allocation

We can use profile information about allocation size for improving main allocator metrics.
If we see a very popular allocation size in profile, we can add it as an explicit segregated size and reduce fragmentation.
To make it work, allocator should support dynamic size table(or should have possibility choose from statically predefined).

### Energy efficiency in allocators

As shown in this [paper](https://www.cs.york.ac.uk/rts/docs/CODES-EMSOFT-CASES-2006/emsoft/p215.pdf) by changing 
various settings of the allocator, it is possible to get very energy efficient allocator.
There is no universal approach in this paper, but we can try to mix approach from this paper 
with our profile-guided approach.

## Pools and OS interactions

All used memory is divided in chunks. Main allocator can extend his pool with these chunks.

For the cases when we can get memory shortage we should have some preallocated buffer which allow Runtime to continue to work, while GC trying to free memory.

Note:
For the IoT systems without MMU Pools should have non-trivial implementation.

For some systems/languages will be implemented context-scoped allocator.
This allocator works over some arena and after the program will be out of the context - this arena will be returned to the OS.

## Spaces

- MemMapSpace, shared between these:
  - Code space (executable)
  - Compiler Internal Space(linked list of arenas)
  - Internal memory space for non-compiler part of runtime (including GC internals)
  - Object space
     - BumpPointerSpace
     - Regular object space
     - Humonguous objects space
     - TLAB space(optional)
     - RegionSpace(optional for some GCs)
     - Non-moving space
- MallocMemSpace
  - Humonguous objects space(optional)

Logical GC spaces:
- young space (optional for some GCs)
- survivor space (optional)
- tenured space

## GC

Garbage collector(GC) automatically recycles memory that it can prove will never be used again.

GC development will be iterative process.

Common requirements:
- precise GC (see [glossary](./glossary.md#memory-management-terms))
- GC should support various [modes](#overview)(performance, power-saving mode, normal mode);
- GC suitable for each mode he shouldn't violate requirements for this mode(see [here](#overview))

Requirements for Runtime:
- support for precise/exact roots
- GC barriers support by Interpreter and JIT
- safepoints support by Interpreter and JIT

Panda should support multiple GCs, since different GCs have different advantages(memory usage, throughput) at different benchmarks/applications.
So we should have possibility to use optimal GC for each application.

### Epsilon GC

Epsilon GC does absolutely nothing but makes the impression that Runtime has GC. I.e., it supports all required GC interfaces and can be integrated into Runtime.

Epsilon GC should be used only for debug and profiling purposes. I.e., we can disable GC and measure in mode "What if we don't have GC".

### STW GC

Stop-The-World GC.

Non-generational non-moving GC, during the work all mutator threads should be at safepoint.

1. Root scan
1. Mark
1. Sweep

### Concurrent Mark Sweep GC 

Requirements:
- concurrent
- generational
- low cpu usage (high throughput)
- acceptable STW pause
- (optional) compaction

We need to conduct more performance analysis experiments for choosing optimal scheme, but for now let's consider these options:
- generational moving (optionally compacting) GC
- (optional) generational non-moving (optionally compacting) GC

Spaces(for moving CMS):
```
+------------+------------+----------------------------+
| Eden/young |  Survivor  |        Tenured/old         |
|            | (optional) |                            |
+------------+------------+----------------------------+
```

Survivor space is optional and only for high-end targets.
Since one of the metric for this GC - high throughput, the most of the objects in the Eden will live enough to die.
If we prioritize energy-efficiency metric and the heap sizes at average not gigantic, it seems that we should avoid using survivor spaces.
So we can support it optionally for experiments. As alternative we can introduce some average age metadata for run of small objects.

Minor GC(STW):
1. Root scan for young gen, CardTable used for finding roots in old gen
1. Mark eden and move alive objects to the tenured(or survivor)
1. Sweep eden

Note: we'll use adaptive thresholds for triggering Minor GC for minimizing STW pause
Note #2: we can tune minor GC by trying make concurrent marking and re-mark, but it will require copy of the card table.

Major GC
1. Concurrent scan of static roots
1. Initial Mark - root scan(STW #1)
1. Concurrent Marking + Reference processor
1. Remark missed during concurrent marking objects (STW #2)
1. Concurrent Sweep + Finalizers
1. Reset

Reference processor - prevents issues with wrong finalization order.

Note: If we don't have Survivor spaces we can implement non-moving generational GC. 

### Region based GC (main)

Requirements:
- concurrent
- generational
- acceptable stable STW pause
- (optional) compaction

Since typical heap size for mobile applications is small - this GC can be considered as good choice for production.

All heap consists of memory regions with fixed size(it can vary, i.e. size of memory region #K+1 can be different than size of memory region #K).
```
+------------------+------------------+-----+------------------+
| Memory region #1 | Memory region #2 | ... | Memory region #N |
| young            | tenured          | ... | tenured          |
+------------------+------------------+-----+------------------+
```

Regions types:
- young regions
- tenured regions
- humonguous regions(for humonguous objects)
- empty regions

Incoming references for each region are tracked via remembered sets:
- old-to-young references
- old-to-old references

Minor GC(only for young regions - STW):
1. Root scan for young gen, remembered sets used for finding roots in old gen
1. Marking young gen + Reference processor + moving alive objects to the tenured space
1. Sweep + finalizers

The size of young space selected to satisfy 

Mixed GC - minor GC + some tenured regions added to the young gen regions after the concurrent marking.
Concurrent marking(triggered when we reach some threshold for tenured generation size):
1. Root scan (STW #1)
1. Concurrent marking + Reference processor
1. Re-mark - finishes marking and update liveness statistics (STW #2)
1. Cleanup - reclaims empty regions and determines if we need mixed collections to reclaim tenured space. Tenured regions selected by using different thresholds.

Note: RSets optionally can be refined with special threads

### Low-pause GC (deffered)

Requirements:
- stable low STW pause/pauseless
- (optional)incremental
- with compaction

No explicit minor GC.

Major GC
1. Concurrent scan of static roots
1. Initial Mark - root scan(STW #1)
1. Concurrent Marking + Reference processor
1. Concurrent Sweep + Finalizers + Concurrent Copy & Compact
1. Reset

Note: good choice for the applications with big heap or for applications when it is hard to provide stable low pause with Region based GC.

Note: compaction is target and mode dependent, so for low-memory devices we can consider [semi-space compaction](./glossary.md#memory-management-terms).
For straight-forward approach we can consider some support from OS to minimize overlapping of semi-space compaction phases between applications.

### GC: interaction with Interpreter, JIT and AOT

#### Safepoints

Prerequisites:
* one HW register reserved for the pointer to the ExecState(per-thread state), let's call it `RVState`
* ExecState structure has field with address of some page used for safepoints and we knew offset of this field `SPaddrOffset`

In general, safepoint will be just presented as some implicit or explicit load from the `[RVState, SPaddrOffset]`.
For example, it can be something like this: `LDR R12, [RVState, #SPaddrOffset]`

Note: In some architectures it is make sense to use store instead of load because it requires less registers.

Note: If it is no MMU available - it is allowed to use explicit condition for safepoint, i.e. something like this(pseudocode):
```
if (SafepointFlag == true) {
    call Runtime::SafepointHandler
}
```

When GC wants to stop the world, it forces it by stopping all threads at the safepoint.
It protects some predefined safepoint memory page, and it leads to segmentation faults in all execution threads when they do the load from this address.

Safepoints should be inserted at the beginning of the method and at the head of each loop.

For each safepoint, we should have a method that can provide GC with information about objects on the stack.
Interpreter already supports such info in the frames.
But for JIT/compiler, it looks like we need some generated(by JIT/compiler) method that can get all necessary data for the safepoint.
This method can actually be just some code without prologue and epilogue.
We'll jump to its beginning from signal handler, and in the end, we should jump back to the safepoint, so probably we should put it near the original code.

So the flow looks like this:

```
 ...
 | compiled/jitted code | ------>
 | safepoint #X in the code | ---seg fault---> 
 | signal handler | ---change return pc explicitly---> 
 | method that prepares data about objects on stack for the #X safepoint and waits until STW ends | ---jump via encoded relative branch to safepoint--->
 | safepoint #X in the code | ---normal execution--->
 | compiled/jitted code | ------>
 ...
```

**Opens**: 
* should we generate method for each safepoint, or all safepoints at once?

#### GC Barriers

GC barrier is a block on writing to(write barrier) or reading from(read barrier) certain memory by the application code. GC Barriers used to ensure heap consistency and optimize some of GC flows.  

##### GC Write Barriers

Heap inconsistency can happen when GC reclaim alive/reachable object.
I.e. these two conditions should happen to reclaim active/reachable:
1. We store reference to a white object into a black object
1. There are no paths from any gray object to that white object

Besides addressing of heap inconsistency problem, write barrier can be used for maintaining incoming references for young generation or region.
 
So we can solve these issues with GC WRB(write barrier). GC WRB can be _pre_(inserted before the store) and _post_(inserted after the store). This barriers used **only** when we store reference to the object to some field of an object.

_Pre_ barrier usually used to solve issue with lost alive object during concurrent marking. Pseudocode(example):
```c++
if (UNLIKELY(concurrent_marking)) {
    auto pre_val = obj.field;
    if (pre_val != nullptr) {
         store_in_buff_to_mark(pre_val); // call function which stores reference to object stored in the field to process it later
    }
}
obj.field = new_val; // STORE for which barrier generated
```    

_Post_ barrier can be used to solve issue with tracking references from tenured generation to the young generation(or inter-region references). In this case we always know external roots for the young generation space(or for region). Pseudocode(abstract example, not real one):
```c++
obj.field = new_val; // STORE for which barrier generated 
if ((AddressOf(obj.field) not in [YOUNG_GENERATION_ADDR_BEG, YOUNG_GENERATION_ADDR_END]) && 
    (AddressOf(new_val) in [YOUNG_GENERATION_ADDR_BEG, YOUNG_GENERATION_ADDR_END])) {
    update_card(AddressOf(obj.field)); // call function which marks some memory range as containing roots for young generation 
}
``` 
Note: Sometimes we don't check if object and stored reference in different generations. Because we get much less overhead this way.

##### GC Read Barriers

Read barriers used during concurrent compaction in some GCs.
For example we concurrently moving object from one place(`from-space`) to the another(`to-space`).
At some moment we can have two instance of the one object.
So we need one of these conditions should stand if we want to keep heap consistent:
1. All writes happen into `to-space` instance of the object, but reads can happen from both `from-space` and `to-space` instances
1. All writes and reads happen into/from `to-space`

#### GC Barriers integration with Interpreter and compiler


From Interpreter you could use runtime interface methods:
```c++
static void PreBarrier(void *obj_field_addr, void *pre_val_addr);
static void PostBarrier(void *obj_field_addr, void *val_addr);
```
Note: for performance, we can put into ExecState address of conditional flag for conditional barriers with trivial condition (`if (*x) ...`).

It is critical to make compiler to encode barriers very optimally. At least fast path should be encoded effectively.
There are several approaches for that:
 1. To describe barrier use some meta-language or IR which can be interpreted/encoded by all compilers compatible with runtime (it is currently not applicable for the runtime)
 1. (a lot of open questions here, so consider this as an idea) One compiler knows how to encode barrier using runtime interfaces (see next item) and could provide some more compiler-friendly interface to the other compilers to encode GC barriers.
 1. The compiler knows for each barrier type how it should be encoded (see pseudocode in `libpandabase/mem/gc_barrier.h`). And could use the runtime to get all required operands to do this.
Let's consider below encoding of PRE_ barrier:
   - get barrier type via RuntimeInterface: `BarrierType GetPreType() const`
   - for this barrier type get all needed operands provided by Runtime via
     `BarrierOperand GCBarrierSet::GetBarrierOperand(BarrierPosition barrier_position, std::string_view name);`
     (you should use operand/parameters names from pseudocode provided in `enum BarrierType`)
   - encode barrier code using loaded operands and pseudocode from `enum BarrierType`

## Memory sanitizers support

Panda Runtime should support [ASAN](https://github.com/google/sanitizers/wiki/AddressSanitizer).

Optional: [MSAN](https://github.com/google/sanitizers/wiki/MemorySanitizer)
(Note: not possible to use without custom built toolchain) 

Desirable, but not easy to support: [HWSAN](https://clang.llvm.org/docs/HardwareAssistedAddressSanitizerDesign.html)
