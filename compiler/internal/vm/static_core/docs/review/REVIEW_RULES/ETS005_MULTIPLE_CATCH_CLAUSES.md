# Multiple catch clauses are prohibited

## Severity
Error

## Applies To
/*.(ets|sts)$
language: ts

## Detect

A violation occurs when a single `try { ... }` statement contains **multiple catch clauses** (for example `catch (e) { ... } catch (e) { ... }`). ArkTS code must use a single `catch` clause per `try` statement and branch on exception types inside that `catch` block instead of chaining catches.

## Example BAD code

```typescript
function foo() {
  try {
    // Initial work
  } catch (e: Error1) {
    // Handle Error1 error
  } catch (e: Error2) {
    // Handle Error2 error
  } catch (e) {
    // Handle base error
  } finally {
    // Finalize
  }
}
```

## Example GOOD code

```typescript
function foo() {
  try {
    // Initial work
  } catch (e) {
    if (e instanceof Error1) {
      // Handle Error1 error
    } else if (e instanceof Error2) {
      // Handle Error2 error
    } else {
      // Handle base error
    }
  } finally {
    // Finalize
  }
}
```

## Fix suggestion

Merge the logic into a single catch clause and use `instanceof` checks inside the `catch` block to handle different exception types.

## Message

Use a single `catch` block per `try` statement. Merge the `catch` clauses and branch on exception types within the block.
