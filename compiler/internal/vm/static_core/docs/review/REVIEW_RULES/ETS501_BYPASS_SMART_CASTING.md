# Avoid smart casts in ArkTS test suites

## Severity
Error

## Applies To
/*.(ets|sts)$
language: ts

## Detect

A violation is declaration of any variable which type can be clarified in compile-time using the right side of the assignment.

## Example BAD code

```typescript
let x: object = 123n
arktest.assertEQ(typeof 123n == "bigint") /* BAD: may be computed statically by frontend */
arktest.assertEQ(typeof x == "bigint")    /* BAD: the same because "x" has a smart type */

let y: string | int | double = 1 /* BAD: x has a smart type computed from the right side of the assignment */
arktest.assertEQ(y == 1)         /* BAD: the whole test is put into a single function, which breaks user-invisible boxing */

type A = Any | string
function main(): void {
    let a: A = 5
    arktest.assertEQ(typeof a, "number")
}
```

## Example GOOD code

```typescript
// GOOD: wrap into a function to by pass smart casting
function foo(x: string | int | double, y: int) : boolean {
    return x == y
}
arktest.assertTrue(foo(1, 1))

// GOOD: wrap into a lambda (for the same reasons)
arktest.assertTrue(((v: string | int | double) => v)(1) == 1)

// GOOD: wrap a return value into a function
function getFive() : Any {
  return 5
}
let a: Any | string = getFive()
arktest.assertTrue(a instanceof Int)
```

## Fix suggestion

Wrap plain values into helper functions.

Currently in the language we do not allow to optimizie (in any sense) across the functions. So the function call is a "concerete wall" for the front-end: it looks at the function's return value at the call-site and uses only this information to proceed further with the call site. With aux functions, there will be no "side effects" on your tests that may be induced by the front-end. This is the best way to test that language and runtime type systems match.

## Message

Type of '{Identifier}' is a subject to compile-time smart casting. Use helper functions to bypass this effect.
