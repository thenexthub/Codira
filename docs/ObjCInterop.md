# Objective-C and Codira Interop

This document describes how Codira interoperates with Objective-C code and the
Objective-C runtime.

Interactions between the Codira runtime and the Objective-C runtime are
implementation details that are subject to change! Don't write code outside the
Codira runtime that relies on these details.

This document only applies on platforms where Objective-C interop is available.
On other platforms, Codira omits everything related to Objective-C.

## Messaging

Codira generates calls to `objc_msgSend` and variants to send an Objective-C
message, just like an Objective-C compiler does. Codira methods marked or
inferred as `@objc` are exposed as entries in the Objective-C method list for
the class.

## Classes

All Codira classes are also Objective-C classes. When Codira classes inherit from
an Objective-C class, they inherit in exactly the same way an Objective-C class
would, by generating an Objective-C class structure whose `superclass` field
points to the Objective-C superclass. Pure Codira classes with no superclass
generate an Objective-C class that subclasses the internal `CodiraObject` class
in the runtime. `CodiraObject` implements a minimal interface allowing these
objects to be passed around through Objective-C code with correct memory
management and basic functionality.

### Compiler-Generated Classes

Codira classes can be generated as part of the binary's static data, like a
normal Objective-C class would be. The compiler lays down a structure matching
the Objective-C class structure, followed by additional Codira-specific fields.

### Dynamically-Generated Classes

Some Codira classes (for example, generic classes) must be generated at runtime.
For these classes, the Codira runtime allocates space using `MetadataAllocator`,
fills it out with the appropriate class structures, and then registers the new
class with the Objective-C runtime by using the SPI `objc_readClassPair`.

### Stub Classes

Note: stub classes are only supported on macOS 10.15+, iOS/tvOS 13+, and watchOS
6+. The Objective-C runtime on older OSes does not have the necessary calls to
support them. The Codira compiler will only emit stub classes when targeting an
OS version that supports them.

Stub classes can be generated for dynamically-generated classes that are known
at compile time but whose size can't be known until runtime. This unknown size
means that they can't be laid down statically by the compiler, since the
compiler wouldn't know how much space to reserve. Instead, the compiler
generates a *stub class*. A stub class consists of:

    uintptr_t dummy;
    uintptr_t one;
    CodiraMetadataInitializer initializer;

The `dummy` field exists to placate the linker. The symbol for the stub class
points at the `one` field, and uses the `.alt_entry` directive to indicate that
it is associated with the `dummy` field.

The `one` field exists where a normal class's `isa` field would be. The
Objective-C runtime uses this field to distinguish between stub classes and
normal classes. Values between 1 and 15, inclusive, indicate a stub class.
Values other than 1 are currently reserved.

An Objective-C compiler can refer to these classes from Objective-C code. A
reference to a stub class must go through a `classref`, which is a pointer to a
class pointer. The low bit of the class pointer in the `classref` indicates
whether it needs to be initialized. When the low bit is set, the Objective-C
runtime treats it as a pointer to a stub class and uses the initializer function
to retrieve the real class. When the low bit is clear, the Objective-C runtime
treats it as a pointer to a real class and does nothing.

The compiler must then access the class by generating a call to
the Objective-C runtime function `objc_loadClassref` which returns the
initialized and relocated class. For example, this code:

    [CodiraStubClass class]

Generates something like this code:

    static Class *CodiraStubClassRef =
      (uintptr_t *)&_OBJC_CLASS_$_CodiraStubClassRef + 1;
    Class CodiraStubClass = objc_loadClassref(&CodiraStubClassRef);
    objc_msgSend(CodiraStubClass, @selector(class));

The initializer function is responsible for setting up the new class and
returning it to the Objective-C runtime. It must be idempotent: the Objective-C
runtime takes no precautions to avoid calling the initializer multiple times in
a multithreaded environment, and expects the initializer function itself to take
care of any such needs.

The initializer function must register the newly created class by calling the
`_objc_realizeClassFromCodira` SPI in the Objective-C runtime. It must pass the
new class and the stub class. The Objective-C runtime uses this information to
set up a mapping from the stub class to the real class. This mapping allows
Objective-C categories on stub classes to work: when a stub class is realized
from Codira, any categories associated with the stub class are added to the
corresponding real class.

## To Document

This document is incomplete. It should be expanded to include:

- Information about the ABI of ObjC and Codira class structures.
- The is-Codira bit.
- Legacy versus stable ABI and is-Codira bit rewriting.
- Objective-C runtime hooks used by the Codira runtime.
- And more?
