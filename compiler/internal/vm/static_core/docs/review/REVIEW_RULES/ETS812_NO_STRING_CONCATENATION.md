# Use StringBuilder to concatenate strings

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is a string concatenation using `+` operator. Instead, a `StringBuilder` type should be used to concatenate strings, which is more optimized for string manipulation.

## Example BAD code
```typescript
function foo(str1: string, str2: string): string {
    return str1 + str2;
}
```

## Example GOOD code
```typescript
function foo(str1: string, str2: string): string {
    const sb = new StringBuilder(str1);
    sb.append(str2);
    return sb.toString();
}
```

## Fix suggestion
Replace `+` operator with `StringBuilder` API

## Message
Encountered string concatenation with '+' operator. Use 'StringBuilder' class for string manipulation.