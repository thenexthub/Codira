# No increase of the AST nodes size

## Severity

Error

## Applies To

/*.(cpp|h)$
language: cpp

## Detect

A violation is addition of instance fields to any of the `AstNode`, `Expression`, `Statement`, `TypedAstNode`, `AnnotatedAstNode`, `TypedStatement`, `AnnotatedStatement`, `AnnotatedExpression`, `MaybeOptionalExpression` classes or to any of their children.

## Example BAD code

```cpp
template <typename T>
class Typed : public T {
    // ...
    // BAD: new field aded to the Typed class
    uint32_t new_field{};
};
```

```cpp
class TypedAstNode : public Typed<AstNode> {
    // ...
    // BAD: new field aded to the child of the Typed class
    uint32_t new_field{};
};
```

```cpp
class ETSClassLiteral : public Expression {
    // ...
    // BAD: new field aded to the child of the Expression class
    uint32_t new_field{};
};
```

## Example GOOD code

N/A

## Fix suggestion

Please consider if your logic can be implemented without introducing new fields to the class. Increase in size for AST nodes significantly impacts overall frontend memory consumption. If adding new field is required, **double-check**:

- Field layout. Avoid wasting memory due to alignment.
- Type of the new field. **DO NOT** add objects that are managed by `std::allocator` as field (e.g. `std::string`, etc)

## Message

Field {fieldName} added to the AST structure. Consider if this is necessary here and if field layout is optimal.
