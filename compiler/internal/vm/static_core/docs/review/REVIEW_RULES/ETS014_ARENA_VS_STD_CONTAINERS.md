# Proper container selection for memory management

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when **Arena-based containers** (`ArenaVector`, `ArenaMap`, `ArenaUnorderedMap`, etc.) are used for **temporary local data structures** that do not need to outlive the current function scope.

Arena allocators pool memory that is deallocated in bulk, which is efficient for persistent compiler structures but wasteful for short-lived local variables that should use stack or RAII-managed heap memory.

**Violation pattern:**
```cpp
void SomeFunction() {
    ArenaVector<Type*> tempList(allocator->Adapter());  // WRONG: temporary local data
    for (...) {
        tempList.push_back(...);
    }
    // tempList destroyed here, but Arena memory persists
}
```

## Example BAD code

```cpp
// BAD: Memory leak when container is destroyed
void ProcessNodes(ir::AstNode* node) {
    ArenaVector<ir::Expression*> expressions(allocator->Adapter());
    node->CollectExpressions(expressions);  // Memory leaked after function returns
    for (auto* expr : expressions) { /* ... */ }
}
```

## Example GOOD code

```cpp
// GOOD: STL container for local temporary data
void ProcessNodes(ir::AstNode* node) {
    std::vector<ir::Expression*> expressions;
    node->CollectExpressions(expressions);  // Properly cleaned up at scope exit
    for (auto* expr : expressions) { /* ... */ }
}
```

## Fix suggestion

- **Use `std::vector`, `std::map`, `std::unordered_map`** and other STL containers for temporary local data structures
- **Use Arena containers** (`ArenaVector`, `ArenaMap`, etc.) only when:
  - Data must persist beyond function scope
  - Data is stored in long-lived compiler structures (AST, Type system, Symbol tables)
  - Data lifetime matches the Arena's lifetime


## Message

Arena container '{ContainerType}' used for temporary local data. Use std::{StdEquivalent} instead to avoid memory waste. Arena containers should only be used for persistent compiler structures.
