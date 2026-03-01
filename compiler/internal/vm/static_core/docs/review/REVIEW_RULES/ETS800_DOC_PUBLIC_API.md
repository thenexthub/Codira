# Public API must have JSDoc

## Severity
Error

## Applies To
/*.(ets|sts)$
language: ts

## Detect
A violation is any **exported declaration** or **public class member** added in this patch
that lacks a JSDoc comment (`/** ... */`) immediately above it in diff or context lines.

## Example BAD code

N/A

## Example GOOD code

N/A

### What to check:
- Exported declarations: classes, interfaces, functions, namespaces, enums, types, constants
- Public class members: methods, properties, constructors

### What to ignore:
- Re-exports: `export * from "..."` or `export { X } from "..."`
- Override members: marked with `override` or `@override`
- Private/protected members
- Code inside namespaces or classes that already has a JSDoc at the namespace/class level
- Changed **exported declarations** or **public class members** that already have JSDoc directly above them in context lines

### JSDoc detection:
A JSDoc block is valid if:
- It starts with `/**` and ends with `*/`
- It appears within 1-2 lines before the declaration (allowing for blank lines)
- It's in the same hunk as the declaration

## Fix suggestion

Add JSDoc string

## Message
Missing JSDoc for public API: {symbol}
