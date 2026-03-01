# ASTChecker

`ASTChecker` is a test suite for precise AST testing.

Tests are defined according to the following grammar:

```
<checker-statement>: "/*" "@@" <directive> "*/"
<directive>: <match-pattern> | <match-at-location-pattern> | <define-pattern> | <skip-option>*
<match-pattern>: <id> | <builtin-pattern>
<match-at-location-pattern>: "?" <location> <builtin-pattern>
<define-pattern>: "@" <id> <builtin-pattern>
<location>: <line> ":" <col>
<builtin-pattern>: "Node" json | "Error" msg | "Warning" msg
<id>: [a-zA-Z0-9_]+
<line>: [0-9]+
<col>: [0-9]+
<skip-option>: <key> '=' <val>
<key>: "SkipErrors"|"SkipWarnings"
<val>: "true"|"false"
```

## Test types by definition



### Reference-defined tests
Reference-defines tests consist of two parts: `match` pattern and `define` pattern.
`Define` pattern:

`/* @@@ <id> <test-type> <properties> */`
where
 - `<id>` may contain letters, numbers and underscores
 - `<test-type>` is `Node`, `Error` or `Warning`
 - `<properties>` is a JSON containing keys and values to be checked in AST node for Node tests and error message for Error and Warning tests

Each `define` pattern should have corrensponding `match` pattern: 

`/* @@ <id> */` with the same `<id>`. This pattern has to be put **before** expression/statement to be tested (see examples below). 
 

### Inplace-defined tests

Inplace have only `match` pattern:

`/* @@ <test-type> <properties> */`
where
- `<test-type>` is `Node`, `Error` or `Warning`
- `<properties>` is a JSON containing keys and values to be checked in AST node for Node tests and error message for Error and Warning tests

This pattern  has to be put exactly before the expression/statement to be tested.

### Location-defined tests
This type of tests is similar to inplace-defined, as they don't have `define` pattern.
`Match-at-location` pattern can be defined anywhere in the file and is bound to the location it specifies

`/* @@? <line>:<col> <test-type> <properties> */`
- `<line>` and `<col>` are line and column (counting from 1), specifying the location to bind the test.
- `<test-type>` is `Node`, `Error` or `Warning`
- `<properties>` is a JSON containing keys and values to be checked in AST node for Node tests and error message for Error and Warning tests

## Test types by testing object

### Node testing
Node test is passed if a node, that meets the following requirements, is present in AST of a test file:
1. contains all (`key`: `value`) pairs, specified in a `<properties>` JSON (Note that such node may contain **not only** these pairs)
2. its location ends at the location of a test

Node test examples:
```
/* @@ Node {"type": "MethodDefinition", "computed": false} */function /* @@ IdentTest */foo(a: number): number{
    return a + 1;
}


/* @@@ IdentTest Node {
    "type": "Identifier",
    "name": "foo"
} */ 
```

### Error and Warning testing
Error tests check that `es2panda` fails with a specified error/raises a warning, at a specified location:

```
function foo(): void {
  let a = new int[/* @@ Error Semantic error: Index fractional part should be zero. */5.1]
}

```
The same test as a `Match-at-location` pattern: 
```
function foo(): void {
  let a = new int[5.1]
}

/* @@? 2:19 Error Semantic error: Index fractional part should be zero. */
```

## Errors and Warnings skipping

By default "SkipErrors" and "SkipWarnings" are set to "false"

If the option "SkipErrors" is set to "true" there is no matching of expected errors with actual ones:
```
function foo(): void {
  let a = new int[5.1]
}

/* @@? 2:19 Error Semantic error: Index fractional part should be zero. */
/* @@# SkipErrors = true  */
```

If the option "SkipWarnings" is set to "true" there is no matching of expected warnings with actual ones:
```
function main(): void {
    try {
        try {
            throw new Exception();
        } catch (e) {}
        finally {
            throw new Exception();
        }
    } catch (e) {}
}

/* @@? 6:17 Warning Warning: Finally clause cannot complete normally  */
/* @@# SkipWarnings = true  */
```

Both options can be set to "true" at the same time:
```
function foo(): void {
  let a = new int[5.1]
}

function main(): void {
    try {
        try {
            throw new Exception();
        } catch (e) {}
        finally {
            throw new Exception();
        }
    } catch (e) {}
}

/* @@? 2:19 Error Semantic error: Index fractional part should be zero. */
/* @@? 10:17 Warning Warning: Finally clause cannot complete normally  */
/* @@# SkipErrors = true  SkipWarnings = true  */
```
