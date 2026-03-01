# Prefer type creation and TypeRelation over IsETSObjectType/IsETSUnionType patterns

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when code uses `IsETSObjectType()`, `IsETSArrayType()`, `IsETSUnionType()` in conditional logic that only handles specific corner cases rather than providing a **general solution** using type system operations.

**Problematic patterns:**
- `IsETSObjectType()` - Does not cover ETSArrayType, constrained type parameters
- `IsETSUnionType()` - Does not cover constrained TypeParameter, IntersectionType
- Conditional logic that branches on these checks for special cases
- Code that handles only some type combinations, not the general case

**Why this is wrong:**
- Only handles known corner cases, breaks with new types
- Does not provide systematic, composable solution
- Type system will evolve (new types may be added)
- Violates open/closed principle


**Ask yourself:**
- "What property am I really checking?" (nullability, reference type, etc.)
- "Can I express this with type operations?" (create union, check supertype, etc.)
- "Will this work if we add new type kinds?"

## Example BAD code

```cpp
// BAD: Only handles specific type combinations
if (arrayElementType->IsETSUnionType() || arrayElementType->IsETSObjectType()) {
    ArenaVector<checker::Type*> types(allocator->Adapter());
    types.push_back(arrayElementType->Clone(checker));
    types.push_back(checker->GlobalETSNullType());
    newArrayElementType = checker->CreateETSUnionType(std::move(types));
} else {
    newArrayElementType = arrayElementType;
}
// WRONG: What about TypeParameter with object constraint?
// WRONG: What about IntersectionType?
// WRONG: What about future type kinds?
```

## Example GOOD code

```cpp
// GOOD: General solution using type properties
// Create union with null for reference types, keep primitive types as-is
Type* newArrayElementType = arrayElementType->IsETSPrimitiveType()
    ? arrayElementType
    : checker->CreateETSUnionType(arrayElementType, checker->GlobalETSNullType());

// This works for:
// - ETSObjectType (reference type → union with null)
// - ETSArrayType (reference type → union with null)
// - ETSUnionType (reference type → union with null)
// - TypeParameter (handled by IsETSPrimitiveType check)
// - Future reference types (handled correctly)
// - Primitive types (kept as-is)
```

**Example: Creating types instead of checking kinds**

```cpp
// BAD: Branching on type kinds
if (type->IsETSUnionType()) {
    // Special union handling
} else if (type->IsETSObjectType()) {
    // Special object handling
} else if (type->IsETSArrayType()) {
    // Special array handling
}

// GOOD: Use type creation API
Type* nullableType = checker->CreateNullableType(type);  // General operation
Type* wideType = checker->GetWideTypeForType(type);      // General operation

// GOOD: Use TypeRelation to check properties
if (checker->Relation()->IsSupertypeOf(checker->GlobalETSObjectType(), type)) {
    // Handles all reference types uniformly
}
```

## Fix suggestion

**Provide general solutions using type creation and TypeRelation:**
- **Create types** that represent desired properties using `checker->CreateETSUnionType()`, etc.
- **Use TypeRelation** to check properties systematically
- **Avoid branching** on specific type kinds; instead work with type operations

## Message
`{IsTypeMethod}` pattern used in '{FunctionName}' handles only specific type cases. Provide general solution using type creation (CreateETSUnionType, etc.) and TypeRelation API that works for all current and future type kinds.
