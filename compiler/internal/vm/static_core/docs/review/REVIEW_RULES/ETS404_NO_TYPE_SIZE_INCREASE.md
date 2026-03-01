# No increase of the Type size

## Severity

Error

## Applies To

/*.(cpp|h)$
language: cpp

## Detect

A violation is addition of instance fields to any of the `Type`, `Signature`, `SignatureInfo` classes or to any of their children.

## Example BAD code

```cpp
class Type {
    // ...
    // BAD: new field aded to the Type class
    uint32_t new_field{};
};
```

```cpp
class LongType : public Type {
    // ...
    // BAD: new field aded to the child of the Type class
    uint32_t new_field{};
};
```

## Example GOOD code

N/A

## Fix suggestion

Please consider if your logic can be implemented without introducing new fields to the `checker::Type`. Increase in size for types significantly impacts overall frontend memory consumption. If adding new field is required, **double-check**:

- Field layout. Avoid wasting memory due to alignment.
- Type of the new field. **DO NOT** add objects that are managed by `std::allocator` as field (e.g. `std::string`, etc)

## Message

Field {fieldName} added to the `Type` or it's child. Consider if this is necessary here and if field layout is optimal.
