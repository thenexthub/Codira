# Declare overloaded functions explicitly

## Severity
Warning

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is usage of special syntax that creates a new overload set from existing functions and binds them with a common alias name: `overload <AliasName> { <Function1>, <Function2>, ... }`.

## Example BAD code
```typescript
function f1(a: int): void {}
function f2(a: int, b: string): void {}
overload f {f1, f2};

f(1);       // calls 'f1'
f(1, '2');  // calls 'f2'
```

## Example GOOD code
```typescript
function f(a: int): void {}
function f(a: int, b: string): void {}

f(1);       // calls 'f(int)' overload
f(1, '2');  // calls 'f(int, string)' overload
```

## Fix suggestion
Replace overload set with explicitly overloaded functions

## Message
Using the special overloading syntax is prohibited. Declare overloaded functions explicitly.