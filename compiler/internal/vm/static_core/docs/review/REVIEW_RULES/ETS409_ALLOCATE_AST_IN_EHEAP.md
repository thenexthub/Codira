# Allocate AstNode in EHeap only

## Severity

Error

## Applies To

/*.(cpp|h)$
language: cpp

## Detect

A violation is construction of any of the `AstNode`, `Expression`, `Statement`, `TypedAstNode`, `AnnotatedAstNode`, `TypedStatement`, `AnnotatedStatement`, `AnnotatedExpression`, `MaybeOptionalExpression` classes or any of their children using API other than:

- `New` method in `EAllocator` class
- `AllocNode` method in `Context` class

Constructing these classes or their children by invoking their constructor or operator `new` is strictly prohibited

## Example BAD code

```cpp
// BAD: allocated `Literal`, which is a child of `Expression` class using operator new
auto *s = new ir::Literal(/*...*/);
```

```cpp
// BAD: allocated `AstNode` by invoking constructor
auto n = ir::AstNode(/*...*/);
```

## Example GOOD code

```cpp
// GOOD: allocated `SpreadElement`, which is a child of `AnnotatedExpression` class using `AllocNode` API
auto *s = ctx->AllocNode<ir::SpreadElement>(/*...*/);
```

```cpp
// GOOD: allocated `NewExpression`, which is a child of `Expression` class using `New` API
auto *const c = allocator->New<NewExpression>(/*...*/);
```

## Fix suggestion

Please use `EAllocator` to allocate AST types. **All** allocations of such types should be uniform throughout the frontent codebase.

## Message

Type {typeName} allocated improperly. Please check the guidelines on allocation of such types.
