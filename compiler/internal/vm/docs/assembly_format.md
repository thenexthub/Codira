# Assembly File Format Specification

## Introduction

This document describes assembly file format for Panda platform. Assembly files are human-readable and human-writeable plain text files, they are supposed to be fed to the Panda assembler, a dedicated tool that translates them to binary files that can be executed by the Panda virtual machine. Please note that this document does not describe bytecode instructions supported by the Panda virtual machine, refer to the [Bytecode ISA Specification](isa/isa.yaml) instead. This document does not specify the binary format of executables supported by the Panda virtual machine, please refer to the [Binary Format Specification](file_format.md) instead.

### Requirements

Panda as a platform is multilingual and flexible by design:

* Panda assembly should not "favor" by any means any existing programming language that is (or intended to be) supported by the platform. Instead, Panda assembly can be thought as a separate close-to-byte-code language with a minimal feature set. All language-specific "traits" that should be supported to generate valid executable binaries with respect to the higher-level semantics should be implemented via metadata annotations (see below).
* Panda assembly should not focus on a certain programming paradigm. E.g. we should not enforce concepts of "class", "object", "method" at the assembly language level because we might support a language which does not implement classic OOP at all.
* When Panda assembler generates a binary executable file, it is not expected to check for language semantics. This responsibility is delegate to "source to binaries" compilers and runtime.
* Panda assembler should not impose any limitation of quantity and internal structure of source code files writtebn in Panda assembly language. Assembler should process as many input source code files as the developer specifies.
* Panda assembler should not follow any implicit conventions about the name of the entry point.

## Comments

Comments are marked with the `#` character. All characters that follow it (including the `#` character itself) are ignored.

## Literals

### Numeric Literals

Following numeric literals are supported:

* Signed/Unsigned decimal/hexadecimal/binary integers not larger than 64 bits. Hexadecimal literals are prefixed with `0x`. Binary literals are prefixed with `0b`.
* Floating-point decimal/hexadecimal literals that can be represented with IEEE 754. Hexadecimal floating-point literals are prefixed with `0x`. They are first converted to a bit representation that corresponds to a hex, and then converted to a double using a bit_cast in accordance with the IEEE 754 standard.

### String Literals

String literal is a sequence of any characters enclosed in `"` characters. Non-printable characters and characters out of Latin-1 character set must be encoded with `mutf8` encoding. For example: `"文字范例"` string literal should be encoded as `"\xe6\x96\x87\xe5\xad\x97\xe8\x8c\x83\xe4\xbe\x8b"`

The following escape sequences can be used in string literals:

    - `\"` double quote, `\x22`
    - `\a` alert, `\x07`
    - `\b` backspace, `\x08`
    - `\f` form feed, `\x0c`
    - `\n` newline, `\x0a`
    - `\r` carriage return, `\x0d`
    - `\t` horizontal tab, `\x09`

## Identifiers

### Simple Identifiers

A simple identifier is a sequence of ASCII characters. Allowed characters in the sequence are:

* Letters from `a` to `z`.
* Letters from `A` to `Z`.
* Digits from `0` to `9`.
* Following characters: `_`, `$`.

Following constraints apply:

* A valid identifier starts with any letter or with `_`.
* All identifiers are case sensitive.

Simple identifiers can be used for naming metadata annotations, primitive data types, aggregate data types, members of aggregate data types, functions and labels.

### Prefixed Identifiers

A prefixed identifier is a sequence of simple identifiers delimited by the `.` char without whitespaces.

Prefixed identifiers can be used for naming metadata annotations, aggregate data types and functions.

## Metadata Annotations

As stated above, current version of Panda assembly does not favor any language as the platform is designed to support many of them. To deal with language-specific metadata, annotations are used, defined as follows:

```
<key1=value1, key2=value2, ...>
```

Values are optional. In such case, only `key` is needed.

Following constraints apply:

* Each key is a valid indetifier.
* All keys are unique within a single annotation list.
* If present, a value is a valid identifier, with following exception: Values can start with a digit.

In all cases where annotations can be optionally used, `optional_annotation` marker is used in this document.

There are keys that indicate that a function must not have an implementation. The absence of these keys suggests otherwise. We shall call metadata containing such keys --- `lonely metadata`.

### Function metadata annotations

#### Standard metadata

A definition of a function is assumed.

| Key | Description |
| ------ | ------ |

#### Lonely metadata

A declaration of a function is assumed.

| Key | Description |
| ------ | ------ |
| `external` | Marks an externally defined function. Does not require value. |
| `native` | Marks an externally defined function. Does not require value. |
| `noimpl` | Marks a function without implementation. Does not require value. |
| `static` | Marks a function as static. Does not require value. |
| `ctor`   | Marks a function as object constructor. It will be renamed in binary file according to particular language rules (`.ctor` for Panda Assembly) |
| `cctor`  | Marks a function as static constructor. It will be renamed in binary file according to particular language rules (`.cctor` for Panda Assembly) |

### Record metadata annotations

#### Standard metadata

A definition of a record is assumed.

| Key | Description |
| ------ | ------ |

#### Lonely metadata

A declaration of a record is assumed.

| Key | Description |
| ------ | ------ |
| `external` | Marks an externally defined record. Does not require value. |

### Field metadata annotations

| Key | Description |
| ------ | ------ |
| `external` | Marks an externally defined field. Does not require value. |
| `static` | Marks an statically defined field. Does not require value. |

### Language specific annotations

Currently Panda Assembly supports annotations for the following languages:

- Java
- PandaAssembly

To specify language `.language` directive is used. It must be declared before any other declarations:
```
.language Java

.function void f() {}
```
By default PandaAssembly language is assumed.

#### Java annotations

Currently Panda Assembly supports following Java annotations

| Key | Description |
| --- | --- |
| `java.access` | Used to specify access level of record, field or function. Possible values: `private`, `protected`, `public`. |
| `java.extends` | Used to specify inheritance between records. Value is the name of the base record. |
| `java.implements` | Used to specify interface inheritance between records. Value is the name of the interface record. Allowed multiple definition. |
| `java.interface` | Used to specify that the record represents Java interface. |
| `java.enum` | Used to specify that the record and its fields represent Java enum. |
| `java.annotation` | Used to specify that the record represents Java annotation. |
| `java.annotation.type` | Used to specify type of annotation. Possible values: `class`, `runtime`. |
| `java.annotation.class` | Used to specify annotation class. Allowed multiple definitions. Value is the name of the record that represent Java annotation |
| `java.annotation.id` | Used to specify annotation id. Annotations with id are used as values of other annotation elements. `java.annotation.class` must be defined first. Allowed multiple definitions (but only one definition for each annotation). |
| `java.annotation.element.name` | Used to specify name of the annotation element. `java.annotation.class` must be defined first. Allowed multiple definitions (but only one definition for each annotation element). |
| `java.annotation.element.type` | Used to specify type of the annotation element. `java.annotation.element.name` must be defined first. Allowed multiple definitions (but only one definition for each annotation element). Possible values: `u1`, `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `f32`, `f64`, `string`, `class`, `enum`, `annotation`, `array`. |
| `java.annotation.element.array.component.type` | Used to specify component type of the array annotation element. `java.annotation.element.type` must be defined first and have `array` value. Allowed multiple definitions (but only one definition for each annotation element). Possible values: `u1`, `i8`, `u8`, `i16`, `u16`, `i32`, `u32`, `i64`, `u64`, `f32`, `f64`, `string`, `class`, `enum`, `annotation`. |
| `java.annotation.element.value` | Used to specify value of the annotation element. Allowed multiple definitions (also multiple definitions for one annotation element if it has `array` type). |


Example:

```
.language Java

.record A <java.access=public> {}
.record B <java.access=public, java.extends=A> {}

.record Iface1 <java.interface>
.record Iface2 <java.interface>

.record C <java.implements=Iface1, java.implements=Iface2> {}

.record A1 <java.annotation, java.annotation.type=runtime> {}
.record A2 <java.annotation, java.annotation.type=runtime> {}

# Annotation elements are represented using abstract methods

.function i32[] A1.NameArr() <noimpl>
.function A1 A2.Name() <noimpl>

# @A2(Name=@A1(NameArr={1,2}))
.record R <java.annotation.class=A1, java.annotation.id=id1, java.annotation.element.name=NameArr, java.annotation.element.type=array, java.annotation.element.array.component.type=i32, java.annotation.element.value=1, java.annotation.element.value=2, java.annotation.class=A2, java.annotation.element.name=Name, java.annotation.element.type=annotation, java.annotation.element.value=id1>
```

## Data Types

Semantics of operations on all data types defined below follows the semantics defined in [Bytecode ISA Specification](isa/isa.yaml).

### Primitive Data Types

Following primitive types are supported:

| Panda Assembler Type | Description |
| ------ | ------ |
| `void` | Type for the result of a function that returns normally, but does not provide a result value to its caller |
| `u1` | Unsinged 1-bit integer number |
| `u8` | Unsigned 8-bit integer number |
| `i8` | Signed 8-bit integer number |
| `u16` | Unsigned 16-bit integer number |
| `i16` | Signed 16-bit integer number |
| `u32` | Unsigned 32-bit integer number |
| `i32` | Signed 32-bit integer number |
| `u64` | Unsigned 64-bit integer number |
| `i64` | Signed 64-bit integer number |
| `f32` | 32-bit single precision floating point number, compliant with IEEE 754 standard |
| `f64` | 64-bit double precision floating point number, compliant with IEEE 754 standard |

All identifiers that are used for naming primitive data types cannot be used for any other purpose.

### Reference Data Types

Following reference types are supported:

| Panda Assembler Type | Description |
| ------ | ------ |
| `cref` | code reference, represents references to the bytecode executable by Panda virtual machine |
| `dref` | data reference, represents references to aggregate data types (see below) |

All identifiers that are used for naming reference data types cannot be used for any other purpose.

### Aggregate Data Types

Aggregate data types are defined as follows:

```
.record RecordName optional_annotation {
    type1 member1 optional_annotation1
    type2 member2 optional_annotation2
    # ...
    typeN memberN optional_annotationN
}
```

Following constraints apply:

* `RecordName`, `type1`, ... `typeN`, `member1`, ... `memberN` are valid identifiers.
* `member1`, ... `memberN` are unique identifiers within a record.
* `RecordName` is unique across all source code files.

Whenever a record should incorporate another record, the name of the nested record must be specified. However, in this context this name implicitly denotes a `dref` type which implements a reference to the data represented by that record. Example:

```
.record Foo {
    i32 member1
    f32 member2
}

.record Bar {
    Foo foo
    f64 member1
    f64 member2
}
```

#### Informal Notice

`.record`s are like `struct`s in C, but without support for "by instance" nesting. This is because the result of a field load should be valid for any member, hence a record member should fit the virtusal register. Constraints on register are defined in [Bytecode ISA Specification](isa/isa.yaml).

### Builtin Aggregate Data Types

Platform has following builtin aggregate types

| Panda Assembler Type | Description |
| ------ | ------ |
| `panda.String` | UTF16 string |

### Arrays

Platform support arrays of primitive and aggregate data types. Array of type `T` has type name `T[]`. Example:
```
.function void f() {
    ...
    newarr v1, v0, i32[]
    ...
    newarr v1, v0, panda.String[]
    ...
    newarr v1, v0, f32[][][]
    ...
}
```

## Functions

Functions are defined as follows:

```
.function FunctionName(ArgumentType0 a0, ... ArgumentTypeN argN) optional_annotation
{
    # code
}
```

Following constraints apply:

* `FunctionName`, `ArgumentType0`, ... `ArgumentTypeN`, `a0`, ... `aN` are valid identifiers.
* All `a0`, ... `aN` are unique within the argument list of the function.
* `FunctionName` is unique across all source code files.

### Function Arguments and Local Variables

By convention, all arguments are named `a0`, ... `aN` and all local variables are named `v0`, ... `vM`. Panda assembler guarantees that all these entities are unambiguously mapped to the underlying virtual registers.

### Function Body

If a function has a body, it consists of optionally labeled sequence of bytecode instructions, one instruction defined per line. Instruction opcodes and formats follow [Bytecode ISA Specification](isa/isa.yaml).

### Static and virtual functions

By default all function are static except ones that are binded to record and accept reference to it as the first parameter:

```
.record R {}

.function void R.foo(R a0) {} # virtual function

.function void R.foo(R a0) <static> {} # static function

.function void R.foo(i32 a0) {} # static function
```

#### Call instructions

Assembler relaxes constraints for call instructions:

- If number of arguments is less than specified in [Bytecode ISA Specification](isa/isa.yaml) it passes `v0` instead of unspecified ones.

- For non range call instructions assembler chooses optimal encoding according to number of specified arguments.

Example:

Following instruction in assembly
```
call.static f, v1
```
will be emitted as
```
call.short.static f, v1, v0
```

### Program Entry Point

Any function which accepts an array of strings as its single argument may serve as a program entry point. The name of the entry point must be specified as a part of the input to the assembler program. An example of a possible entry point is:

```
.record _panda_array_string <external>

.function foo(_panda_array_string a0)
{
    # code
}
```

### Exception handlers

Try, catch and finally blocks can be declared using `.catch` and `.catchall` directives:
```
.catch <exception_record>, <try_begin_label>, <try_end_label>, <catch_begin_label>
.catchall <try_begin_label>, <try_end_label>, <catch_begin_label>
```

Example:
```
.record Exception1 {}
.record Exception2 {}

.function void foo()
{
    ...
try_begin:
    ...
try_end:
    ...
catch_begin1:
    ...
catch_begin2:
    ...
catchall_begin1:
    ...

    .catch Exception1, try_begin, try_end, catch_begin1
    .catch Exception2, try_begin, try_end, catch_begin2
    .catchall try_begin, try_end, catchall_begin1
}
```

Also there are more safer directives, which allow to specify exact bounds
of an exceptions handler for more precise verification of control-flow in
byte-code verifier.

```
.catch <exception_record>, <try_begin_label>, <try_end_label>, <catch_begin_label>, <catch_end_label>
.catchall <try_begin_label>, <try_end_label>, <catch_begin_label>, <catch_end_label>
```

They are almost identical to `.catch` and `.catchall` differ only by specifying end label of the
exception handler. End label is the label that immediately follows last instruction of the
exception handler.

## Pseudo-BNF

Instruction flow is omitted for simplicity:

```
# Literals are represented in double-quotes as "literal value".
# Free-form descriptions are represented as "<description here>"
# Empty symbol is represented as E.

defs          := defs def | E
def           := rec_def | func_def

# Identifiers:

letter_lower    := "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
letter_upper    := "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
digit           := "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
char_misc       := "_"

char_non_dig   := letter_lower | letter_upper | char_misc
char_simple    := char_non_dig | digit

id_simple      := char_non_dig id_simple_tail
id_simple_tail := id_simple_tail char_simple | E

id_prefixed    := id_simple | id_simple "." id_prefixed

# Records and types:
rec_def       := ".record" rec_name rec_add
rec_add       := def_pair_rec_meta rec_body | def_lonely_rec_meta
rec_name      := id_prefixed
rec_body      := "{" fields "}"
type_def      := "u1" | "u8" | "i8" | "u16" | "i16" | "u32" | "i32" | "i64" | "f32" | "f64" | "any" | rec_name | type_def []

# Fields of records:
fields        := fields field_def | E
field_def     := field_type field_name def_field_meta
field_type    := type_def
field_name    := id_simple

# Functions:
func_def      := ".function" func_sig func_add
func_add      := def_pair_func_meta func_body | def_lonely_func_meta
func_sig      := func_ret func_name func_args
func_ret      := type_def
func_name     := id_prefixed
func_args     := "(" arg_list ")"
arg_list      := <","-separated list of argument names and their respective types>
func_body     := "{" func_code "}"
func_code     := <newline-separated sequence of bytecode instructions and their operands>

# Function metadata annotations:
def_pair_func_meta    := "<" func_meta_list ">" | E
def_lonely_func_meta  := "<" func_lonely_meta_list ">"
func_meta_list        := func_meta_list func_meta_item "," | E
func_meta_item        := func_kv_pair | func_id
func_kv_pair          := <an element of the function standard metadata list that assumes the assignment of a value, and the value that is assigned to it, separated by the sign "=">
func_id               := <an element of the function standard metadata list>
func_lonely_meta_list := func_lonely_meta_list func_meta_item "," | E
func_meta_item        := func_kv_lonely_pair | func_lonely_id
func_kv_lonely_pair   := <an element of the function lonely metadata list that assumes the assignment of a value, and the value that is assigned to it, separated by the sign "=">
func_lonely_id        := <an element of the function lonely metadata list>

# Record metadata annotations:
def_pair_rec_meta    := "<" rec_meta_list ">" | E
def_lonely_rec_meta  := "<" rec_lonely_meta_list ">"
rec_meta_list        := rec_meta_list rec_meta_item "," | E
rec_meta_item        := rec_kv_pair | rec_id
rec_kv_pair          := <an element of the record standard metadata list that assumes the assignment of a value, and the value that is assigned to it, separated by the sign "=">
rec_id               := <an element of the record standard metadata list>
rec_lonely_meta_list := rec_lonely_meta_list rec_meta_item "," | E
rec_meta_item        := rec_kv_lonely_pair | rec_lonely_id
rec_kv_lonely_pair   := <an element of the record lonely metadata list that assumes the assignment of a value, and the value that is assigned to it, separated by the sign "=">
rec_lonely_id        := <an element of the record lonely metadata list>

# Field metadata annotations:
def_field_meta       := "<" field_meta_list ">" | E
field_meta_list      := field_meta_list field_meta_item "," | E
field_meta_item      := field_kv_pair | field_id
field_kv_pair        := <an element of the field metadata list that assumes the assignment of a value, and the value that is assigned to it, separated by the sign "=">
field_id             := <an element of the field metadata list>
```

## Important notes

- Assembler doesn't guarantee that functions, records and their fields will be located in binary file in the same order as they are located in assembly one

## Appendix A, Informative: Code Layout Sample

```
# External records and functions:
.record Record1 <external>
.function Record1.function1(Record1 a0, f64 a1) <external>

.record Foo <java.extends=SomeRecord> {
    i32 member1 <java.access=private>
    i32 member2 <java.access=public>
    i32 member3 <java.access=static, java.instantiation=static>
}

.function Foo.constructor1(Foo a0) <java.ctor>
{
    # code for an overloaded "constructor" (whatever you mean by it)
}

.function Foo.constructor2(Foo a0, i32 a1) <java.ctor>
{
    # code for an overloaded "constructor" (whatever you mean by it)
}

.function Foo.func1(Foo a0, i32 a1) <java.access=public>
{
    # code
}

# "Interface" function:
.function Foo.func2(Foo a0, i32 a1) <noimpl>

.function entry_point(_panda_array_string a0)
{
    # After loading the binary, control will be transferred here
}
```

Apart from metadata annotations, `Foo.` prefixes (remaining a pure naming convention for the assembler!) can be additionally processed during linkage to "bind" functions to records making them "true" methods from the OOP world.

**Strings** and **arrays** can be thought as `external` record with some manipulating functions. There is no support for generics due to the low-level nature of the assembler, hence arrays of different types are implemented with different external record.

## Appendix B, Informative: Mapping Panda Assembler TYpes to JVM Types

This section serves purely illustrative purposes.

| Panda Assembler Type | Corresponding JVM Type |
| ------ | ------ |
| `u1` | `bool` |
| `u8` | N/A |
| `i8` | `byte` |
| `u16` | `char` |
| `i16` | `short` |
| `u32` | N/A |
| `i32` | `int` |
| `u64` | N/A |
| `i64` | `long` |
| `f32` | `float` |
| `f64` | `double` |
| `cref` | N/A |
| `dref` | `reference` |

## Appendix C, TODO List

* Specify `cref` and indirect calls to functions.
* Elaborate on bytecode definition.
* Compose formal definitions for literals.
