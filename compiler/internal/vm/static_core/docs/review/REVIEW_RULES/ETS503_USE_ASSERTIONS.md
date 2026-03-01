# Use assertions for runnable tests

## Severity
Error

## Applies To
/*.(ets|sts)$
language: ts

## Detect

Each runnable (aka "positive") test must contain `arktest.*` assertions to match actually computed values against expected ones.

Runnable tests are found by following rules:

* A test is considered runnable if its code does **not** contain special tags: `compile-only`, `negative`, `@@ ... @@@ ...`
* A test is consideted non-runnable (aka "negative") if its code contains special tags: `compile-only`, `negative`, `@@ ... @@@ ...`

## Example BAD code

```typescript
function foo(b : boolean) : int | string {
    return b === true ? 1 : "1"
}
foo(true)
foo(false)
```

## Example GOOD code

```typescript
function foo(b : boolean) : int | string {
    return b === true ? 1 : "1"
}

arktest.assertTrue(foo(true) instanceof Int)
arktest.assertTrue(foo(false) instanceof String)
arktest.assertEQ(foo(true), 1)
arktest.assertEQ(foo(false), "1")
```

## Fix suggestion

Introduce reasonable `arktest.*` assertions to match actually computed values against expected ones.

## Message

Each runnable (aka "positive") test must contain `arktest.*` assertions to match actually computed values against expected ones.
