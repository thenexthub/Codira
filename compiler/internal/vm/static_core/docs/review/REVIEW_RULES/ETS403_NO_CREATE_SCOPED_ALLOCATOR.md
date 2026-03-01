# No increase of the Type size

## Severity

Error

## Applies To

/*.(cpp|h)$
language: cpp

## Detect

A violation is creation of scoped allocator using the following `EHeap` API:

- `CreateScopedAllocator()`
- `NewScopedAllocator()`

## Example BAD code

```cpp
// BAD: CreateScopedAllocator invoked in code
auto allocator1 = EHeap::CreateScopedAllocator();
// BAD: NewScopedAllocator invoked in code
auto allocator2 = EHeap::NewScopedAllocator().release();
```

## Example GOOD code

N/A

## Fix suggestion

Do not manually instantiate scoped allocators. All needed instances of scoped allocators are already provided by frontend.

## Message

Do not manually create scoped allocators.
