# Casting numeric type to enum is prohibited

## Severity
Error

## Applies To
/*.(ets|sts)$
language: ts

## Detect

A violation is any **cast expression** with the **as** operator between numeric type and enum type.

Types whose declaration is not available in the patch shouldn't be checked.

## Example BAD code

```typescript
enum E {
    A,
    B,
    C
}

function foo(e: E, i: int) {
    let x = e as int
    let y = i as E
}
```

## Example GOOD code

```typescript
enum E {
    A,
    B,
    C
}

function foo(e: E, i: int) {
    let x = e.valueOf()
    let y = E.fromValue(i)
}
```

## Fix suggestion

* Use `<Expression>.valueOf()` form when a value of an enum type is converted to a value of a numeric type.
* Use `<EnumType>.fromValue(<Expression>)` form when a value of a numeric type is converted to a value of an enum type.

## Message

Invalid cast from '{ExpressionType}' to '{TargetType}', `enum` casts are not supported. Use '{ConversionMethodCallExpression}' expression instead.
