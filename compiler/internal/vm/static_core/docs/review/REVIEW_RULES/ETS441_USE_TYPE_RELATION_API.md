# Use TypeRelation API for type compatibility checks

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when type checking or type compatibility logic is implemented **without using the TypeRelation API**, particularly:
- Manual type compatibility checks using conditional logic
- Custom subtyping or assignability implementations
- Type checking that bypasses `TypeRelation::IsSupertypeOf`, `IsAssignableTo`, `IsIdenticalTo`
- Implementing type rules directly instead of using centralized type system

**Why TypeRelation is mandatory:**
- Centralizes type system semantics in one place
- Ensures consistency across all type checks
- Makes type rules auditable and maintainable
- Aligns with specification requirements
- Prevents divergent type checking logic

**TypeRelation API methods:**
- `IsSupertypeOf(t1, t2)` - Check if t1 is a supertype of t2
- `IsIdenticalTo(t1, t2)` - Check if types are identical
- `IsAssignableTo(source, target)` - Check assignment compatibility
- `IsCompatible(t1, t2)` - Check type compatibility
- `NeedWideningCast(source, target)` - Check if widening needed

## Example BAD code

```cpp
// BAD: Manual type compatibility check
static bool IsCompatibleTypeArgument(Type* typeArgument, Type* constraint,
                                     ETSChecker* checker) {
    if (typeArgument->IsETSVoidType()) {
        typeArgument = checker->GlobalETSUndefinedType();  // WRONG: manual conversion
    }

    // Custom compatibility logic bypassing TypeRelation
    if (typeArgument->IsETSPrimitiveType() && constraint->IsETSObjectType()) {
        return false;
    }

    return checker->Relation()->IsSupertypeOf(constraint, typeArgument);
}

// BAD: Implementing assignment rules manually
bool CanAssign(Type* source, Type* target) {
    if (source->IsETSVoidType() || target->IsETSVoidType()) return false;
    if (source->IsETSNullType() && target->IsETSObjectType()) return true;
    // ... more manual rules
}
```

## Example GOOD code

```cpp
// GOOD: Use TypeRelation directly
static bool IsCompatibleTypeArgument(Type* typeArgument, Type* constraint,
                                     ETSChecker* checker) {
    // TypeRelation handles all type compatibility rules correctly
    return checker->Relation()->IsSupertypeOf(constraint, typeArgument);
}

// GOOD: Delegate to TypeRelation for all type checks
bool CanAssign(Type* source, Type* target, ETSChecker* checker) {
    return checker->Relation()->IsAssignableTo(source, target);
}

// GOOD: Use TypeRelation for identity checks
if (checker->Relation()->IsIdenticalTo(returnType, expectedType)) {
    // Types match exactly
}
```

## Fix suggestion

**Replace custom type checking with TypeRelation API:**
1. Identify what type relationship you're checking (supertype, identical, assignable)
2. Use appropriate TypeRelation method
3. If TypeRelation cannot express your check, this indicates:
   - Missing specification for the feature
   - Misunderstanding of type semantics
   - Need for specification clarification

**Never implement type rules manually** - always use or extend TypeRelation.

## Message

Type compatibility check implemented without TypeRelation API in '{FunctionName}'. Use TypeRelation methods (IsSupertypeOf, IsAssignableTo, IsIdenticalTo) for all type checking. If TypeRelation cannot express this check, request specification clarification.
