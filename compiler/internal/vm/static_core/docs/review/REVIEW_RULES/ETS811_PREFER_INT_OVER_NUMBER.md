# Prefer int over number when only integer values are valid

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is usage of `number` type in contexts where naturally only integer values would be valid:
- a value used as array index expression;
- a counter variable, initialized with integer value and changed by incrementing/decrementing;
- used to change an array length;
- used as an iteration variable of `for` loop in the form `for (initialization; condition; increment/decrement) { ... }`.

In such scenarios, an `int` type is preferred and should be used instead.

## Example BAD code
```typescript
function printArray(arr: Array<int>) {
    for (let i: number = 0; i < 10; i++) {
        console.log(arr[i.toInt()])
    }
}
```

## Example GOOD code
```typescript
function printArray(arr: Array<int>) {
    for (let i: int = 0; i < 10; i++) {
        console.log(arr[i])
    }
}
```

## Fix suggestion
Replace `number` with `int` type

## Message
Encountered type 'number' where only integer values are valid. It is recommended to use type 'int' instead