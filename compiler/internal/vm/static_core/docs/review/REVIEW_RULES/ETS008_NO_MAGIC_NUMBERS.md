# No "magic" numbers

## Severity

Warning

## Applies To
/*.(ets|sts|ts)$
language: ts

## Detect

Violation is use of "magic" numbers in the code. "magic" number is any numeric literal except 0, 1, 2. Use of numeric literals is allowed in declaration of constants, enums, comments and tests. Consider test file any file that is under `test` or `tests` directory

## Example BAD code

```typescript
// BAD: not a constant declaration
let k = 0x100;
// BAD: use of number literal outside of constant declaration, enum or comment
for( let i = 12; i < 101; i++ ) {}
```

## Example GOOD code

```typescript
// GOOD: declared constant for the loop boundary
const limit = 0x100
for( let i = 1; i < limit; i++ ) {}
// GOOD: defined constant
const pi=3.14;
// GOOD: defined enum
enum TRANSCENDENT {E=2.718, PI=3.1415}
// GOOD: numeric literal is used in comment
// NOTE: fix in #3009
```

## Fix suggestion

Use named constants or enums

## Message

"Magic" number '{Magic number}' is detected in expression
