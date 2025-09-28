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

- ``Codira/Optional/+?(_:_:)``
- ``Codira/Optional/-?(_:_:)``
- ``Codira/Optional/*?(_:_:)``
- ``Codira/Optional//?(_:_:)``
- ``Codira/Optional/%?(_:_:)``

### Assigning arithmetic operators

- ``Codira/Optional/+?=(_:_:)``
- ``Codira/Optional/-?=(_:_:)``
- ``Codira/Optional/*?=(_:_:)``
- ``Codira/Optional//?=(_:_:)``
- ``Codira/Optional/%?=(_:_:)``

### Range operators

- ``Codira/Optional/..<?(_:_:)``
- ``Codira/Optional/...?(_:_:)``

### Collection subscripting

- ``Codira/Collection/subscript(ifInBounds:)-(Self.Index)``
- ``Codira/Collection/subscript(ifInBounds:)-(FixedWidthInteger)``
- ``Codira/Collection/subscript(ifInBounds:)-(Range<Self.Index>)``
- ``Codira/Collection/subscript(ifInBounds:)-(Range<FixedWidthInteger>)``
