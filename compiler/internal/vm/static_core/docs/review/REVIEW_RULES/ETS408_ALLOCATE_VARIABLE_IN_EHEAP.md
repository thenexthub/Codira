# Allocate Variable in EHeap only

## Severity

Error

## Applies To

/*.(cpp|h)$
language: cpp

## Detect

A violation is construction of any of the `Variable` class or any of it's children using API other than:

- `New` method in `EAllocator` class

Constructing these classes or their children by invoking their constructor or operator `new` is strictly prohibited

## Example BAD code

```cpp
// BAD: allocated `LocalVariable`, which is a child of `Variable` using operator new
auto *l = new varbinder::LocalVariable(/*...*/);
```

```cpp
// BAD: allocated `Variable` by invoking constructor
auto v = varbinder::Variable(/*...*/);
```

## Example GOOD code

```cpp
// GOOD: allocated `GlobalVariable`, which is a child of `Variable` class using `New` API
auto *d = allocator->New<GlobalVariable>();
```

## Fix suggestion

Please use `EAllocator` to allocate varbinder types. **All** allocations of such types should be uniform throughout the frontent codebase.

## Message

Type {typeName} allocated improperly. Please check the guidelines on allocation of such types.
