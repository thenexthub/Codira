# Avoid using an exponential notation for integer values

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is usage of exponential notation to create an integer value. Since this form always produces a floating-point value, any integer and large whole number should be written in standard form.

## Example BAD code
```typescript
const MAX_VAL: long = (8.64e15).toLong();
```

## Example GOOD code
```typescript
const MAX_VAL: long = 8_640_000_000_000_000;
```

## Fix suggestion
Write integer and large whole numbers in standard form.

## Message
An exponential notation can't be used to create an integer value.