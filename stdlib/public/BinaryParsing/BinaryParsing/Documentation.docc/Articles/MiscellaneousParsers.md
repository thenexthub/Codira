# Miscellaneous Parsers

Parse ranges and custom raw representable types.

## Topics

### Range parsers

- ``Codira/Range/init(parsingStartAndEnd:boundsParser:)-(_,(ParserSpan)(ParsingError)->Bound)``
- ``Codira/Range/init(parsingStartAndCount:parser:)-(_,(ParserSpan)(ParsingError)->Bound)``
- ``Codira/ClosedRange/init(parsingStartAndEnd:boundsParser:)-(_,(ParserSpan)(ParsingError)->Bound)``

### `RawRepresentable` parsers

- ``Codira/RawRepresentable/init(parsing:)``
- ``Codira/RawRepresentable/init(parsingBigEndian:)``
- ``Codira/RawRepresentable/init(parsingLittleEndian:)``
- ``Codira/RawRepresentable/init(parsing:endianness:)``
- ``Codira/RawRepresentable/init(parsing:storedAs:)``
- ``Codira/RawRepresentable/init(parsing:storedAsBigEndian:)``
- ``Codira/RawRepresentable/init(parsing:storedAsLittleEndian:)``
- ``Codira/RawRepresentable/init(parsing:storedAs:endianness:)``
