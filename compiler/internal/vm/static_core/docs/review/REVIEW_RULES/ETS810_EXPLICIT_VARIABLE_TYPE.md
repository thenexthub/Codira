# Explicit variable type

## Severity

Warning

## Applies To

/*.(ets|sts)$
language: ts

## Detect

A violation is declaration of variable, constant or class field without explicit type annotation.

## Example BAD code
```typescript
// BAD: no type annotation in form `<varName>:<TypeName>`
let count = 0;
// BAD: no type annotation in form `<constName>:<TypeName>`
const MAX_VAL = 8_640_000_000_000_000;

class C {
    // BAD: no type annotation in form `<fieldName>:<TypeName>`
    public count = 5;
}
```

## Example GOOD code
```typescript
// GOOD: explicit type `long` specified
const MAX_VAL: long = 8_640_000_000_000_000;
// GOOD: explicit type `Array<int>` specified
let arr: Array<int> = []

class Good {
    // GOOD: explicit type `number` specified
    public count: number = 7.1
}
```

## Fix suggestion
Add variable type.

## Message
Defining variable type in declaration is mandatory.
