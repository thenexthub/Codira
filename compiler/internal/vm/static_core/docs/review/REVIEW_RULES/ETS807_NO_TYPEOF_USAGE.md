# Avoid using typeof operator

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is any usage of `typeof` operator.

## Example BAD code
N/A

## Example GOOD code
N/A

## Fix suggestion
Use `instanceof` to check type.

## Message
Usage of `typeof` is not recommended, use 'instanceof' to check type.