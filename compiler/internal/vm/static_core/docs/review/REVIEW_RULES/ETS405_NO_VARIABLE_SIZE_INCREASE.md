# No increase of the AST nodes size

## Severity

Error

## Applies To

/*.(cpp|h)$
language: cpp

## Detect

A violation is addition of instance fields to the `Variabe` class or to any of it's children.

## Example BAD code

```cpp
class Variable {
    // ...
    // BAD: new field aded to the Variable class
    uint32_t new_field{};
};
```

```cpp
class LocalVariable : public Variable {
    // ...
    // BAD: new field aded to the child of the Variable class
    uint32_t new_field{};
};
```

## Example GOOD code

N/A

## Fix suggestion

Please consider if your logic can be implemented without introducing new fields to the class. Increase in size for variables significantly impacts overall frontend memory consumption. If adding new field is required, **double-check**:

- Field layout. Avoid wasting memory due to alignment.
- Type of the new field. **DO NOT** add objects that are managed by `std::allocator` as field (e.g. `std::string`, etc)

## Message

Field {fieldName} added to the `Variable` class or it's child. Consider if this is necessary here and if field layout is optimal.
