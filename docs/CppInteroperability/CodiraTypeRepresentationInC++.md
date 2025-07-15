# Codira Type Representation In C++

This document describes in details how Codira types are represented in C++.
It also covers related topics, like debug info representation for Codira types in
C++.

## Type Categories

### Value Types

1) Primitive Codira types like `Int`, `Float` , `OpaquePointer`, `UnsafePointer<int>?` are mapped to primitive C++ types. `int` , `float`, `void *`, `int * _Nullable`.

* Debug info: Does C++ debug info suffices?

2) Non-resilient fixed-layout Codira value type, e.g. `String` is mapped to a C++ class that stores the value in opaque buffer inline, e.g.:

```c++
class language::String {
  ...
  alignas(8) char buffer[24]; // Codira value is stored here.
}
```

* Debug info: ...


3) Resilient (or opaque layout) inline-allocated Codira value type small enough to fit into inline buffer. e.g `URL` is mapped to a C++ class that stores the value in opaque buffer inline, e.g.:

```c++
class Foundation::URL {
  ...
  uintptr_t pointer;         // pointer has alignment to compute buffer offset?
  alignas(N) char buffer[M]; // Codira value is stored here.
};
```

concrete examples:

```c++
// representation for buffer aligned at 8:
{/*pointer=*/0x3, ....}; // buffer is alignas(2^3 = 8)

// representation for buffer aligned at 16:
{/*pointer=*/0x4, ....}; // buffer is alignas(2^4 = 16)

// where pointer < 10 for inline stores.
```

* Debug info: ...


4) Resilient (or opaque layout) boxed Codira value , e.g. `SHA256` is mapped to a C++ class that stores the value boxed up on the heap, e.g.:

```c++
class CryptoKit::SHA256 {
  ...
  uintptr_t pointer;         // Codira value is stored on the heap pointed by this pointer.
  alignas(8) char buffer[8];
};
```

* Debug info: ...


5) Generic non-resilient fixed-layout Codira value type, e.g. `Array<Int>`, `String?`, is mapped to a C++ class that stores the value in opaque buffer inline, e.g.:

```c++
class language::Array<language::Int> {
  ...
  alignas(8) char buffer[8]; // Codira value is stored here.
}
```

* Debug info: ...


6) Generic opaque-layout / resilient / opaque-layout template type params Codira value type, e.g. `SHA256?`, is mapped to a C++ class that stores the value boxed up on the heap, e.g.:

```c++
class language::Optional<CryptoKit::SHA256> {
  ...
  uintptr_t pointer;        // Codira value is stored on the heap pointed by this pointer.
  alignas(N) char buffer[M];
}
```

* Debug info: ...

### Class Types

Class type is mapped to a C++ class that has a pointer to the underlying Codira instance in the base class:

```c++
class BaseClass {
private:
  void *_opaquePointer; // Codira class instance pointer is stored here.
}; 
class Vehicle: public BaseClass {
public:
}
```

* Debug info: ...

### Existential Types

Error type is mapped to a specific C++ `language::Error` class that stores the pointer to the error:

```c++
class Error {
private:
  void *_opaquePointer; // Codira error instance pointer is stored here.:
};
```

* Debug info: ...


Existential type. e.g. `any Hashable` maps to a C++ class that stores the opaque existential value (`language::any<language::Hashable>`):

```c++
class language::OpaqueExistential {
  // approximate layout.
  alignas(8) char buffer[8*5]; // opaque existential is stored here (inline or boxed by Codira)
};
class language::any<language::Hashable>: public language::OpaqueExistential {
};
```

* Debug info: ...
