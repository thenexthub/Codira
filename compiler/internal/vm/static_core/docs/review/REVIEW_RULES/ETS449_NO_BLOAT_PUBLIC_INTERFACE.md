# Avoid bloating public class interfaces

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when a **public or protected method** is added to a class (especially `ETSChecker`, `ETSAnalyzer`, or similar large classes) when the method could be implemented as a **static function** with limited scope.

**Violation indicators:**
- Method is used only once in a single translation unit
- Method does not access private class members
- Method could be a free function taking the class instance as a parameter
- Adding the method to already bloated classes (>1000 LOC, >50 methods)

**Example violation:**
```cpp
// In ETSChecker.h (already 1000+ LOC)
class ETSChecker {
public:
    bool ValidateTupleIndexFromEtsObject(/* params */);  // WRONG: used once in object.cpp
};

// In object.cpp
bool ETSChecker::ValidateTupleIndexFromEtsObject(/* params */) {
    // Implementation used only once in this file
}
```

## Example BAD code

```cpp
// BAD: Bloats ETSChecker interface
// ETSChecker.h
class ETSChecker {
public:
    bool ValidateTupleIndexFromEtsObject(ir::Expression* expr, checker::Type* type);
    Type* ResolveGenericTypeParameter(TypeParameter* param);  // Used once
    void ProcessUnionBranches(UnionType* union);  // Used once
};
```

## Example GOOD code

```cpp
// GOOD: Static helper functions in implementation file
// object.cpp
static bool ValidateTupleIndexFromEtsObject(ETSChecker* checker,
                                            ir::Expression* expr,
                                            checker::Type* type) {
    // Implementation here
}

// GOOD: Only add to class if genuinely reusable
class ETSChecker {
public:
    // Only methods used across multiple files or requiring private access
    bool IsTypeAssignable(Type* source, Type* target);
};
```

## Fix suggestion

- **Define as static function** in the implementation file (`.cpp`) where it's used
- Pass required class instances as parameters
- Only add to class interface if:
  - Used in multiple translation units
  - Needs access to private members
  - Part of a logical public API contract
  - Required for polymorphism/overriding

## Message

Method '{MethodName}' in class '{ClassName}' can be a static function. Classes like ETSChecker and ETSAnalyzer should minimize public interface bloat. Define as static helper function in implementation file unless used across multiple translation units or requires private member access.
