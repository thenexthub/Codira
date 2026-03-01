# Typed catch parameter is prohibited

## Severity
Error

## Applies To
/*.(ets|sts)$
language: ts

## Detect

A violation is any **catch clause** in a `try { ... } catch (...) { ... }` statement where the `catch` parameter
has an explicit **type annotation**. The `catch` parameter must be untyped in ArkTS code.

## Example BAD code

```typescript
function foo() {
  try {
    // Do work
  } catch (e: Error1) {
    // Handle Error1 error
  }

  try {
    // Do work
  } catch (e: Error2) {
    // Handle Error2 error
  } catch (e: Error3) {
    // Handle Error3 error
  }
}
```

## Example GOOD code

```typescript
function foo() {
  try {
    // Do work
  } catch (e) {
    if (e instanceof Error1) {
      // Handle Error1 error
    }
  }

  try {
    // Do work
  } catch (e) {
    if (e instanceof Error2) {
      // Handle Error2 error
    } else if (e instanceof Error3) {
      // Handle Error3 error
    }
  }
}
```

## Fix suggestion

Remove the type annotation from the catch parameter and perform type checks inside the body using `instanceof` according to the program logic.

## Message

Catch parameter '{Identifier}' must not declare a type. Remove the type annotation and use runtime checks inside the `catch` block.
