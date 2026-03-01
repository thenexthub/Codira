# No type pointer comparison for type checking

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when **pointer equality** (`==`, `!=`) is used to compare **Type objects** for type identity checking, such as:
- `type == checker->GlobalETSStringType()`
- `objType == targetType`
- `paramType != expectedType`

**Why this is wrong:**
- Pointer comparison only works for singleton types (globals)
- Fails for structurally identical but separately allocated types
- Does not handle type equivalence correctly (e.g., type aliases)
- Breaks when types are reconstructed or normalized
- Does not respect type system semantics

**Exception:** Pointer comparison is acceptable only when:
- Checking for null: `type == nullptr`
- Comparing within a single local scope for performance in hot paths (with documentation)

## Example BAD code

```cpp
// BAD: Pointer comparison for type identity
if (objType == checker->GlobalETSBuiltinStringType()) {
    // String-specific handling
}

// BAD: Direct pointer comparison between types
if (leftType == rightType) {
    // Assumes pointer equality means type equality
}

// BAD: Comparing function return type by pointer
if (signature->ReturnType() == checker->GlobalVoidType()) {
    // void return handling
}
```

## Example GOOD code

```cpp
// GOOD: Use TypeRelation for type identity
if (checker->Relation()->IsIdenticalTo(objType, checker->GlobalETSBuiltinStringType())) {
    // String-specific handling
}

// GOOD: Structural type comparison
if (checker->Relation()->IsIdenticalTo(leftType, rightType)) {
    // Types are identical
}

// GOOD: Null check is acceptable
if (type == nullptr) {
    // Handle null type
}
```

## Fix suggestion

Use **TypeRelation::IsIdenticalTo(t1, t2)** for type identity checks:
- Handles structural type equality correctly
- Works with type aliases and normalized types
- Respects type system semantics
- Future-proof against type system changes

For other type relationships, use appropriate TypeRelation methods:
- `IsSupertypeOf` for subtyping checks
- `IsAssignableTo` for assignment compatibility

## Message

Type pointer comparison detected: '{ComparisonExpression}'. Use TypeRelation::IsIdenticalTo(t1, t2) instead of pointer equality for type identity checks. Pointer comparison only works for singleton types and violates type system semantics.
