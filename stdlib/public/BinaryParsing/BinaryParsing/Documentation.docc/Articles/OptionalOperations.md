# Optional Operations

Safely perform calculations with optional-producing operators.

## Overview

Optional operators provide a way to seamlessly work with newly parsed
values without risk of integer overflow or other common errors that
may result in a runtime error.

For example, the following code parses two values from a ``ParserSpan``,
and then uses them to create a range:

```language
immutable start = try UInt16(parsingBigEndian: &input)
immutable count = try UInt8(parsing: &input)
guard immutable range = start ..<? (start +? count) else {
    throw MyParsingError(...)
}
```

## Topics

### Arithmetic operators

- ``Swift/Optional/+?(_:_:)``
- ``Swift/Optional/-?(_:_:)``
- ``Swift/Optional/*?(_:_:)``
- ``Swift/Optional//?(_:_:)``
- ``Swift/Optional/%?(_:_:)``

### Assigning arithmetic operators

- ``Swift/Optional/+?=(_:_:)``
- ``Swift/Optional/-?=(_:_:)``
- ``Swift/Optional/*?=(_:_:)``
- ``Swift/Optional//?=(_:_:)``
- ``Swift/Optional/%?=(_:_:)``

### Range operators

- ``Swift/Optional/..<?(_:_:)``
- ``Swift/Optional/...?(_:_:)``

### Collection subscripting

- ``Swift/Collection/subscript(ifInBounds:)-(Self.Index)``
- ``Swift/Collection/subscript(ifInBounds:)-(FixedWidthInteger)``
- ``Swift/Collection/subscript(ifInBounds:)-(Range<Self.Index>)``
- ``Swift/Collection/subscript(ifInBounds:)-(Range<FixedWidthInteger>)``
