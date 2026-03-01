# String literal resolution in AOT-compiled code

To get string literal by its identifier application has to perform special runtime call. To improve performance in AOT
mode such runtime calls could be replaced with load from a special PLT-slot in AOT file. In that case compiled code 
loads a value from a slot associated with a string and checks if that value is valid pointer. If it's not then the
application have to perform `ResolveStringAot` runtime call that will return a pointer. That runtime call may also save
returned string pointer into the slot and subsequent executions of the same code will bypass runtime calls.

## Compiler support

During AOT compilation `LoadString` instruction could be either encoded as regular `ResolveString` runtime call or as
a PLT-slot check with `ResolveStringAot` runtime call as a fallback.
To use the latter encoding the following conditions have to be met:

- compiled method should not be a static class initializer (initializers get invoked only once and the first
  resolution attempt requires runtime call, so usage of regular `ResolveString` call reduces the overhead);
- `--compiler-aot-load-string-plt` option should be turned on (it is on by default).

If the condition is met then the compiler reserves a unique PLT-slot (with type `STRING_SLOT`) for the string and
emits load from that slot, followed by the check that the loaded value is a valid pointer and the slow path.

The slow path performs `ResolveStringAot` runtime call that returns resolved string.

## Runtime support

There are two options controlling when and how many string will be saved into PLT-slots:
- `--aot-string-gc-roots-limit` controls how many time the same string should be resolved before its pointer will be 
  saved into the associated PLT-slot;
- `--resolve-string-aot-threshold` controls how many strings could be save to PLT-slots (option applied to the whole
  application, not to a single AOT-file).

`RuntimeStringAot` runtime call resolves the string as a regular `RuntimeString` call, but also does the  following
things:
- checks the value already stored in `STRING_SLOT` PLT-slot - after AOT-file loading all such slots contain `0` and
  then each resolution attempt will increment value inside the slot until it either get replaced with a string pointer,
  or it reaches `PANDA_32BITS_HEAP_START_ADDRESS - 1` (after that invocations will not increment slot value);
- if PLT-slot's value is already a pointer then the resolved string gets returned;
- if amount of already filled PLT-slots is above `--resolve-string-aot-threshold` then the resolved string gets 
  returned;
- if PLT-slot's value is below `--aot-string-gc-roots-limit` then slot's value is incremented and the resolved string is
  returned;
- if PLT-slot's value is above `--aot-string-gc-roots-limit` then slot's value is replaced by string pointer, slot is
  registered as GC-root in `AotManager` and the resolved string is returned.

## GC support

String pointers stored inside PLT-slots have to be treated as GC-roots to prevent its scavenge in case there are no
references to it.
`AotManager` is responsible for storing PLT-slots containing references. It also provides methods to iterate over these
slots in order to scan and update them. As an optimization `AotManager` stores information about whether or not the 
PLT-slot is referencing to an object in young space. That information allow to significantly reduce amount of scanned 
PLT-slots during young GC, but it also requires update during update phase. As yet another optimization PLT-slots are 
updated only if `StringTable` was updated during GC, because both these structures are referencing to the same string (
PLT-slots referencing a subset of strings references by the `StringTable`).
