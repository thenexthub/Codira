# Avoid using for..of syntax for iterating

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is a `for` loop in form `for (variable of expression) { ... }`.

## Example BAD code
```typescript
function foo(arr: Array<string>) {
    for (let s of arr) {
        console.log(s);
    }
}
```

## Example GOOD code
```typescript
function foo(arr: Array<string>) {
    let len = arr.length;
    for (let i = 0; i < len; i++) {
        console.log(arr[i]);
    }
}
```

## Fix suggestion
Use traditional `for` loop syntax.

## Message
Usage of `for..of` loop syntax is not recommended. Use traditional `for` loop to iterate over collection if possible.