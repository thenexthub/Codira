# Panda Binary File Format

This document describes the Panda binary file format with the following goals
in mind:

* Compactness,
* Support for fast access to information,
* Support for low memory footprint, and
* Extensibility and compatibility.

|

## Compactness

Many mobile applications use numerous types, methods and fields. Their number
is so large that it does not fit into a 16-bit unsigned integer. It makes an
application developer create several files, and not all data can be deduplicated
as a result.

Current binary file format must extend these limits to conform to modern
requirements. To achieve this, all references in a binary file are 4 bytes long.
It allows having 4Gb for addressing fields, methods, classes, etc.

The format uses [TaggedValue](#taggedvalue) which allows storing only the
information available and avoids 0 offsets to any missing information.

In order to achieve greater compactness, 16-bit indices are used to refer to
classes, methods and fields in the bytecode and some metadata. A file can
contain multiple indices, each one covering a part of the file as described
in [RegionHeader](#regionheader).

|

## Fast information access

The binary file format must support fast access to information. It means that
redundant references are to be avoided. Where possible, the binary file format
must also avoid data indices (like sorted list of strings). However, the binary
format described above supports one index: a sorted list of offsets to classes.
This index is compact and allows finding type definitions quickly (while finding
type definitions normally requires much runtime when launching an application).

All classes, fields, and methods are foreign or local as explained below:

-  *Foreign* classes, fields, and methods are declared in other files and
   referenced from the current binary file.
-  *Local* classes, fields and methods are declared within the current file.
   A local entity has the same header as the corresponding foreign entity.

It is irrelevant for an entity offset whether the entity is local or foreign.

Runtime can easily check the offset type by checking it in the foreign region
(*\[foreign_off; foreign_off + foreign_size)*). Depending on the result, the
following occurs at runtime:

-  In case of a foreign entity, the search for an entity is performed in other
   files, or
-  In case of a local entity, a runtime object is created from the definition
   by the offset.

Most data structures have the 4-byte alignment in order to improve the speed of
data access.

All multibyte values are little-endian as most target architectures are.

|

## Offsets

Unless otherwise specified, all offsets are calculated from the beginning of
the file.

An offset cannot contain values in the range *\[0; 32)*, except in some cases
mentioned specifically.

|

## Support for low memory footprint

As practice proves, applications make no use of most file data. Memory footprint
of a file can be significantly reduced as a consequence by grouping the data
that is used frequently. To support this feature, the described binary file
format uses offsets and does not specify how structures must be located
in relation to one another.

|

## Extensibility and compatibility

The binary file format supports future changes via version number.
The version field in the header is 4 bytes long and is encoded as byte array to
avoid misinterpretation on platforms with different endianness.

Any tool which supports format version `N` must support format version `N - 1` too.

|

## Data types

| Type       | Description                                  |
| ----       | -----------                                  |
| `uint8_t`  | 8-bit unsigned integer value                 |
| `uint16_t` | 16-bit unsigned integer value                |
| `uint32_t` | 32-bit little endian unsigned integer value  |
| `uleb128`  | unsigned integer value in leb128 encoding    |
| `sleb128`  | signed integer value in leb128 encoding      |

|

### MUTF-8 Encoding

Binary file format uses MUTF-8 (Modified UTF-8) encoding for strings.

|

### String

Alignment: none

Format:

| Name               | Format        | Description |
| ----               | ------        | ----------- |
| `utf16_length`     | `uleb128`     | `len << 1 \| is_ascii` where `len` is the length of the string in UTF-16 code units |
| `data`             | `uint8_t[]`   | 0-terminated character sequence in MUTF-8 encoding |

|

### TaggedValue

Alignment: none

Format:

| Name               | Format        | Description |
| ----               | ------        | ----------- |
| `tag_value`        | `uint8_t`     | The first 8 bits contain tag which determines the meaning of the data. Depending on the tag, optional data is possible. Runtime must be able to determine the size of data. |
| `data`             | `uint8_t[]`   | Optional payload |

|

## String syntax

|

### TypeDescriptor

```
TypeDescriptor -> PrimitiveType | ArrayType | RefType
PrimitiveType  -> 'Z' | 'B' | 'H' | 'S' | 'C' | 'I' | 'U' | 'J' | 'Q' | 'F' | 'D' | 'A'
ArrayType      -> '[' TypeDescriptor
RefType        -> 'L' ClassName ';'
```

`PrimitiveType` is a one-letter encoding for a primitive type:

| Type  | Encoding |
| ----  | :--: |
| `u1`  | `Z` |
| `i8`  | `B` |
| `u8`  | `H` |
| `i16` | `S` |
| `u16` | `C` |
| `i32` | `I` |
| `u32` | `U` |
| `f32` | `F` |
| `f64` | `D` |
| `i64` | `J` |
| `u64` | `Q` |
| `any` | `A` |

`ClassName` is a qualified name of a class with `.` replaced with `/`.

|

## Access flags

#### Field access flags

| Name             | Value    | Description |
| ----             | :---:    | ----------- |
| `ACC_PUBLIC`     | `0x0001` | Declared public; can be accessed from outside its package. |
| `ACC_PRIVATE`    | `0x0002` | Declared private; usable only within the defining class. |
| `ACC_PROTECTED`  | `0x0004` | Declared protected; can be accessed within subclasses. |
| `ACC_STATIC`     | `0x0008` | Declared static. |
| `ACC_FINAL`      | `0x0010` | Declared final; never directly assigned to after object construction (JLS §17.5). |
| `ACC_READONLY`   | `0x0020` | Declared readonly; assigned exactly once during object construction. |
| `ACC_VOLATILE`   | `0x0040` | Declared volatile; cannot be cached. |
| `ACC_TRANSIENT`  | `0x0080` | Declared transient; not written or read by a persistent object manager. |
| `ACC_SYNTHETIC`  | `0x1000` | Declared synthetic; not present in the source code. |
| `ACC_ENUM`       | `0x4000` | Declared as an element of an enum. |

|

#### Method access flags

| Name               | Value    | Description |
| ----               | :---:    | ----------- |
| `ACC_PUBLIC`       | `0x0001` | Declared public; can be accessed from outside its package. |
| `ACC_PRIVATE`      | `0x0002` | Declared private; accessible only within the defining class. |
| `ACC_PROTECTED`    | `0x0004` | Declared protected; can be accessed within subclasses. |
| `ACC_STATIC`       | `0x0008` | Declared static. |
| `ACC_FINAL`        | `0x0010` | Declared final; must not be overridden. |
| `ACC_SYNCHRONIZED` | `0x0020` | Declared synchronized; invocation is wrapped by a monitor use. |
| `ACC_BRIDGE`       | `0x0040` | A bridge method, generated by the compiler. |
| `ACC_VARARGS`      | `0x0080` | Declared with variable number of arguments. |
| `ACC_NATIVE`       | `0x0100` | Declared native; |
| `ACC_ABSTRACT`     | `0x0400` | Declared abstract; no implementation is provided. |
| `ACC_STRICT`       | `0x0800` | Declared strictfp; floating-point mode is FP-strict. |
| `ACC_SYNTHETIC`    | `0x1000` | Declared synthetic; not present in the source code. |

|

#### Class access flags

| Name             | Value    | Description |
| ----             | :---:    | ----------- |
| `ACC_PUBLIC`     | `0x0001` | Declared public; can be accessed from outside its package. |
| `ACC_FINAL`      | `0x0010` | Declared final; no subclasses allowed. |
| `ACC_SUPER`      | `0x0020` | No special meaning, exists for compatibility |
| `ACC_INTERFACE`  | `0x0200` | Is an interface, not a class. |
| `ACC_ABSTRACT`   | `0x0400` | Declared abstract; must not be instantiated. |
| `ACC_SYNTHETIC`  | `0x1000` | Declared synthetic; not present in the source code. |
| `ACC_ANNOTATION` | `0x2000` | Declared as an annotation type. |
| `ACC_ENUM`       | `0x4000` | Declared as an enum type. |

|

## Source language

A file can be emitted from sources that are written in following languages:

| Name             | Value         |
| ----             | ----          |
| Reserved         | `0x00`        |
| `Panda Assembly` | `0x01`        |
| Reserved         | `0x02 - 0x0f` |


Source language can be specified for classes and methods. For classes,
Panda Assembly language is assumed by default. Default language for methods
is that of the class.

|

## Data layout

A file begins with the header which is located at offset 0.
All other data can be reached from the header.

|

### Header

Alignment: 4 bytes

Format:

| Name                   | Format          | Description |
| ----                   | ------          | ----------- |
| `magic`                | `uint8_t[8]`    | Magic string. Must be 'P' 'A' 'N' 'D' 'A' '\\0' '\\0' '\\0' |
| `checksum`             | `uint8_t[4]`    | adler32 checksum of the file except magic and checksum fields. |
| `version`              | `uint8_t[4]`    | Version of the format. Current version is 0006. |
| `file_size`            | `uint32_t`      | Size of the file in bytes. |
| `foreign_off`          | `uint32_t`      | Offset to a foreign region. The region must contain elements of types only [ForeignField](#foreignfield), [ForeignMethod](#foreignmethod), or [ForeignClass](#foreignclass). Using `foreign_off` points to the first entity is not necessary. Runtime must use `foreign_off` and `foreign_size` to determine the type of offset. |
| `foreign_size`         | `uint32_t`      | Size of the foreign region in bytes. |
| `num_classes`          | `uint32_t`      | Number of classes defined in the file. This is also the number of elements in the [ClassIndex](#classindex) structure. |
| `class_idx_off`        | `uint32_t`      | Offset to the class index structure. The offset must point to a structure in [ClassIndex](#classindex) format. |
| `num_export_table`    | `uint32_t`      | Number of exported classes defined in the file. The number can also be zero since the field is optional.  |
| `export_table_off`    | `uint32_t`      | Offset to the exported class index structure. The offset must point to a structure in [ClassIndex](#classindex) |
| `num_lnps`             | `uint32_t`      | Number of line number programs in the file. This is also the number of elements in the [LineNumberProgramIndex](#linenumberprogramindex) structure. |
| `lnp_idx_off`          | `uint32_t`      | Offset to the line number program index structure. The offset must point to a structure in [LineNumberProgramIndex](#linenumberprogramindex) format. |
| `num_literalarrays`    | `uint32_t`      | Number of literalArrays defined in the file. This is also the number of elements in the [LiteralArrayIndex](#literalarrayindex) structure. |
| `literalarray_idx_off` | `uint32_t`      | Offset to the literalarray index structure. The offset must point to a structure in [LiteralArrayIndex](#literalarrayindex) format. |
| `num_index_regions`    | `uint32_t`      | Number of the index regions in the file. This is also the number of elements in the [RegionIndex](#regionindex) structure. |
| `index_section_off`    | `uint32_t`      | Offset to the index section. The offset must point to a structure in [RegionIndex](#indexheaderindex) format. |

Constraint: size of header must be > 16 bytes. [FieldType](#fieldType) uses this fact.

|

### RegionHeader

To address file structures using 16-bit indices, the file is split into regions. Each region has a class, a method, a field, and proto indices. It is described in the `RegionHeader` structure.

Alignment: 4 bytes

Format:

| Name              | Format     | Description |
| ----              | ------     | ----------- |
| `start_off`       | `uint32_t` | Start offset of the region. |
| `end_off`         | `uint32_t` | End offset of the region. |
| `class_idx_size`  | `uint32_t` | Number of elements in the [ClassRegionIndex](#classregionindex) structure. Max value is 65536. |
| `class_idx_off`   | `uint32_t` | Offset to the class index structure. The offset must point to a structure in [ClassRegionIndex](#classregionindex) format. |
| `method_idx_size` | `uint32_t` | Number of elements in the [MethodRegionIndex](#methodregionindex) structure. Max value is 65536. |
| `method_idx_off`  | `uint32_t` | Offset to the method index structure. The offset must point to a structure in [MethodRegionIndex](#methodregionindex) format. |
| `field_idx_size`  | `uint32_t` | Number of elements in the [FieldRegionIndex](#fieldregionindex) structure. Max value is 65536. |
| `field_idx_off`   | `uint32_t` | Offset to the field index structure. The offset must point to a structure in [FieldRegionIndex](#fieldregionindex) format. |
| `proto_idx_size`  | `uint32_t` | Number of elements in the [ProtoRegionIndex](#protoregionindex) structure. Max value is 65536. |
| `proto_idx_off`   | `uint32_t` | Offset to the proto index structure. The offset must point to a structure in [ProtoRegionIndex](#protoregionindex) format. |

Constraint: regions must not overlap each other.

|

### RegionIndex

`RegionIndex` structure is aimed to allow runtime to find index structure that covers specified offset in the file.
It is organized as an array of [RegionHeader](#regionheader) structures.
All regions are sorted by the start offset of the region. Number of elements in the index is `num_index_regions`
from [Header](#header).

Alignment: 4 bytes

|

### ForeignField

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `class_idx`        | `uint16_t`      | Index of the declaring class in a [`ClassRegionIndex`](#classregionindex) structure. Corresponding index entry must be an offset to a [Class](#class) or a [ForeignClass](#foreignclass). |
| `type_idx`         | `uint16_t`      | Index of the field's type in a [`ClassRegionIndex`](#classregionindex) structure. Corresponding index entry must be in [FieldType](#fieldtype) format. |
| `name_off`         | `uint32_t`      | Offset to the name of the field. The offset must point to a [String](#string) |
| `access_flags`     | `uleb128`       | Access flags of the field. The value must be a combination of the [Field access flags](#field-access-flags). For foreign fields, only `ACC_STATIC` flag is used, other flags should be ignored. |

**Note**. Proper region index to resolve `class_idx` and `type_idx` can be found by a foreign field offset.

|

### Field

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `class_idx`        | `uint16_t`      | Index of the declaring class in a [`ClassRegionIndex`](#classregionindex) structure. Corresponding index entry must be an offset to a [Class](#class). |
| `type_idx`         | `uint16_t`      | Index of the field's type in a [`ClassRegionIndex`](#classregionindex) structure. Corresponding index entry must be in [FieldType](#fieldtype) format. |
| `name_off`         | `uint32_t`      | Offset to the name of the field. The offset must point to a [String](#string) |
| `access_flags`     | `uleb128`       | Access flags of the field. The value must be a combination of the [Field access flags](#field-access-flags). |
| `field_data`       | `TaggedValue[]` | Variable length list of tagged values. Each element must have type [TaggedValue](#taggedvalue). Tag must have values from [FieldTag](#fieldtag), and follow the order of increasing tag (except `0x00` tag). |

**Note**. Proper region index to resolve `class_idx` and `type_idx` can be found by a field's offset.

|

### FieldTag

| Name                      | Tag    | Quantity | Data format  | Description |
| ----                      | :-:    | :------: | -----------  | ----------- |
| `NOTHING`                 | `0x00` | `1`      | `none`       | No more values. The value with this tag must be the last. |
| `INT_VALUE`               | `0x01` | `0-1`    | `sleb128`    | Integral value of the field. This tag is used when the field has type `boolean`, `byte`, `char`, `short`, or `int`. |
| `VALUE`                   | `0x02` | `0-1`    | `uint8_t[4]` | Contains value in the [Value](#value) format. |
| `RUNTIME_ANNOTATIONS`     | `0x03` | `>=0`    | `uint8_t[4]` | Offset to runtime **visible** annotation of the field. If the field has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `ANNOTATIONS`             | `0x04` | `>=0`    | `uint8_t[4]` | Offset to runtime **invisible** annotation of the field. If the field has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `RUNTIME_TYPE_ANNOTATION` | `0x05` | `>=0`    | `uint8_t[4]` | Offset to runtime **visible** type annotation of the field. If the field has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `TYPE_ANNOTATION`         | `0x06` | `>=0`    | `uint8_t[4]` | Offset to runtime **invisible** type annotation of the field. If the field has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |

**Note**. Only `INT_VALUE` or `VALUE` tags must be present.

|

### FieldType

Any offset in the range `[0; sizeof(Header))` is invalid because the first bytes of the file contain the header, and the size of the header is longer than 16 bytes.
`FieldType` encoding uses this fact to encode primitive types of the field in the low 4 bits.
The value of a non-primitive type is an offset to [Class](#class) or to [ForeignClass](#foreignclass). In either case, [FieldType](#fieldtype) is `uint32_t`.

Primitive types are encoded as follows:

| Type  | Code |
| ----  | :--: |
| `u1`  | 0x00 |
| `i8`  | 0x01 |
| `u8`  | 0x02 |
| `i16` | 0x03 |
| `u16` | 0x04 |
| `i32` | 0x05 |
| `u32` | 0x06 |
| `f32` | 0x07 |
| `f64` | 0x08 |
| `i64` | 0x09 |
| `u64` | 0x0a |
| `any` | 0x0b |

|

### ForeignMethod

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `class_idx`        | `uint16_t`      | Index of the declaring class in a [`ClassRegionIndex`](#classregionindex) structure. Corresponding index entry must be an offset to a [Class](#class) or a [ForeignClass](#foreignclass). |
| `proto_idx`        | `uint16_t`      | Index of the method's prototype in a [`ProtoRegionIndex`](#classregionindex) structure. Corresponding index entry must be an offset to a [Proto](#proto). |
| `name_off`         | `uint32_t`      | Offset to the name of the method. The offset must point to a [String](#string). |
| `access_flags`     | `uleb128`       | Access flags of the method. The value must be a combination of [Method access flags](#method-access-flags). Flag `ACC_STATIC` can be used for any method. Other flags for foreign methods are ignored. |

**Note**. Proper region index to resolve `class_idx` and `proto_idx` can be found by a foreign method's offset.

|

### Method

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `class_idx`        | `uint16_t`      | Index of the declaring class in a [`ClassRegionIndex`](#classregionindex) structure. Corresponding index entry must be an offset to a [Class](#class). |
| `proto_idx`        | `uint16_t`      | Index of the method's prototype in a [`ProtoRegionIndex`](#classregionindex) structure. Corresponding index entry must be an offset to a [Proto](#proto). |
| `name_off`         | `uint32_t`      | Offset to the name of the method. The offset must point to a [String](#string). |
| `access_flags`     | `uleb128`       | Access flags of the method. The value must be a combination of [Method access flags](#method-access-flags). |
| `method_data`      | `TaggedValue[]` | Variable length list of tagged values. Each element must have type [TaggedValue](#taggedvalue). Tag must have values from [MethodTag](#methodtag), and follow the order of increasing tag (except `0x00` tag). |

**Note**. Proper region index to resolve `class_idx` and `proto_idx` can be found by the method's offset.

### MethodTag

| Name                       | Tag     | Quantity  | Data format      | Description |
| ----                       | :-:     | :------:  | -----------      | ----------- |
| `NOTHING`                  | `0x00`  | `1`       | `none`           | No more values. The value with this tag must be the last. |
| `CODE`                     | `0x01`  | `0-1`     | `uint8_t[4]`     | Data represents the offset to method's code. The offset must point to [Code](#code). |
| `SOURCE_LANG`              | `0x02`  | `0-1`     | `uint8_t`        | Data represents the [source language](#source-language). |
| `RUNTIME_ANNOTATION`       | `0x03`  | `>=0`     | `uint8_t[4]`     | Data represents the offset to runtime **visible** annotation of the method. If the method has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `RUNTIME_PARAM_ANNOTATION` | `0x04`  | `0-1`     | `uint8_t[4]`     | Data represents the offset to the runtime **visible** annotations of the method's parameters. The offset must point to the value in [ParamAnnotations](#paramannotations) format. |
| `DEBUG_INFO`               | `0x05`  | `0-1`     | `uint8_t[4]`     | Data represents the offset to debug information related to the method. The offset must point to [InstDebugInfo](#debuginfo). |
| `ANNOTATION`               | `0x06`  | `>=0`     | `uint8_t[4]`     | Data represents the offset to runtime **invisible** annotation of the method. If the method has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `PARAM_ANNOTATION`         | `0x07`  | `0-1`     | `uint8_t[4]`     | Data represents the offset to the runtime **invisible** annotations of the method's parameters. The offset must point to the value in [ParamAnnotations](#paramannotations) format. |
| `TYPE_ANNOTATION`          | `0x08`  | `>=0`     | `uint8_t[4]`     | Data represents the offset to runtime **invisible** type annotation of the method. If the method has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `RUNTIME_TYPE_ANNOTATION`  | `0x09`  | `>=0`     | `uint8_t[4]`     | Data represents the offset to runtime **visible** type annotation of the method. If the method has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `PROFILE_INFO`             | `0x0a`  | `>=0`     | `uint8_t[]`      | Data represents the profile information. Format is unspecified and determined by the language used in the file.

|

### ForeignClass

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `name`             | `String`        | Name of the foreign type. The name must conform to [TypeDescriptor](#typedescriptor) syntax. |

|

### Class

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `name`             | `String`        | Name of the class. The name must conform to [TypeDescriptor](#typedescriptor) syntax. |
| `super_class_off`  | `uint8_t[4]`    | Offset to the name of the super class or `0` for root object class (`panda.Object` in Panda Assembly, plugin-specific in plugins). Non-zero offset must point to a [ForeignClass](#foreignclass) or to a [Class](#class). |
| `access_flags`     | `uleb128`       | Access flags of the class. The value must be a combination of [Class access flags](#class-access-flags). |
| `num_fields`       | `uleb128`       | Number of fields the class has.
| `num_methods`      | `uleb128`       | Number of methods the class has.
| `class_data`       | `TaggedValue[]` | Variable length list of tagged values. Each element must have type [TaggedValue](#taggedvalue). Tag must have values from [ClassTag](#classtag), and follow the order of increasing tag (except `0x00` tag). |
| `fields`           | `Field[]`       | Class fields. Number of elements is `num_fields`. Each element must have [Field](#field) format. |
| `methods`          | `Method[]`      | Class methods. Number of elements is `num_methods`. Each element must have [Method](#method) format. |

#### ClassTag

| Name                      | Tag    | Quantity  | Data format           | Description |
| ----                      | :-:    | :------:  | -----------           | ----------- |
| `NOTHING`                 | `0x00` | `1`       | `none`                | No more values. The value with this tag must be the last. |
| `INTERFACES`              | `0x01` | `0-1`     | `uleb128 uint8_t[]`   | List of interfaces the class implements. Data contains number of interfaces encoded in `uleb128` format followed by indices of the interfaces in a [`ClassRegionIndex`](#classregionindex) structure. Each index is 2 bytes long, and must be resolved to offset point to a [ForeignClass](#foreignclass) or to a [Class](#class). Number of indices is equal to the number of interfaces. |
| `SOURCE_LANG`             | `0x02` | `0-1`     | `uint8_t`             | Data represents the [source language](#source-language). |
| `RUNTIME_ANNOTATION`      | `0x03` | `>=0`     | `uint8_t[4]`          | Offset to runtime **visible** annotation of the class. If the class has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `ANNOTATION`              | `0x04` | `>=0`     | `uint8_t[4]`          | Offset to runtime **invisible** annotation of the class. If the class has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `RUNTIME_TYPE_ANNOTATION` | `0x05` | `>=0`     | `uint8_t[4]`          | Offset to runtime **visible** type annotation of the class. If the class has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `TYPE_ANNOTATION`         | `0x06` | `>=0`     | `uint8_t[4]`          | Offset to runtime **invisible** type annotation of the class. If the class has several annotations, then the tag can be repeated. The offset must point to the value in [Annotation](#annotation) format. |
| `SOURCE_FILE`             | `0x07` | `0-1`     | `uint8_t[4]`          | Offset to a file name string containing source code of this class. |

**Note**. Proper region index to resolve interfaces indices can be found by the class's offset.

|

### LiteralArray

Alignment: none

| Name               | Format        | Description |
| ----               | ------        | ----------- |
| `num_literals`     | `uint32_t`    | num of literals that a literalarray has. |
| `literals`         | `literal[]`   | Array of `literal` in one LiteralArray. The array has `num_literals` elements in [Literal](#literal) format. |

|

### Proto

Alignment: 2 bytes

Format:

| Name               | Format        | Description |
| ----               | ------        | ----------- |
| `shorty`           | `uint16_t[]`  | Short representation of the prototype. Encoding of the ``shorty`` is described in [Shorty](#shorty). |
| `reference_types`  | `uint16_t[]`  | Array of indices of the method's signature non-primitive types. Each non-primitive type in a ``shorty`` has a corresponding element in the array. The size of the array equals the number of reference types in the ``shorty``. |

**Note**. Proper region index to resolve reference types indices can be found by the proto's offset.

|

#### Shorty

*Shorty* is a short description of a method's signature without detailed
information on reference types. A ``shorty`` begins with a return type followed
by method arguments, and ends with `0x0`.

The ``shorty`` syntax is as follows:

```
Shorty -> ReturnType ParamTypeList End
ReturnType -> Type
ParamTypeList -> '' | Type ParamTypeList
Type -> <encoded type>
End -> 0x0
```

`<encoded type>` must be one of:

| Type   | Value   |
| ----   | :---:   |
| `void` | `0x01`  |
| `u1`   | `0x02`  |
| `i8`   | `0x03`  |
| `u8`   | `0x04`  |
| `i16`  | `0x05`  |
| `u16`  | `0x06`  |
| `i32`  | `0x07`  |
| `u32`  | `0x08`  |
| `f32`  | `0x09`  |
| `f64`  | `0x0a`  |
| `i64`  | `0x0b`  |
| `u64`  | `0x0c`  |
| `ref`  | `0x0d`  |
| `any`  | `0x0e`  |

All ``shorty`` elements are divided into groups of 4 elements starting from the
beginning. Each group is encoded in `uint16_t`. Each element is encoded in 4
bits. A group of four elements `v1`, `v2`, ..., `v4` is encoded in `uint16_t`
as follow:

```
| bits   | 0 - 3 | 4 - 7 | 8 - 11 | 12 - 15 |
| ------ | ----- | ----- | ------ | ------- |
| values | v1    | v2    | v3     | v4      |
```

If the group contains less then 4 elements, then the rest bits are filled
with `0x0`.

|

### Code

Alignment: none

Format:

| Name               | Format        | Description |
| ----               | ------        | ----------- |
| `num_vregs`        | `uleb128`     | Number of registers (without argument registers). |
| `num_args`         | `uleb128`     | Number of arguments. |
| `code_size`        | `uleb128`     | Size of instructions in bytes. |
| `tries_size`       | `uleb128`     | Number of try blocks. |
| `instructions`     | `uint8_t[]`   | Instructions. |
| `try_blocks`       | `TryBlock[]`  | Array of try blocks. The array has `tries_size` elements in [TryBlock](#tryblock) format. |

|

### TryBlock

Alignment: none

Format:

| Name               | Format         | Description |
| ----               | ------         | ----------- |
| `start_pc`         | `uleb128`      | Start `pc` of the `try` block. This `pc` points to the first instruction covered by this try block. |
| `length`           | `uleb128`      | Number of instructions covered by the `try` block. |
| `num_catches`      | `uleb128`      | Number of catch blocks associated with the `try` block. |
| `catch_blocks`     | `CatchBlock[]` | Array of `catch` blocks associated with the `try` block. The array has `num_catches` elements in [CatchBlock](#catchblock) format. Catch blocks follows in the order runtime must check the exception type. The `catch all` block, if present, must be the last. |

|

### CatchBlock

Alignment: none

Format:

| Name               | Format        | Description |
| ----               | ------        | ----------- |
| `type_idx`         | `uleb128`     | Index + 1 of the exception's type the block handles in a [`ClassRegionIndex`](#classregionindex) structure or 0 in case of a `catch all` block. The corresponding index entry must be an offset to a [ForeignClass](#foreignclass) or to [Class](#class). If the index is 0, then it is a `catch all` block that catches all exceptions. |
| `handler_pc`       | `uleb128`     | `pc` of the first instruction of the exception handler. |
| `code_size`        | `uleb128`     | Handler's code size in bytes |

**Note**. Proper region index to resolve `type_idx` can be found by the corresponding method's offset.

|

### Annotation

Alignment: none

Format:

| Name               | Format                | Description |
| ----               | ------                | ----------- |
| `class_idx`        | `uint16_t`            | Index of the declaring class in a [`ClassRegionIndex`](#classregionindex) structure. Corresponding index entry must be an offset to a [Class](#class) or a [ForeignClass](#foreignclass). |
| `count`            | `uint16_t`            | The number of name-value pairs in the annotation (number of elements in `elements` array). |
| `elements`         | `AnnotationElement[]` | Array of annotation elements. Each element is in [AnnotationElement](#annotationelement) format. The order of elements must be the same as in the annotation class. |
| `element_types`    | `uint8_t[]`           | Array of annotation element's types. Each element in the array describes the type of `AnnotationElement`. The order of elements in the array matches the order of the `elements` field. |

**Note**. Proper region index to resolve `class_idx` can be found by the annotation's offset.

|

### Tags description

|        Type       | Tag |
| ------------------| --- |
| `u1`              | `1` |
| `i8`              | `2` |
| `u8`              | `3` |
| `i16`             | `4` |
| `u16`             | `5` |
| `i32`             | `6` |
| `u32`             | `7` |
| `i64`             | `8` |
| `u64`             | `9` |
| `f32`             | `A` |
| `f64`             | `B` |
| `string`          | `C` |
| `record`          | `D` |
| `method`          | `E` |
| `enum`            | `F` |
| `annotation`      | `G` |
| `method_handle`   | `J` |
| `array`           | `H` |
| `u1[]`            | `K` |
| `i8[]`            | `L` |
| `u8[]`            | `M` |
| `i16[]`           | `N` |
| `u16[]`           | `O` |
| `i32[]`           | `P` |
| `u32[]`           | `Q` |
| `i64[]`           | `R` |
| `u64[]`           | `S` |
| `f32[]`           | `T` |
| `f64[]`           | `U` |
| `string[]`        | `V` |
| `record[]`        | `W` |
| `method[]`        | `X` |
| `enum[]`          | `Y` |
| `annotation[]`    | `Z` |
| `method_handle[]` | `@` |
| `nullptr_string`  | `*` |

The correct value for element with `nullptr_string` tag is 0 (`\x00\x00\x00\x00`)

|

### AnnotationElement

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `name_off`         | `uint32_t`      | Offset to the element's name. The offset must point to a [String](#string). |
| `value`            | `uint32_t`      | Value of the element. If the annotation element has type ``boolean``, ``byte``, ``short``, ``char``, ``int``, or ``float``, then the field *value* contains the value itself in the corresponding [Value](#value) format. Otherwise, the field contains offset to a [Value](#value). The format of the value can be determined based on element's type. |


### ParamAnnotations

Alignment: none

Format:

| Name               | Format                | Description |
| ----               | ------                | ----------- |
| `count`            | `uint32_t`            | Number of parameters that a method has. This number includes synthetic and mandated parameters. |
| `annotations`      | `AnnotationArray[]`   | Array of annotation lists for each parameter. The array has `count` elements, and each element is in [AnnotationArray](#annotationarray) format. |

|

### AnnotationArray

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `count`            | `uint32_t`      | Number of elements in the array. |
| `offsets`          | `uint32_t[]`    | Array of offsets to parameter annotations. Each offset must refer to an [Annotation](#annotation). The array has `count` elements. |

|

### Value

There are different value encodings depending on the value's type.

|

#### ByteValue

Alignment: None

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t`       | Signed 1-byte integer value. |

|

#### ShortValue

Alignment: 2 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t[2]`    | Signed 2-byte integer value. |

|

#### IntegerValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t[4]`    | Signed 4-byte integer value. |

|

#### LongValue

Alignment: 8 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t[8]`    | Signed 8-byte integer value. |

|

#### FloatValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t[4]`    | 4-byte bit pattern, zero-extended to the right, and interpreted as an IEEE754 32-bit floating point value. |

|

#### DoubleValue

Alignment: 8 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t[8]`    | 8-byte bit pattern, zero-extended to the right, and interpreted as an IEEE754 64-bit floating point value. |

|

#### StringValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint32_t`      | The value represents an offset to [String](#string). |

|

#### EnumValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint32_t`      | The value represents an offset to an enum's field. The offset must point to [Field](#field) or [ForeignField](#foreignfield). |

|

#### ClassValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint32_t`      | The value represents an offset to [Class](#class) or [ForeignClass](#foreignclass). |

|

#### AnnotationValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint32_t`      | The value represents an offset to [Annotation](#annotation). |

|

#### MethodValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint32_t`      | The value represents an offset to [Method](#method) or [ForeignMethod](#foreignmethod). |

|

#### MethodHandleValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint32_t`      | The value represents an offset to [MethodHandle](#methodhandle). |

|

#### MethodTypeValue

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint32_t`      | The value represents an offset to [Proto](#proto). |

|

#### ArrayValue

Alignment: None

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `count`            | `uleb128`       | Number of elements in the array. |
| `elements`         | `Value[]`       | Unaligned array of [Value](#value) items. The array has `count` elements. |

|

### Literal

Different literal encodings depend on the num of a value's bytes.

|

#### ByteOne

Alignment: None

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t`       | 1-byte value. |

|

#### ByteTwo

Alignment: 2 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t[2]`    | 2-byte value. |

|

#### ByteFour

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t[4]`    | 4-byte value. |

|

#### ByteEight

Alignment: 8 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `value`            | `uint8_t[8]`    | 8-byte value. |

|

### ClassIndex

`ClassIndex` structure is aimed to allow runtime to find a type definition
by name quickly.
The structure is organized as an array of offsets from the beginning of the
file to the [Class](#class) or [ForeignClass](#foreignclass) structures.
All offsets are sorted by the corresponding class names.
The number of elements in the index is `num_classes` from [Header](#header).

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `offsets`          | `uint32_t[]`    | Sorted array of offsets to [Class](#class) or [ForeignClass](#foreignclass) structures. The array must be sorted by class names. |

|

### LineNumberProgramIndex

`LineNumberProgramIndex` structure is aimed to allow using more compact
references to [Line Number Program](#line-number-program).
The structure is organized as an array of offsets from the beginning of the
file to the [Line Number Program](#line-number-program) structures.
The number of elements in the index is `num_lnps` from [Header](#header).

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `offsets`          | `uint32_t[]`    | Array of offsets to [Line Number Program](#line-number-program) structures. |

|

### ClassRegionIndex

`ClassRegionIndex` structure is aimed to allow runtime to find a type definition
by index. The structure is organized as an array of [FieldType](#fieldtype).
The number of elements in the index is `class_idx_size` from [RegionHeader](#regionheader).

Alignment: 4 bytes

Format:

| Name               | Format             | Description |
| ----               | ------             | ----------- |
| `types`            | `FieldType[]`      | Array of [FieldType](#fieldtype) structures. |

|

### MethodRegionIndex

`MethodRegionIndex` structure is aimed to allow runtime to find a method definition by index.
The structure is organized as an array of offsets from the beginning of the file
to the [Method](#method) or the [ForeignMethod](#foreignmethod) structure.
The number of elements in the index is `method_idx_size` from [RegionHeader](#regionheader).

Alignment: 4 bytes

Format:

| Name               | Format             | Description |
| ----               | ------             | ----------- |
| `offsets`          | `uint32_t[]`       | Array of offsets to [Method](#method) or [ForeignMethod](#foreignmethod) structures. |

|

### FieldRegionIndex

`FieldRegionIndex` structure is aimed to allow runtime to find a field
definition by index. The structure is organized as an array of offsets from
the beginning of the file to the [Field](#field) or the
[ForeignField](#foreignfield) structure. The number of elements in the index
is `field_idx_size` from [RegionHeader](#regionheader).

Alignment: 4 bytes

Format:

| Name               | Format             | Description |
| ----               | ------             | ----------- |
| `offsets`          | `uint32_t[]`       | Array of offsets to [Field](#field) or [ForeignField](#foreignfield) structures. |

|

### ProtoRegionIndex

`ProtoRegionIndex` structure is aimed to allow runtime to find a proto
definition by index. The structure is organized as an array of offsets from
the beginning of the file to the [Proto](#proto) structure.
The number of elements in the index is `proto_idx_size` from
[RegionHeader](#regionheader).

Alignment: 4 bytes

Format:

| Name               | Format             | Description |
| ----               | ------             | ----------- |
| `offsets`          | `uint32_t[]`       | Array of offsets to [Proto](#proto) structures. |

|

### LiteralArrayIndex

`LiteralArrayIndex` structure is aimed to allow runtime to find a LiteralArray
definition by index. The structure is organized as an array of offsets from the
beginning of the file to the [LiteralArray](#literalarray) structures. The
number of elements in the index is `num_literalarrays` from [Header](#header).

Alignment: 4 bytes

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `offsets`          | `uint32_t[]`    | Sorted array of offsets to [LiteralArray](#literalarray) structures. |

|

### InstDebugInfo

Debug information contains mapping between program counter of a method, line
numbers in source code, and information about local variables. The format is
derived from DWARF Debugging Information Format, Version 3 (see item 6.2).
The mapping and local variable information are encoded in
[line number program](#line-number-program), which is interpreted by
[the state machine](#state-machine). All constants the program refers to are
moved into [the constant pool](#constant-pool) in order to deduplicate the same
line number programs of different methods.

Alignment: none

Format:

| Name                      | Format          | Description |
| ----                      | ------          | ----------- |
| `line_start`              | `uleb128`       | The initial value of line register of [the state machine](#state-machine). |
| `num_parameters`          | `uleb128`       | Number of method parameters. |
| `parameters`              | `uleb128[]`     | Parameters names of the method. The array has `num_parameters` elements. Each element is an offset to [String](#string) or 0 if there is no name. |
| `constant_pool_size`      | `uleb128`       | Size of constant pool in bytes. |
| `constant_pool`           | `uleb128[]`     | [Constant pool](#constant-pool) data of length `constant_pool_size` bytes. |
| `line_number_program_idx` | `uleb128`       | [Line number program](#line-number-program) index in a [`LineNumberProgramIndex`](#linenumberprogramindex) structure. The program has variable length and ends with `DBG_END_SEQUENCE` opcode. |

|

#### Constant pool

Many methods have similar line number programs. The difference is only in
variable names, variable types, and file names. In order to deduplicate such
programs, all constants the program refers to are stored in the constant pool.
During program interpretation, [the state machine](#state-machine) tracks a
pointer to the constant pool. When [the state machine](#state-machine) interprets
an instruction that requires a constant argument, the machine reads memory for
the value that the constant pool pointer points to, and then increments the
pointer. Thus, the program has no explicit reference to constants, and can be
deduplicated.

|

#### State Machine

The aim of the state machine is to generate mapping between the program counter,
line numbers, and local variable information. The machine has the following
registers:

| Name                 | Initial value | Description       |
| ----                 | ------------- | ----------------- |
| `address`            | 0             | Program counter (refers to method's instructions). Must monotonically increase only. |
| `line`               | `line_start` from [InstDebugInfo](#debug_info). | Unsigned integer which corresponds to a line number in source code. All lines are numbered beginning at 1, and the register must not have a value less then 1. |
| `file`               | Value of `SOURCE_FILE` tag in `class_data` (see [Class](#class)) or 0 | Offset to the name of source file. If there is no such information (`SOURCE_FILE` tag is absent in [Class](#class)), then the register value is 0. |
| `prologue_end`       | `false`       | The register indicates the current address that is the one where entry breakpoint of the method can be set. |
| `epilogue_begin`     | `false`       | The register indicates the current address that is the one where exit breakpoint of the method can be set. |
| `constant_pool_ptr`  | Address of the `constant_pool`'s first byte from [InstDebugInfo](#debug_info). | Pointer to the current constant value. |

|

#### Line Number Program

A line number program consists of instructions. Each instruction has one byte
opcode and optional arguments. Depending on opcode, an argument's value can be
encoded into the instruction. Otherwise, the instruction requires the value
to be read from the constant pool.

| Opcode                 | Value        | Instruction Format | Constant pool arguments   | Description                           |
| ------                 | -----        | ------------------ | -----------------------   | -----------                           |
| `END_SEQUENCE`         | `0x00`       |                    |                           | Marks the end of line number program. |
| `ADVANCE_PC`           | `0x01`       |                    | `uleb128`                 | Increments `address` register by the value that `constant_pool_ptr` refers to without emitting a line. |
| `ADVANCE_LINE`         | `0x02`       |                    | `sleb128`                 | Increments `line` register by the value that `constant_pool_ptr` refers to without emitting a line. |
| `START_LOCAL`          | `0x03`       | `sleb128`          | `uleb128 uleb128`         | Introduces a local variable with the name and type that `constant_pool_ptr` refers to at the current address. The number of the register that contains the variable is encoded in the instruction. Register value `-1` signifies the accumulator register. The name is an offset to [String](#String), and the type is an offset to [ForeignClass](#foreignclass) or [Class](#class). If offset is `0`, then the corresponding information is missing. |
| `START_LOCAL_EXTENDED` | `0x04`       | `sleb128`          | `uleb128 uleb128 uleb128` | Introduces a local variable with the name, type, and type signature that `constant_pool_ptr` refers to at the current address. The number of the register that contains the variable is encoded in the instruction. Register value `-1` signifies the accumulator register. The name is an offset to [String](#String), the type is an offset to [ForeignClass](#foreignclass) or [Class](#class), and the signature is an offset to TODO: figure out what are signatures.If offset is `0`, then the corresponding information is missing. |
| `END_LOCAL`            | `0x05`       | `sleb128`          |                           | Marks that local variable in the specified register is out of scope. The register number is encoded in the instruction. Register value `-1` signifies the accumulator register. |
| `RESTART_LOCAL`        | `0x06`       | `sleb128`          |                           | Re-introduces a local variable at the specified register. The name and type are the same as the last local in the register. The number of the register is encoded in the instruction. Register value `-1` signifies the accumulator register. |
| `SET_PROLOGUE_END`     | `0x07`       |                    |                           | Sets `prologue_end` register to `true`. Any special opcodes clear `prologue_end` register. |
| `SET_EPILOGUE_BEGIN`   | `0x08`       |                    |                           | Sets `epilogue_end` register to `true`. Any special opcodes clear `epilogue_end` register. |
| `SET_FILE`             | `0x09`       |                    | `uleb128`                 | Sets `file` register to the value `constant_pool_ptr` refers to. The argument is an offset to [String](#string) which represents the file name or `0`. |
| `SET_SOURCE_CODE`      | `0x0a`       |                    | `uleb128`                 | Sets `source_code` register to the value `constant_pool_ptr` refers to. The argument is an offset to [String](#string) which represents the source code or `0`. |
| `SET_COLUMN` | `0x0b` | | `uleb128` | Sets `column` register by the value that `constant_pool_ptr` refers to |
| Special opcodes        | `0x0c..0xff` |                    |                           | |

Special opcodes:

[The state machine](#state-machine) interprets each special opcode as follows
(see DWARF Debugging Information Format item 6.2.5.1 Special Opcodes):

1. Calculate the adjusted opcode: `adjusted_opcode = opcode - OPCODE_BASE`.
2. Increment `address` register: `address += adjusted_opcode / LINE_RANGE`.
3. Increment `line` register: `line += LINE_BASE + (adjusted_opcode % LINE_RANGE)`.
4. Emit line number.
5. Set `prologue_end` register to `false`.
6. Set `epilogue_begin` register to `false`.

Where:

* `OPCODE_BASE = 0x0c`: the first special opcode.
* `LINE_BASE = -4`: the smallest line number increment.
* `LINE_RANGE = 15`: the number of line increments presented.

|

### MethodHandle

Alignment: none

Format:

| Name               | Format          | Description |
| ----               | ------          | ----------- |
| `type`             | `uint8_t`       | Type of the handle. Must be one of [MethodHandle's type](#types_of_methodhandle). |
| `offset`           | `uleb128`       | Offset to the entity of the corresponding type. Type of the entity is determined depending on the handle's type (see [Types of MethodHandle](#types_of_methodhandle)). |

|

#### Types of MethodHandle

The types available for a method handle are as follows:

| Name                 | Code              | Description |
| ----                 | :--:              | ----------- |
| `PUT_STATIC`         | `0x00`            | Method handle refers to a static setter. Offset in [MethodHandle](#methodhandle) must point to [Field](#field) or [ForeignField](#foreignfield). |
| `GET_STATIC`         | `0x01`            | Method handle refers to a static getter. Offset in [MethodHandle](#methodhandle) must point to [Field](#field) or [ForeignField](#foreignfield). |
| `PUT_INSTANCE`       | `0x02`            | Method handle refers to an instance getter. Offset in [MethodHandle](#methodhandle) must point to [Field](#field) or [ForeignField](#foreignfield). |
| `GET_INSTANCE`       | `0x03`            | Method handle refers to an instance setter. Offset in [MethodHandle](#methodhandle) must point to [Field](#field) or [ForeignField](#foreignfield). |
| `INVOKE_STATIC`      | `0x04`            | Method handle refers to a static method. Offset in [MethodHandle](#methodhandle) must point to [Method](#method) or [ForeignMethod](#foreignmethod). |
| `INVOKE_INSTANCE`    | `0x05`            | Method handle refers to an instance method. Offset in [MethodHandle](#methodhandle) must point to [Method](#method) or [ForeignMethod](#foreignmethod). |
| `INVOKE_CONSTRUCTOR` | `0x06`            | Method handle refers to a constructor. Offset in [MethodHandle](#methodhandle) must point to [Method](#method) or [ForeignMethod](#foreignmethod). |
| `INVOKE_DIRECT`      | `0x07`            | Method handle refers to a direct method. Offset in [MethodHandle](#methodhandle) must point to [Method](#method) or [ForeignMethod](#foreignmethod). |
| `INVOKE_INTERFACE`   | `0x08`            | Method handle refers to an interface method. Offset in [MethodHandle](#methodhandle) must point to [Method](#method) or [ForeignMethod](#foreignmethod). |

|

#### Argument Types

A bootstrap method can accept static arguments of the following types:

| Type               | Code            | Description |
| -----              | :--:            | ----------- |
| `Integer`          | `0x00`          | The corresponding argument has [IntegerValue](#integervalue) encoding. |
| `Long`             | `0x01`          | The corresponding argument has [LongValue](#longvalue) encoding. |
| `Float`            | `0x02`          | The corresponding argument has [FloatValue](#floatvalue) encoding. |
| `Double`           | `0x03`          | The corresponding argument has [DoubleValue](#doublevalue) encoding. |
| `String`           | `0x04`          | The corresponding argument has [StringValue](#stringvalue) encoding. |
| `Class`            | `0x05`          | The corresponding argument has [ClassValue](#classvalue) encoding. |
| `MethodHandle`     | `0x06`          | The corresponding argument has [MethodHandleValue](#methodhandlevalue) encoding. |
| `MethodType`       | `0x07`          | The corresponding argument has [MethodTypeValue](#methodtypevalue) encoding. |
