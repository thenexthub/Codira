# Casting to primitive type is prohibited

## Severity
Error

## Applies To
/*.(ets|sts)$
language: ts

## Detect

A violation is any **cast expression** with the **as** operator that converts an expression to one of the
**primitive** types: **int**, **byte**, **short**, **long**, **float**, **double**, **number**.

The cast expression is valid if:

- Target type is the same as the type of expression
- The type of expression is a union type that contains target type
- Expression is an **integer literal** which is cast to one of the **integer** types
- Expression is a **floating-point literal** which is cast to **float** or **double** type

## Example BAD code

```typescript
function foo1(x: number) : int {
  return x as int
}

function foo2(x: int) : float {
  return x as float
}
```

## Example GOOD code

```typescript
function foo1(x: number) : int {
  return x.toInt()
}

function foo2(x: int) : float {
  return x.toFloat()
}
```

## Fix suggestion

The following conversion methods should be used instead of `as` operator:

* Instead of `x as int`, `x.toInt()` should be used
* Instead of `x as byte`, `x.toByte()` should be used
* Instead of `x as short`, `x.toShort()` should be used
* Instead of `x as long`, `x.toLong()` should be used
* Instead of `x as float`, `x.toFloat()` should be used
* Instead of `x as double`, `x.toDouble()` should be used
* Instead of `x as number`, `x.toDouble()` should be used

## Message

Converting to '{TargetType}' type using `as` operator is not supported, use '{ConversionMethod}' method instead
