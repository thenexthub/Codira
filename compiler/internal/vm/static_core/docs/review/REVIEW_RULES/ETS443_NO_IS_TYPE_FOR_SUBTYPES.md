# Use TypeRelation for types with subtypes

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when `Is<SomeType>()` methods are used on **types that have subtypes** without considering subtype relationships. Common problematic patterns:
- `type->IsETSStringType()` - does not handle string literal types
- `type->IsETSObjectType()` - does not handle ETSArrayType
- `type->IsETSUnionType()` - does not handle constrained TypeParameter, IntersectionType
- Any `IsXXXType()` check where XXX has possible subtypes or related types

**Why this is wrong:**
- If type T can be used in a context, **any subtype of T** can also be used
- `IsXXXType()` only checks exact type, not type hierarchy
- Misses type aliases, constrained generics, and intersection types
- Violates Liskov substitution principle
- Creates bugs when new related types are introduced

**Core principle:** Type checking must respect the **type hierarchy**, not just exact type matches.

## Example BAD code

```cpp
// BAD: Handles string but not string literals (subtypes)
checker::Type* ETSAnalyzer::Check(ir::ForOfStatement* st) const {
    checker::Type* exprType = st->Right()->Check(checker);

    if (exprType->IsETSStringType()) {  // WRONG: Misses string literal types
        elemType = checker->GlobalBuiltinETSStringType();
    }
}

// BAD: Misses array types (ETSArrayType is subtype relationship)
if (leftType->IsETSStringType() || rightType->IsETSStringType() ||
    leftType->IsETSBigIntType() || rightType->IsETSBigIntType()) {
    // WRONG: Only handles exact types, not subtypes or literals
}

// BAD: Doesn't handle union of objects or constrained type parameters
if (arrayElementType->IsETSUnionType() || arrayElementType->IsETSObjectType()) {
    // WRONG: Misses TypeParameter with object constraint, IntersectionType
}
```

## Example GOOD code

```cpp
// GOOD: Use TypeRelation to check string compatibility
checker::Type* ETSAnalyzer::Check(ir::ForOfStatement* st) const {
    checker::Type* exprType = st->Right()->Check(checker);
    Type* stringType = checker->GlobalBuiltinETSStringType();

    if (checker->Relation()->IsSupertypeOf(stringType, exprType)) {
        // Handles string and all string subtypes (literals, etc.)
        elemType = stringType;
    }
}

// GOOD: Use TypeRelation for type compatibility
Type* stringType = checker->GlobalETSStringType();
Type* bigintType = checker->GlobalETSBigIntType();

if (checker->Relation()->IsSupertypeOf(stringType, leftType) ||
    checker->Relation()->IsSupertypeOf(stringType, rightType) ||
    checker->Relation()->IsSupertypeOf(bigintType, leftType) ||
    checker->Relation()->IsSupertypeOf(bigintType, rightType)) {
    // Handles types and all their subtypes correctly
}

// GOOD: Primitive type checks are acceptable (no subtypes)
if (type->IsETSIntType()) {
    // OK: int has no subtypes in type hierarchy
}
```

## Fix suggestion

**Use TypeRelation API to check type relationships:**
- `Relation()->IsSupertypeOf(T, actualType)` - Check if actualType is T or subtype of T
- `Relation()->IsAssignableTo(actualType, T)` - Check if actualType can be used where T is expected
- `Relation()->IsIdenticalTo(T, actualType)` - Only when exact type match is genuinely required

**Only use `IsXXXType()` when:**
- Implementing low-level TypeRelation methods themselves
- Checking for primitive types (no subtypes)
- Explicitly checking for exact type in controlled contexts (with justification)

## Message

`{TypeCheckMethod}` used on type that may have subtypes in '{FunctionName}'. Use TypeRelation::IsSupertypeOf(expectedType, actualType) to properly handle type hierarchies, subtypes, and related types.
