# No direct use of EAllocator

## Severity

Error

## Applies To

/*.(cpp|h)$
language: cpp

## Detect

A violation any use of `EAllocator` type identifier.

## Example BAD code

```cpp
// BAD: use of EAllocator
return EAllocator {};
```

## Example GOOD code

N/A

## Fix suggestion

New instances of `EAllocator` should not appear in code.

## Message

Do not use `EAllocator` type identifier.
