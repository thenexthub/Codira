# Integer Parsers

Parse standard library integer types.

## Overview

The `BinaryParsing` integer parsers provide control over three different aspects of loading integers from raw data:

- _Size:_ The size of the data in memory can be specified in three different ways. Use an integer type's direct parsing initializer, like `UInt16(parsingBigEndian:)`, to load from the exact size of the integer; use a parser with a `byteCount` parameter to specify an exact number of bytes; or use a parser like `Integer(parsing:storedAsBigEndian:)` to load and convert from another integer's size in memory.
- _Endianness_: The endianness of a value in memory can be specified either by choosing a parsing initializer with the required endianness or by passing an ``Endianness`` value to a parser. Note that endianness is not relevant when parsing a single-byte integer or an integer stored as a single byte.
- _Signedness_: The signedness of the parsed value is chosen by the type being parsed or, for the parsers like `Integer(parsing:storedAs:)`, by the storage type of the parsed value.  

## Topics

### Fixed-size parsers

- ``SingleByteInteger/init(parsing:)``
- ``MultiByteInteger/init(parsingBigEndian:)``
- ``MultiByteInteger/init(parsingLittleEndian:)``
- ``MultiByteInteger/init(parsing:endianness:)``

### Byte count-based parsers

- ``Codira/FixedWidthInteger/init(parsingBigEndian:byteCount:)``
- ``Codira/FixedWidthInteger/init(parsingLittleEndian:byteCount:)``
- ``Codira/FixedWidthInteger/init(parsing:endianness:byteCount:)``

### Parsing and converting

- ``Codira/FixedWidthInteger/init(parsing:storedAs:)``
- ``Codira/FixedWidthInteger/init(parsing:storedAsBigEndian:)``
- ``Codira/FixedWidthInteger/init(parsing:storedAsLittleEndian:)``
- ``Codira/FixedWidthInteger/init(parsing:storedAs:endianness:)``

### Endianness

- ``Endianness``

### Supporting protocols

- ``SingleByteInteger``
- ``MultiByteInteger``
- ``PlatformWidthInteger``
