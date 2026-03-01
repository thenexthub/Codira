# Allocate Type in EHeap only

## Severity

Error

## Applies To

/*.(cpp|h)$
language: cpp

## Detect

A violation is construction of any of the `Type`, `Signature`, `SignatureInfo` classes or any of their children using API other than:

- `New` method in `EAllocator` class

Constructing these classes or their children by invoking their constructor or operator `new` is strictly prohibited

## Example BAD code

```cpp
// BAD: allocated `Signature` using operator new
auto *s = new checker::Signature(/*...*/);
```

```cpp
// BAD: allocated `ByteType`, which is a child of `Type` by invoking constructor
auto b = checker::ByteType(/*...*/);
```

## Example GOOD code

```cpp
// GOOD: allocated `DoubleType`, which is a child of `Type` class using `New` API
auto *d = allocator->New<DoubleType>();
```

## Fix suggestion

Please use `EAllocator` to allocate checker types. **All** allocations of such types should be uniform throughout the frontent codebase.

## Message

Type {typeName} allocated improperly. Please check the guidelines on allocation of such types.
