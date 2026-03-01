# Memory management via raw new / delete is prohibited

## Severity
Error

## Applies To
/*.(cpp|h)$
language: cpp

## Detect

A violation occurs when code uses raw **`new`** or **`delete`** operators for manual memory management.
This includes:
- Direct `new` expressions: `Type* ptr = new Type(...)`
- Direct `delete` expressions: `delete ptr` or `delete[] array`
- Placement new without Arena allocator context

**Exceptions** (not violations):
- Usage within Arena allocator implementation itself
- Placement new with explicit Arena allocator: `new (allocator->Alloc(...)) Type(...)`

## Example BAD code

```cpp
// BAD: Raw new causes memory leak
ir::Expression *expr = new ir::Identifier(...);
ArenaVector<ir::Expression *> expressions(...);
expressions.push_back(new ir::CallExpression(...));
```

## Example GOOD code

```cpp
// GOOD: Use AllocNode for AST nodes
ir::Expression *expr = AllocNode<ir::Identifier>(...);

// GOOD: Use std:: containers for local data
std::vector<ir::Expression *> expressions;
expressions.push_back(AllocNode<ir::CallExpression>(...));
```

## Fix suggestion

Replace raw memory management with appropriate alternatives:
- For **local data structures**: Use `std::vector`, `std::unordered_map`, or other STL containers
- For **AST nodes** and **persistent structures**: Use Arena allocator with `AllocNode<T>()` or `allocator->New<T>()`
- For **temporary compiler data**: Use `ArenaVector`, `ArenaMap` with appropriate allocator

## Message

Raw `{new|delete}` operator detected. Use Arena allocator (AllocNode<T>() or allocator->New<T>()) for persistent structures, or std:: containers for temporary data structures.
