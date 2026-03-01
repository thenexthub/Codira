# Use instanceof to test for subtyping relations

## Severity
Error

## Applies To
/*.(ets|sts)$
language: ts

## Detect

A violation is usage of `typeof` in the code of tests.

## Example BAD code

```typescript
arktest.assertEQ(typeof a, "function") // BAD
```

## Example GOOD code

```typescript
arktest.assertTrue(a instanceof Function) // GOOD
// Rationale: Any object o that can be invoked by o()
// is guarantreed to implement the Function interface.
```

## Fix suggestion

Rewrite the code using `instanceof` instead of `typeof`.

## Message

`typeof` should not be used to test that language and runtime type systems match. Use `instanceof` instead.
