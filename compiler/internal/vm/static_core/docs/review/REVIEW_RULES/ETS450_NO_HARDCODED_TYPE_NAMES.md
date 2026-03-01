# No hardcoded type name strings for type checking

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when **hardcoded string literals** representing type names are used for **type checking** or **type comparison** operations, such as:
- Comparing `type->ToAssemblerName().str()` against string literals
- Using `strcmp`, `==`, `find`, or similar string operations on type names
- Pattern matching type names like `"escompat.Record"`, `"escompat.Array"`, `"std.core.Object"`

**Why this is wrong:**
- Type name strings are fragile and break with refactoring
- Does not handle type hierarchies, generics, or aliases correctly
- Bypasses the type system's proper comparison mechanisms
- Violates encapsulation of type identity

**Violation patterns:**
```cpp
// WRONG: String comparison for type checking
if (type->ToAssemblerName().str() == "escompat.Record") { /* ... */ }
if (objType->Name() == "escompat.Array") { /* ... */ }
```

## Example BAD code

```cpp
// BAD: Hardcoded type name comparison
if (type != nullptr && objType->IsETSObjectType() &&
    objType->ToAssemblerName().str() == compiler::Signatures::BUILTIN_RECORD) {
    // Special handling for Record type
}

// BAD: String matching for Array type
if (objType->ToAssemblerName().str() == "escompat.Array") {
    // Array-specific logic
}
```

## Example GOOD code

```cpp
// GOOD: Use TypeRelation to check type relationships
Type* recordType = checker->GlobalETSBuiltinRecordType();
if (checker->Relation()->IsIdenticalTo(objType, recordType)) {
    // Special handling for Record type
}

// GOOD: Check if type is assignable to Array
Type* arrayType = checker->GlobalBuiltinETSArrayType();
if (checker->Relation()->IsSupertypeOf(arrayType, objType)) {
    // Array-compatible logic
}
```

## Fix suggestion

Use the **TypeRelation API** for all type checking operations:
- `TypeRelation::IsSupertypeOf(t1, t2)` - Check if t1 is a supertype of t2
- `TypeRelation::IsIdenticalTo(t1, t2)` - Check if types are identical
- `TypeRelation::IsAssignableTo(source, target)` - Check assignability
- Use type system queries through `ETSChecker` methods

If the check cannot be expressed through TypeRelation, this indicates either:
1. A missing type system feature (request spec clarification)
2. Incorrect understanding of type semantics


## Message

Hardcoded type name comparison detected: '{TypeNameString}'. Use TypeRelation API (IsSupertypeOf, IsIdenticalTo, IsAssignableTo) instead of string comparison for type checking. If TypeRelation cannot express this check, request specification clarification.
