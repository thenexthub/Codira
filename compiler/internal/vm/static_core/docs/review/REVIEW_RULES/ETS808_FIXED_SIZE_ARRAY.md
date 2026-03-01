# Prefer using FixedArray<T> when array size doesn't change

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation occurs when `Array<T>` type is used to create an array object which:
- never changes the value of `length` property and
- never calls any method

In such scenario, using the `Array<T>` type is excessive, and the `FixedArray<T>` type is recommended instead as a more optimal for a fixed-size array usage scenario.

## Example BAD code
```typescript
function foo(l: number) {
    let arr = new Array<number>(l);
    for (let i = 0; i < l; i++>) {
        arr[i] = 1 << i;
    }
}
```

## Example GOOD code
```typescript
function foo(l: number) {
    let arr = new FixedArray<number>(l);
    for (let i = 0; i < l; i++>) {
        arr[i] = 1 << i;
    }
}
```

## Fix suggestion
Replace `Array<T>` with `FixedArray<T>` type

## Message
Usage of 'Array<{ElementType}>' type is excessive. It is recommended to use 'FixedArray<{ElementType}>' type instead