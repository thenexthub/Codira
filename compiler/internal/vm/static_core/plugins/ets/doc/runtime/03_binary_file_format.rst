..
    Copyright (c) 2021-2024 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

.. _RT Binary File Format:

Binary File Format
******************

The |LANG| binary file format is designed with the following goals in mind:

- Compactness,
- Fast information access,
- Low memory footprint,
- Extensibility, and
- Compatibility.

**Compactness**. Many mobile applications use numerous types, methods,
and fields. The number is very large, and does not fit into a 16-bit
unsigned integer. As a result, an application developer creates several
files, and some data cannot be deduplicated.

Current binary file format must extend these limits to conform to modern
requirements. It is achieved by making all direct references in a binary
file 4 bytes long to allow 4Gb addressing.

16-bit indices are used to refer to classes, methods and fields in
bytecode and metadata for greater compactness. A file can contain multiple
indices, each covering a part of a file as described in :ref:`RegionHeader <RT RegionHeader>`.

A file format uses :ref:`TaggedValue <RT TaggedValue>` to enable compact value storage to
store only the information available and avoid zero offset to any missing
information.

**Fast information access**. The binary file format must support fast
access to information. It means that redundant references are to be
avoided. The binary file format must also avoid data indices (e.g.,
sorted list of strings) where possible. However, the current binary file
format supports one index that is a sorted list of offsets to classes.
This compact index allows finding type definitions quickly as the
binary file is loaded.

The current binary format distinguishes between two types of classes,
fields, and methods:

- *Foreign* entities are referenced in the current binary file but
  defined in other binary files.
- *Local* entities are defined within the current binary file. 

As a local* entity and a corresponding *foreign* entity have the same header,
offsets to *foreign* and *local* entities can be used interchangeably when
a class, a field, or a method is referenced in a binary file.

A class, field or method type can be deduced at runtime by checking
whether its offset belongs to a foreign region
(``[foreign_off; foreign_off + foreign_size)``) or not. The definitions of
*foreign* entities are found in other binary files by using information
from the *foreign* entity header. The definitions of *local* entities are
found in the current binary file by using a *local* entity offset.

**Low memory footprint**. Practice proves that applications make no use
of most file data. Memory footprint of a file can be reduced significantly by
grouping the frequently-used data.
The described binary file format supports this feature by using offsets
and by not specifying the required placement of entities in relation to one
another.

**Extensibility and Compatibility**. The binary file format supports
future changes via version number. The version field in the header is
4-byte long, and is encoded as byte array to avoid misinterpretation on
platforms with different endianness. A tool that supports  version ``'N'``
of the binary file format must also support the format of version ``'N-1'``.

|

.. _RT Alignment:

Alignment
=========

Most entities have the 4-byte alignment in order to improve the speed of
data access. All exceptions are specified explicitly in appropriate descriptions
below.

|

.. _RT Endianness:

Endianness
==========

All multi-byte values are little-endian as most target architectures are.

|

.. _RT Offsets:

Offsets
=======

Unless specified otherwise, all offsets are calculated from the beginning
of the file.

An offset cannot contain values in the range ``[0; sizeof(Header))``,
except in certain cases specifically mentioned.

|

.. _RT Data Types:

Data Types
==========

.. _RT Integer Types:

Integer Types
-------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Description

   * - ``uint8_t``
     - 8-bit unsigned integer value

   * - ``uint16_t``
     - 16-bit unsigned integer value

   * - ``uint32_t``
     - 32-bit little-endian unsigned integer value

   * - ``uleb128``
     - Unsigned integer value in leb128 encoding

   * - ``sleb128``
     - Signed integer value in leb128 encoding

.. _RT String:

String Format
-------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``utf16_length``
     - ``uleb128``
     -  ``len << 1 | is_ascii`` where ``len`` is the length of the string in 
        UTF-16 code units

   * - ``data``
     - ``uint8_t[]``
     -  ``\0`` terminated character sequence in MUTF-8 (Modified UTF-8)
        encoding

.. _RT TaggedValue:

TaggedValue Format
------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``tag_value``
     - ``uint8_t``
     -  Tag that determines the encoding of ``data``.

   * - ``data``
     - ``uint8_t[]``
     -  Optional payload

.. _RT Source Language:

Source Language
---------------

Source language can be specified for classes and methods. Default language for
methods is that of the class. A file can be emitted from the sources written
in the following languages:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Source
     - Encoding
   * - ``Ark Assembly``
     - ``0x01``

Values ``0x00``, ``0x02`` - ``0x0f`` are reserved.

|

.. _RT File Layout:

File Layout
===========

A binary file begins with the :ref:`Header <RT Header>` located at offset ``0``.
Any other data can be reached from the header.

.. _RT Header:

Header Format
-------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 25 15 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``magic``
     - ``uint8_t[8]``
     - Magic string - 'PANDA\0\0\0'.

   * - ``checksum``
     - ``uint8_t[4]``
     - ``adler32`` checksum of the file except magic and checksum fields.

   * - ``version``
     - ``uint8_t[4]``
     - Version of file format.

   * - ``file_size``
     - ``uint32_t``
     - Size of the file in bytes.

   * - ``foreign_off``
     - ``uint32_t``
     - Offset to a foreign region. The region must contain
       :ref:`ForeignField <RT ForeignField>`, :ref:`ForeignMethod <RT ForeignMethod>`, or :ref:`ForeignClass <RT ForeignClass>` only.
       ``foreign_off`` does not necessarily point at the first *foreign*
       entity. ``foreign_off`` and ``foreign_size`` must be used at runtime to 
       determine whether a given offset is *foreign* or *local*.

   * - ``foreign_size``
     - ``uint32_t``
     - Size of the foreign region in bytes.

   * - ``num_classes``
     - ``uint32_t``
     - Number of classes defined in the file, and of elements in the entity
       pointed at by ``class_idx_off``.

   * - ``class_idx_off``
     - ``uint32_t``
     - Offset to the class index. The offset must point at an entity
       in the :ref:`ClassIndex <RT ClassIndex>` format.

   * - ``num_export_table``
     - ``uint32_t``
     - Number of exported classes defined in the file, and of elements in
       the entity pointed at by ``export_table_off``.

   * - ``export_table_off``
     - ``uint32_t``
     - Offset to the exported class index. The offset must point at
       an entity in the :ref:`ClassIndex <RT ClassIndex>` format.

   * - ``num_lnps``
     - ``uint32_t``
     - Number of line number programs in the file, and of elements in
       the entity pointed at by ``lnp_idx_off``.

   * - ``lnp_idx_off``
     - ``uint32_t``
     - Offset to the line number program index. The offset must point
       to an entity in the :ref:`LineNumberProgramIndex <RT LineNumberProgramIndex>` format.

   * - ``num_literalarrays``
     - ``uint32_t``
     - Number of literalArrays defined in the file, and of elements in
       the entity pointed at by ``literalarray_idx_off``.

   * - ``literalarray_idx_off``
     - ``uint32_t``
     - Offset to the literal array index. The offset must point at an
       entity in the :ref:`LiteralArrayIndex <RT LiteralArrayIndex>` format.

   * - ``num_index_regions``
     - ``uint32_t``
     - Number of the index regions in the file, and of elements in
       the entity pointed at by ``index_section_off``.

   * - ``index_section_off``
     - ``uint32_t``
     - Offset to the index section. The offset must point at an entity in
       the :ref:`RegionIndex <RT RegionIndex>` format.

**Constraint**: size of header must be greater than 16 bytes. :ref:`FieldType <RT FieldType>`
uses this fact.

.. _RT ClassIndex:

ClassIndex Format
-----------------

``ClassIndex`` entities are designed to allow to quickly find a type definition
by name at runtime. It is organized as an array of offsets to a
:ref:`Class <RT Class>` or to a :ref:`ForeignClass <RT ForeignClass>`.
All offsets are sorted by the corresponding class names.

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``offsets``
     - ``uint32_t[]``
     - Sorted array of offsets to a :ref:`Class <RT Class>` or to a
       :ref:`ForeignClass <RT ForeignClass>`.

.. _RT LineNumberProgramIndex:

LineNumberProgramIndex Format
-----------------------------

``LineNumberProgramIndex`` entities are designed to allow using more compact
references to :ref:`Line Number Program <RT Line Number Program>`. It is organized
as an array of offsets to a :ref:`Line Number Program <RT Line Number Program>`

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``offsets``
     - ``uint32_t[]``
     - Array of offsets to a :ref:`Line Number Program <RT Line Number Program>`.

.. _RT LiteralArrayIndex:

LiteralArrayIndex Format
------------------------

``LiteralArrayIndex`` entities are designed to allow to find a
:ref:`LiteralArray <RT LiteralArray>` definition by index at runtime.
It is organized as an array of offsets to a :ref:`LiteralArray <RT LiteralArray>`.

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``offsets``
     - ``uint32_t[]``
     - Sorted array of offsets to a :ref:`LiteralArray <RT LiteralArray>`.

.. _RT RegionIndex:

RegionIndex Format
------------------

``RegionIndex`` entities are designed to allow finding the index
that covers a specified offset in the file at runtime. It is organized as
sorted array of a :ref:`RegionHeader <RT RegionHeader>`.

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 20 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``region_headers``
     - ``RegionHeader[]``
     - File regions sorted by the start offset of the region. Each element
       must have the :ref:`RegionHeader <RT RegionHeader>` format.

**Constraint**: Two different regions must not overlap with each other.

.. _RT RegionHeader:

RegionHeader Format
-------------------

To address file entities by using 16-bit indices, a binary file is split
into regions.The information on each region is stored in the ``RegionHeader`` format.

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``start_off``
     - ``uint32_t``
     - Start offset of the region.

   * - ``end_off``
     - ``uint32_t``
     - End offset of the region.

   * - ``class_idx_size``
     - ``uint32_t``
     - Number of elements in an index pointed at by ``class_idx_off``.
       Max value is 65536.

   * - ``class_idx_off``
     - ``uint32_t``
     - Offset to a class index. The offset must point at a
       :ref:`ClassRegionIndex <RT ClassRegionIndex>`.

   * - ``method_idx_size``
     - ``uint32_t``
     - Number of elements in an index pointed at by ``method_idx_off``.
       Max value is 65536.

   * - ``method_idx_off``
     - ``uint32_t``
     - Offset to a method index. The offset must point at a
       :ref:`MethodRegionIndex <RT MethodRegionIndex>`.

   * - ``field_idx_size``
     - ``uint32_t``
     - Number of elements in an index pointed at by ``field_idx_off``.
       Max value is 65536.

   * - ``field_idx_off``
     - ``uint32_t``
     - Offset to a field index. The offset must point at a
       :ref:`FieldRegionIndex <RT FieldRegionIndex>`.

   * - ``proto_idx_size``
     - ``uint32_t``
     - Number of elements in an index pointed at by ``proto_idx_off``.
       Max value is 65536.

   * - ``proto_idx_off``
     - ``uint32_t``
     - Offset to a proto index. The offset must point at a
       :ref:`ProtoRegionIndex <RT ProtoRegionIndex>`.

.. _RT ClassRegionIndex:

ClassRegionIndex Format
-----------------------

``ClassRegionIndex`` entities are designed to allow finding a type
definition by index at runtime. It is organized as an array of offsets to a
:ref:`Class <RT Class>` or to a :ref:`ForeignClass <RT ForeignClass>`.

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``types``
     - ``uint32_t[]``
     - Offsets to a :ref:`Class <RT Class>` or to a :ref:`ForeignClass <RT ForeignClass>`.

.. _RT MethodRegionIndex:

MethodRegionIndex Format
------------------------

``MethodRegionIndex`` entities are designed to allow finding a method
definition by index at runtime. It is organized as an array of offsets
to a :ref:`Method <RT Method>` or to a :ref:`ForeignMethod <RT ForeignMethod>`.

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``methods``
     - ``uint32_t[]``
     - Array of offsets to a :ref:`Method <RT Method>` or to a :ref:`ForeignMethod <RT ForeignMethod>`.

.. _RT FieldRegionIndex:

FieldRegionIndex Format
-----------------------

``FieldRegionIndex`` entities are designed to allow finding a field
definition by index at runtime. It is organized as an array of offsets
to a :ref:`Field <RT Field>` or to a :ref:`ForeignField <RT ForeignField>`.

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``fields``
     - ``uint32_t[]``
     - Array of offsets to a :ref:`Field <RT Field>` or to a :ref:`ForeignField <RT ForeignField>`.

.. _RT ProtoRegionIndex:

ProtoRegionIndex Format
-----------------------

``ProtoRegionIndex`` entities are designed to allow finding a proto
definition by index at runtime.

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``protos``
     - ``uint32_t[]``
     - Array of offsets to a :ref:`Proto <RT Proto>`.

.. _RT ForeignField:

ForeignField Format
-------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 10 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - Index of a declaring class in the corresponding :ref:`ClassRegionIndex <RT ClassRegionIndex>`.
       The corresponding index entry must be an offset to a :ref:`Class <RT Class>` or to a
       :ref:`ForeignClass <RT ForeignClass>`.

   * - ``type_idx``
     - ``uint16_t``
     - Index of the field type in the corresponding :ref:`ClassRegionIndex <RT ClassRegionIndex>`.
       The corresponding index entry must have the :ref:`FieldType <RT FieldType>` format.

   * - ``name_off``
     - ``uint32_t``
     - Offset to the name of the field. The offset must point at a
       :ref:`String <RT String>`.

   * - ``access_flags``
     - ``uleb128``
     - Access flags of a field. The value must be a combination of
       :ref:`Field Access Flags <RT Field Access Flags>`.

.. note::
   A proper region index to resolve ``class_idx`` and ``type_idx`` can be
   found by a foreign field offset.

.. _RT Field:

Field Format
------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 20 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - Index of a declaring class in the corresponding :ref:`ClassRegionIndex <RT ClassRegionIndex>`.
       The corresponding index entry must be an offset to a :ref:`Class <RT Class>` or to a
       :ref:`ForeignClass <RT ForeignClass>`.

   * - ``type_idx``
     - ``uint16_t``
     - Index of the field type in the corresponding :ref:`ClassRegionIndex <RT ClassRegionIndex>`.
       The corresponding index entry must have the :ref:`FieldType <RT FieldType>` format.

   * - ``name_off``
     - ``uint32_t``
     - Offset to the name of the field. The offset must point at a
       :ref:`String <RT String>`.

   * - ``access_flags``
     - ``uleb128``
     - Access flags of the field. The value must be a combination of
       :ref:`Field Access Flags <RT Field Access Flags>`.

   * - ``field_data``
     - ``TaggedValue[]``
     - Variable length list of tagged values. Each element must have the
       :ref:`TaggedValue <RT TaggedValue>` format. A tag must have values from
       :ref:`Field Tags <RT Field Tags>`, and follow the increasing
       tag order (except ``0x00`` tag, which must be the last).

.. note::
   A proper region index to resolve ``class_idx`` and ``type_idx`` can
   be found by a foreign field offset.

.. _RT FieldType:

FieldType
---------

Any offset in the range ``[0; sizeof(Header))`` is invalid because the first
bytes of the file contain the header. The ``FieldType`` encoding uses this fact to
encode primitive types of the field in the low 4 bits. The value of a
non-primitive type is an offset to a :ref:`Class <RT Class>` or to a :ref:`ForeignClass <RT ForeignClass>`.
In either case, :ref:`FieldType <RT FieldType>` is ``uint32_t``. Primitive types are encoded
as follows:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Encoding

   * - ``u1``
     - ``0x00``

   * - ``i8``
     - ``0x01``

   * - ``u8``
     - ``0x02``

   * - ``i16``
     - ``0x03``

   * - ``u16``
     - ``0x04``

   * - ``i32``
     - ``0x05``

   * - ``u32``
     - ``0x06``

   * - ``f32``
     - ``0x07``

   * - ``f64``
     - ``0x08``

   * - ``i64``
     - ``0x09``

   * - ``u64``
     - ``0x0a``

   * - ``any``
     - ``0x0b``

.. _RT Field Access Flags:

Field Access Flags
------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``ACC_PUBLIC``
     - ``0x0001``
     - Declared public.

   * - ``ACC_PRIVATE``
     - ``0x0002``
     - Declared private.

   * - ``ACC_PROTECTED``
     - ``0x0004``
     - Declared protected.

   * - ``ACC_STATIC``
     - ``0x0008``
     - Declared static.

   * - ``ACC_FINAL``
     - ``0x0010``
     - Declared final.

   * - ``ACC_READONLY``
     - ``0x0020``
     - Declared readonly.

   * - ``ACC_VOLATILE``
     - ``0x0040``
     - Declared volatile.

   * - ``ACC_TRANSIENT``
     - ``0x0080``
     - Declared transient.

   * - ``ACC_SYNTHETIC``
     - ``0x1000``
     - Declared synthetic; not present in the source code.

   * - ``ACC_ENUM``
     - ``0x4000``
     - Declared an element of an enum.

**Constraint**: Only the ``ACC_STATIC`` flag is used for a :ref:`ForeignField <RT ForeignField>`.
Other flags are ignored.

.. _RT Field Tags:

Field Tags
----------

.. list-table::
   :width: 100%
   :align: left
   :widths: 30 10 10 15 35
   :header-rows: 1

   * - Name
     - Tag
     - Quantity
     - Data Format
     - Description

   * - ``NOTHING``
     - ``0x00``
     - ``1``
     - ``none``
     - No more values. The value with this tag must be the last.

   * - ``INT_VALUE``
     - ``0x01``
     - ``0-1``
     - ``sleb128``
     - Integral value of a field. This tag is used when a field has type
       ``boolean``, ``byte``, ``char``, ``short`` or ``int``.


   * - ``VALUE``
     - ``0x02``
     - ``0-1``
     - ``uint8_t[4]``
     - Contains a :ref:`Value <RT Value>`. Encoding depends on ``type_idx``.

   * - ``RUNTIME_ANNOTATIONS``
     - ``0x03``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a field annotation **visible** at runtime. If a field has
       several annotations, then the tag can be repeated. The offset must point
       at an :ref:`Annotation <RT Annotation>`.

   * - ``ANNOTATIONS``
     - ``0x04``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a field annotation **invisible** at runtime. If a field
       has several annotations, then the tag can be repeated. The offset must
       point at an :ref:`Annotation <RT Annotation>`.

   * - ``RUNTIME_TYPE_ANNOTATION``
     - ``0x05``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a field type annotation **visible** at runtime. If a field
       has several annotations, then the tag can be repeated. The offset must
       point at an :ref:`Annotation <RT Annotation>`.

   * - ``TYPE_ANNOTATION``
     - ``0x06``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a field type annotation **invisible** at runtime. If a
       field has several annotations, then the tag can be repeated. The offset
       must point at an :ref:`Annotation <RT Annotation>`.

**Constraint**: ``INT_VALUE`` and ``VALUE`` tags are mutually exclusive.

.. _RT ForeignMethod:

ForeignMethod Format
--------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 10 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - Index of a declaring class in the corresponding :ref:`ClassRegionIndex <RT ClassRegionIndex>`.
       The corresponding index entry must be an offset to a :ref:`Class <RT Class>` or to a
       :ref:`ForeignClass <RT ForeignClass>`.

   * - ``proto_idx``
     - ``uint16_t``
     - Index of a method prototype in the corresponding :ref:`ProtoRegionIndex <RT ProtoRegionIndex>`.
       The corresponding index entry must be an offset to a :ref:`Proto <RT Proto>`.

   * - ``name_off``
     - ``uint32_t``
     - Offset to the name of the method. The offset must point at a
       :ref:`String <RT String>`.

   * - ``access_flags``
     - ``uleb128``
     - Access flags of the method. The value must be a combination of
       :ref:`Method Access Flags <RT Method Access Flags>`.

.. note::
   A proper region index to resolve ``class_idx`` and ``proto_idx`` can be
   found by a foreign method offset.

.. _RT Method:

Method Format
-------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 20 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - Index of a declaring class in the corresponding :ref:`ClassRegionIndex <RT ClassRegionIndex>`.
       The corresponding index entry must be an offset to a :ref:`Class <RT Class>` or to a
       :ref:`ForeignClass <RT ForeignClass>`.

   * - ``proto_idx``
     - ``uint16_t``
     - Index of a method prototype in the corresponding :ref:`ProtoRegionIndex <RT ProtoRegionIndex>`.
       The corresponding index entry must be an offset to a :ref:`Proto <RT Proto>`.

   * - ``name_off``
     - ``uint32_t``
     - Offset to the name of the method. The offset must point at a
       :ref:`String <RT String>`.

   * - ``access_flags``
     - ``uleb128``
     - Access flags of the method. The value must be a combination of
       :ref:`Method Access Flags <RT Method Access Flags>`.

   * - ``method_data``
     - ``TaggedValue[]``
     - Variable length list of tagged values. Each element must have the
       :ref:`TaggedValue <RT TaggedValue>` format. A tag must have values from
       :ref:`Method Tags <RT Method Tags>`, and follow the increasing tag
       order (except ``0x00`` tag, which must be the last).

.. note::
   A proper region index to resolve ``class_idx`` and ``proto_idx`` can
   be found by a foreign method offset.

.. _RT Proto:

Proto Format
------------

**Alignment**: 2 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 15 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``shorty``
     - ``uint16_t[]``
     - Short representation of a prototype. Syntax is described in
       :ref:`Shorty <RT Shorty>`.

   * - ``reference_types``
     - ``uint16_t[]``
     - Array of indices of the non-primitive types in a method signature.
       Each ``ref`` type in a ``shorty`` has a corresponding element
       in the array. The size of the array equals the number of ``ref`` types
       in the ``shorty``.

.. note::
   A proper region index to resolve reference type indices can be found
   by the proto offset.

.. _RT Shorty:

Shorty Syntax
-------------

``Shorty`` is a short description of a method signature without detailed
information on reference types. A ``shorty`` begins with a return type
followed by method arguments, and ends with ``0x0``.

The ``shorty`` syntax is as follows:

.. code-block:: abnf

    Shorty:
        ReturnType ParamTypeList 0x0
        ;
    ReturnType:
        Type
        ;
    ParamTypeList:
        (Type)*
        ;

``Type`` encoding:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Encoding

   * - ``void``
     - ``0x01``

   * - ``u1``
     - ``0x02``

   * - ``i8``
     - ``0x03``

   * - ``u8``
     - ``0x04``

   * - ``i16``
     - ``0x05``

   * - ``u16``
     - ``0x06``

   * - ``i32``
     - ``0x07``

   * - ``u32``
     - ``0x08``

   * - ``f32``
     - ``0x09``

   * - ``f64``
     - ``0x0a``

   * - ``i64``
     - ``0x0b``

   * - ``u64``
     - ``0x0c``

   * - ``any``
     - ``0x0d``

   * - ``ref``
     - ``0x0e``

All ``Shorty`` elements are divided into groups of four elements starting from the
beginning. Each group is encoded in ``uint16_t``. Each element is encoded in 4
bits. A group of four elements ``v1``, ``v2``, ``v3``, and ``v4`` is encoded in
``uint16_t`` as follows:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Values
     - Bits

   * - ``v1``
     - ``0-3``

   * - ``v2``
     - ``4-7``

   * - ``v3``
     - ``8-11``

   * - ``v4``
     - ``12-15``

If a group contains less than 4 elements, then the rest bits are filled
with ``0x0``.

.. _RT Method Access Flags:

Method Access Flags
-------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``ACC_PUBLIC``
     - ``0x0001``
     - Declared public.

   * - ``ACC_PRIVATE``
     - ``0x0002``
     - Declared private.

   * - ``ACC_PROTECTED``
     - ``0x0004``
     - Declared protected.

   * - ``ACC_STATIC``
     - ``0x0008``
     - Declared static.

   * - ``ACC_FINAL``
     - ``0x0010``
     - Declared final.

   * - ``ACC_SYNCHRONIZED``
     - ``0x0020``
     - Declared synchronized.

   * - ``ACC_BRIDGE``
     - ``0x0040``
     - Bridge method, generated by the compiler.

   * - ``ACC_VARARGS``
     - ``0x0080``
     - Declared with variable number of arguments.

   * - ``ACC_NATIVE``
     - ``0x0100``
     - Declared native.

   * - ``ACC_ABSTRACT``
     - ``0x0400``
     - Declared abstract.

   * - ``ACC_STRICT``
     - ``0x0800``
     - Declared strictfp.

   * - ``ACC_SYNTHETIC``
     - ``0x1000``
     - Declared synthetic; not present in the source code.

**Constraint**: Only the ``ACC_STATIC`` flag is used for a :ref:`ForeignMethod <RT ForeignMethod>`.
Other flags are ignored.

.. _RT Method Tags:

Method Tags
-----------

.. list-table::
   :width: 100%
   :align: left
   :widths: 30 10 10 15 35
   :header-rows: 1

   * - Name
     - Tag
     - Quantity
     - Data Format
     - Description

   * - ``NOTHING``
     - ``0x00``
     - ``1``
     - ``none``
     - No more values. The value with this tag must be the last.

   * - ``CODE``
     - ``0x01``
     - ``0-1``
     - ``uint8_t[4]``
     - Offset to the code of a method. The offset must point at a :ref:`Code <RT Code>`.

   * - ``SOURCE_LANG``
     - ``0x02``
     - ``0-1``
     - ``uint8_t``
     - The :ref:`Source Language <RT Source Language>` of a method.

   * - ``RUNTIME_ANNOTATION``
     - ``0x03``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a method annotation **visible** at runtime. If a method
       has several annotations, then the tag can be
       repeated. The offset must point at an :ref:`Annotation <RT Annotation>`.

   * - ``RUNTIME_PARAM_ANNOTATION``
     - ``0x04``
     - ``0-1``
     - ``uint8_t[4]``
     - Offset to a method parameters annotation **visible** at runtime.
       The offset must point at a :ref:`ParamAnnotations <RT ParamAnnotations>`.

   * - ``DEBUG_INFO``
     - ``0x05``
     - ``0-1``
     - ``uint8_t[4]``
     - Offset to debug information related to a method.
       The offset must point at a :ref:`InstDebugInfo <RT InstDebugInfo>`.

   * - ``ANNOTATION``
     - ``0x06``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset a method annotation **invisible** at runtime.
       If a method has several annotations, then the tag can be
       repeated. The offset must point at an :ref:`Annotation <RT Annotation>`.

   * - ``PARAM_ANNOTATION``
     - ``0x07``
     - ``0-1``
     - ``uint8_t[4]``
     - Offset to a method parameters annotation **invisible** at runtime.
       The offset must point at a :ref:`ParamAnnotations <RT ParamAnnotations>`.

   * - ``TYPE_ANNOTATION``
     - ``0x08``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset a type annotation of a method **invisible** at runtime.
       If a method has several annotations, then the tag can be
       repeated. The offset must point at an :ref:`Annotation <RT Annotation>`.

   * - ``RUNTIME_TYPE_ANNOTATION``
     - ``0x09``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset a method annotation **visible** at runtime.
       If a method has several annotations, then the tag can be
       repeated. The offset must point at an :ref:`Annotation <RT Annotation>`.

   * - ``PROFILE_INFO``
     - ``0x0a``
     - ``>=0``
     - ``uint8_t[]``
     - Profile information. Format is unspecified and
       determined by the language used in a file.

.. _RT Code:

Code Format
-----------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``num_vregs``
     - ``uleb128``
     - Number of registers (without argument registers).

   * - ``num_args``
     - ``uleb128``
     - Number of arguments.

   * - ``code_size``
     - ``uleb128``
     - Size of instructions in bytes.

   * - ``tries_size``
     - ``uleb128``
     - Number of try blocks.

   * - ``instructions``
     - ``uint8_t[]``
     - Instructions.

   * - ``try_blocks``
     - ``TryBlock[]``
     - Array of try blocks. The array has ``tries_size`` elements.
       Each element has the :ref:`TryBlock <RT TryBlock>` format.

.. _RT TryBlock:

TryBlock Format
---------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 15 70
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``start_pc``
     - ``uleb128``
     - Start ``pc`` of a ``TryBlock``. This ``pc`` points to the first
       instruction covered by a ``TryBlock``.

   * - ``length``
     - ``uleb128``
     - Number of instructions covered by a ``TryBlock``.

   * - ``num_catches``
     - ``uleb128``
     - Number of catch blocks associated with a ``TryBlock``.

   * - ``catch_blocks``
     - ``CatchBlock[]``
     - Array of :ref:`CatchBlock <RT CatchBlock>` associated with a ``TryBlock``.
       The array has ``num_catches`` elements in :ref:`CatchBlock <RT CatchBlock>` format.
       Catch blocks are ordered in the order in which their exception type must be
       checked at runtime. The *catch all* block, if present, must be the last.

.. _RT CatchBlock:

CatchBlock Format
-----------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 10 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``type_idx``
     - ``uleb128``
     - Index + 1 of the exception type handled by a
       :ref:`CatchBlock <RT CatchBlock>`, or ``0`` in case of a *catch all* block.
       The corresponding index entry must be an offset to a :ref:`ForeignClass <RT ForeignClass>`
       or to a :ref:`Class <RT Class>`. If the index is 0, then it is a *catch all* block
       that catches all exceptions.

   * - ``handler_pc``
     - ``uleb128``
     - ``pc`` of the first instruction of the exception handler.

   * - ``code_size``
     - ``uleb128``
     - Handler code size in bytes.

.. note::
   A proper region index to resolve ``type_idx`` can be found by the
   corresponding method offset.

.. _RT ParamAnnotations:

ParamAnnotations Format
-----------------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 20 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``count``
     - ``uint32_t``
     - Number of parameters that a method has. This number includes synthetic
       and mandated parameters.

   * - ``annotations``
     - ``AnnotationArray[]``
     - Array of annotation lists for each parameter. The array has ``count``
       elements. Each element must have the :ref:`AnnotationArray <RT AnnotationArray>` format.

.. _RT AnnotationArray:

AnnotationArray Format
----------------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 10 15 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``count``
     - ``uint32_t``
     - Number of elements in the array.

   * - ``offsets``
     - ``uint32_t[]``
     - Array of offsets to parameter annotations. Each offset must refer to
       an :ref:`Annotation <RT Annotation>`. The array has ``count`` elements.

.. _RT ForeignClass:

ForeignClass Format
-------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``name``
     - ``String``
     - Name of a foreign type. The name must conform to
       :ref:`Type Descriptor <RT Type Descriptor>` syntax.

.. _RT Class:

Class Format
------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 20 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``name``
     - ``String``
     - Name of a type. The name must conform to :ref:`Type Descriptor <RT Type Descriptor>`
       syntax.

   * - ``super_class_off``
     - ``uint8_t[4]``
     - Offset to the name of a super class or ``0`` for root object class.
       A non-zero offset must point at a :ref:`ForeignClass <RT ForeignClass>`
       or at a :ref:`Class <RT Class>`.

   * - ``access_flags``
     - ``uleb128``
     - Access flags of a class. A value must be a combination of
       :ref:`Class Access Flags <RT Class Access Flags>`.

   * - ``num_fields``
     - ``uleb128``
     - Number of fields a class has.

   * - ``num_methods``
     - ``uleb128``
     - Number of methods a class has.

   * - ``class_data``
     - ``TaggedValue[]``
     - Variable length list of tagged values. Each element must have the
       :ref:`TaggedValue <RT TaggedValue>` format. A tag must have values from
       :ref:`Class Tags <RT Class Tags>`, and follow the increasing
       tag order (except ``0x00`` tag, which must be the last).

   * - ``fields``
     - ``Field[]``
     - Class fields. Number of elements is ``num_fields``. Each element must
       have the :ref:`Field <RT Field>` format.

   * - ``methods``
     - ``Method[]``
     - Class methods. Number of elements is ``num_methods``. Each element must
       have the :ref:`Method <RT Method>` format.

.. _RT Type Descriptor:

Type Descriptor
---------------

Type descriptors in a binary file have the following syntax:

.. code-block:: abnf

    TypeDescriptor:
        PrimitiveType
        | ArrayType
        | RefType
        | UnionType
        ;
    PrimitiveType:
        'Z'
        | 'B'
        | 'H'
        | 'S'
        | 'C'
        | 'I'
        | 'U'
        | 'F'
        | 'D'
        | 'J'
        | 'Q'
        | 'A'
        ;
    ArrayType:
        '[' TypeDescriptor
        ;
    RefType:
        'L' RefTypeName ';'
        ;
    UnionType:
        '{U' TypeDescriptor TypeDescriptor (TypeDescriptor)* '}'
        ;

``RefTypeName`` is a :ref:`Fully Qualified Name <RT Fully Qualified Name>` of the type with all dots (``'.''``) replaced for slashes (``'/''``).

``PrimitiveType`` encoding:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Encoding
     - Description

   * - ``u1``
     - ``Z``
     - 1-bit unsigned integer

   * - ``i8``
     - ``B``
     - 8-bit signed integer

   * - ``u8``
     - ``H``
     - 8-bit unsigned integer

   * - ``i16``
     - ``S``
     - 16-bit signed integer

   * - ``u16``
     - ``C``
     - 16-bit unsigned integer

   * - ``i32``
     - ``I``
     - 32-bit signed integer

   * - ``u32``
     - ``U``
     - 32-bit unsigned integer

   * - ``f32``
     - ``F``
     - Floating-point number with single precision

   * - ``f64``
     - ``D``
     -  Floating-point number with double precision

   * - ``i64``
     - ``J``
     - 64-bit signed integer

   * - ``u64``
     - ``Q``
     - 64-bit unsigned integer

   * - ``any``
     - ``A``
     - any

**Constraint**: ``UnionType`` must be canonicalized in a binary file to
ensure that no equivalent union type is present. Canonicalization
presumes alphabetically sorting ``TypeDescriptor`` s of all
*constituent types*.

.. _RT Class Access Flags:

Class Access Flags
------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``ACC_PUBLIC``
     - ``0x0001``
     - Declared public.

   * - ``ACC_FINAL``
     - ``0x0010``
     - Declared final.

   * - ``ACC_SUPER``
     - ``0x0020``
     - No special meaning. Added for compatibility only

   * - ``ACC_INTERFACE``
     - ``0x0200``
     - Is an interface.

   * - ``ACC_ABSTRACT``
     - ``0x0400``
     - Declared abstract.

   * - ``ACC_SYNTHETIC``
     - ``0x1000``
     - Declared synthetic; not present in the source code.

   * - ``ACC_ANNOTATION``
     - ``0x2000``
     - Declared an annotation type.

   * - ``ACC_ENUM``
     - ``0x4000``
     - Declared an enum type.

.. _RT Class Tags:

Class Tags
----------

.. list-table::
   :width: 100%
   :align: left
   :widths: 30 10 10 15 35
   :header-rows: 1

   * - Name
     - Tag
     - Quantity
     - Data Format
     - Description

   * - ``NOTHING``
     - ``0x00``
     - ``1``
     - ``none``
     - No more values. The value with this tag must be the last.

   * - ``INTERFACES``
     - ``0x01``
     - ``0-1``
     - ``uleb128 uint8_t[]``
     - List of interfaces a class implements. Data contains the number of
       interfaces encoded in the ``uleb128`` format followed by indices of
       interfaces in the :ref:`ClassRegionIndex <RT ClassRegionIndex>` format. Each index is
       2-byte long, and when resolved must point at a
       :ref:`ForeignClass <RT ForeignClass>` or at a :ref:`Class <RT Class>`. The number of indices is equal
       to the number of interfaces.

   * - ``SOURCE_LANG``
     - ``0x02``
     - ``0-1``
     - ``uint8_t``
     - Data represents the :ref:`Source Language <RT Source Language>`.

   * - ``RUNTIME_ANNOTATION``
     - ``0x03``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a class annotation **visible** at runtime. If a class has
       several annotations, then the tag can be repeated. The offset must point
       at an :ref:`Annotation <RT Annotation>`.

   * - ``ANNOTATION``
     - ``0x04``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a class annotation **invisible** at runtime. If a class
       has several annotations, then the tag can be repeated. The offset must
       point at an :ref:`Annotation <RT Annotation>`.

   * - ``RUNTIME_TYPE_ANNOTATION``
     - ``0x05``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a class type annotation **visible** at runtime. If a class
       has several annotations, then the tag can be repeated. The offset must
       point at an :ref:`Annotation <RT Annotation>`.

   * - ``TYPE_ANNOTATION``
     - ``0x06``
     - ``>=0``
     - ``uint8_t[4]``
     - Offset to a class type annotation **invisible** at runtime. If a
       class has several annotations, then the tag can be repeated. The offset
       must point at an :ref:`Annotation <RT Annotation>`.

   * - ``SOURCE_FILE``
     - ``0x07``
     - ``0-1``
     - ``uint8_t[4]``
     - Offset to a file name string containing source code of a class.

.. note::
   A proper region index to resolve interface indices can be found
   by the class offset.

.. _RT Annotation:

Annotation Format
-----------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 25 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``class_idx``
     - ``uint16_t``
     - Index of a declaring class in the :ref:`ClassRegionIndex <RT ClassRegionIndex>` format.
       The corresponding index entry must be an offset to a :ref:`Class <RT Class>` or to a
       :ref:`ForeignClass <RT ForeignClass>`.

   * - ``count``
     - ``uint16_t``
     - The number of name-value pairs in the annotation
       (number of elements in ``elements`` array).

   * - ``elements``
     - ``AnnotationElement[]``
     - Array of annotation elements. Each element must have the
       :ref:`AnnotationElement <RT AnnotationElement>` format. The order of elements must be the
       same as in the annotation class.

   * - ``element_types``
     - ``uint8_t[]``
     - Array of annotation element types. Each element in ``element_types``
       describes the type of a corresponding :ref:`AnnotationElement <RT AnnotationElement>` in
       ``elements``. The order of elements in the array matches the order of
       the ``elements`` field. Each element must have a value from
       the :ref:`Annotation Element Type <RT Annotation Element Type>`.

.. note::
   A proper region index to resolve ``class_idx`` can be found by the annotation offset.

.. _RT Annotation Element Type:

Annotation Element Type
-----------------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Type
     - Tag

   * - ``u1``
     - ``1``

   * - ``i8``
     - ``2``

   * - ``u8``
     - ``3``

   * - ``i16``
     - ``4``

   * - ``u16``
     - ``5``

   * - ``i32``
     - ``6``

   * - ``u32``
     - ``7``

   * - ``i64``
     - ``8``

   * - ``u64``
     - ``9``

   * - ``f32``
     - ``A``

   * - ``f64``
     - ``B``

   * - ``string``
     - ``C``

   * - ``record``
     - ``D``

   * - ``method``
     - ``E``

   * - ``enum``
     - ``F``

   * - ``annotation``
     - ``G``

   * - ``method_handle``
     - ``J``

   * - ``array``
     - ``H``

   * - ``u1[]``
     - ``K``

   * - ``i8[]``
     - ``L``

   * - ``u8[]``
     - ``M``

   * - ``i16[]``
     - ``N``

   * - ``u16[]``
     - ``O``

   * - ``i32[]``
     - ``P``

   * - ``u32[]``
     - ``Q``

   * - ``i64[]``
     - ``R``

   * - ``u64[]``
     - ``S``

   * - ``f32[]``
     - ``T``

   * - ``f64[]``
     - ``U``

   * - ``string[]``
     - ``V``

   * - ``record[]``
     - ``W``

   * - ``method[]``
     - ``X``

   * - ``enum[]``
     - ``Y``

   * - ``annotation[]``
     - ``Z``

   * - ``method_handle[]``
     - ``@``

   * - ``nullptr_string``
     - ``*``

.. note::
   The correct value for element with ``nullptr_string`` tag is ``0``
   (``\x00\x00\x00\x00``)

.. _RT AnnotationElement:

AnnotationElement Format
------------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 15 15 70
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``name_off``
     - ``uint32_t``
     - Offset to the element name. The offset must point at a :ref:`String <RT String>`.

   * - ``value``
     - ``uint32_t``
     - Value of the element. If the type of an annotation element is ``boolean``,
       ``byte``, ``short``, ``char``, ``int``, or ``float``, then the field
       ``value`` contains the value itself in the corresponding :ref:`Value <RT Value>`
       format. Otherwise, the field contains an offset to the :ref:`Value <RT Value>`.
       The format of this :ref:`Value <RT Value>` can be determined based on the
       :ref:`AnnotationElement <RT AnnotationElement>` type.

.. _RT Value:

Value Format
------------

There are different value encodings depending on the value type.

.. _RT ByteValue:

ByteValue Format
----------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t``
     - Signed 1-byte integer value.

.. _RT ShortValue:

ShortValue Format
-----------------

**Alignment**: 2 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[2]``
     - Signed 2-byte integer value.

.. _RT IntegerValue:

IntegerValue Format
-------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[4]``
     - Signed 4-byte integer value.

.. _RT LongValue:

LongValue Format
----------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[8]``
     - Signed 8-byte integer value.

.. _RT FloatValue:

FloatValue Format
-----------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[4]``
     - 4-byte bit pattern, zero-extended to the right, and interpreted as an
       IEEE754 32-bit floating point value.

.. _RT DoubleValue:

DoubleValue Format
------------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[8]``
     - 8-byte bit pattern, zero-extended to the right, and interpreted as an
       IEEE754 64-bit floating point value.

.. _RT StringValue:

StringValue Format
------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - The value represents an offset to a :ref:`String <RT String>`.

.. _RT EnumValue:

EnumValue Format
----------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - The value represents an offset to an enum field. An offset must
       point at a :ref:`Field <RT Field>` or at a :ref:`ForeignField <RT ForeignField>`.

.. _RT ClassValue:

ClassValue Format
-----------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - The value represents an offset to a :ref:`Class <RT Class>` or to a :ref:`ForeignClass <RT ForeignClass>`.

.. _RT AnnotationValue:

AnnotationValue Format
----------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - The value represents an offset to a :ref:`Annotation <RT Annotation>`.

.. _RT MethodValue:

MethodValue Format
------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - The value represents an offset to a :ref:`Method <RT Method>` or to a :ref:`ForeignMethod <RT ForeignMethod>`.

.. _RT MethodHandleValue:

MethodHandleValue Format
------------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - The value represents an offset to a :ref:`MethodHandle <RT MethodHandle>`.

.. _RT MethodTypeValue:

MethodTypeValue Format
----------------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint32_t``
     - The value represents an offset to a :ref:`Proto <RT Proto>`.

.. _RT ArrayValue:

ArrayValue Format
-----------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``count``
     - ``uleb128``
     - Number of elements in the array.

   * - ``value``
     - ``uint32_t``
     - Unaligned array of :ref:`Value <RT Value>` entities. The array has ``count`` elements.
       Format depends on the ``tag_value`` field.

.. _RT LiteralArray:

LiteralArray Format
-------------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 15 65
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``num_literals``
     - ``uint32_t``
     - Interpreted differently for different :ref:`Literal Tags <RT Literal Tags>`.
       If ``literals`` have an ``ARRAY_*`` ``tag``, then ``num_literals``
       equals to the number of elements in an array. For other tags ``num_literals``
       equals to the number of elements in ``literals`` array times 2 (since each ``tag``
       can be interpreted as separate :ref:`Literal <RT Literal>`).

   * - ``literals``
     - ``Literal[]``
     - Elements of a literal array. The number of elements is ``num_literals``.
       Each element must have the :ref:`Literal <RT Literal>` format.

.. _RT Literal Tags:

Literal Tags
------------

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Tag
     - Value
     - ``data`` encoding

   * - ``TAGVALUE``
     - ``0x00``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``BOOL``
     - ``0x01``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``INTEGER``
     - ``0x02``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``FLOAT``
     - ``0x03``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``DOUBLE``
     - ``0x04``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``STRING``
     - ``0x05``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``BIGINT``
     - ``0x06``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``METHOD``
     - ``0x07``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``GENERATORMETHOD``
     - ``0x08``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ACCESSOR``
     - ``0x09``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``METHODAFFILIATE``
     - ``0x0a``
     - :ref:`ByteTwo <RT ByteTwo>`

   * - ``ARRAY_U1``
     - ``0x0b``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``ARRAY_U8``
     - ``0x0c``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``ARRAY_I8``
     - ``0x0d``
     - :ref:`ByteOne <RT ByteOne>`

   * - ``ARRAY_U16``
     - ``0x0e``
     - :ref:`ByteTwo <RT ByteTwo>`

   * - ``ARRAY_I16``
     - ``0x0f``
     - :ref:`ByteTwo <RT ByteTwo>`

   * - ``ARRAY_U32``
     - ``0x10``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ARRAY_I32``
     - ``0x11``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ARRAY_U64``
     - ``0x12``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``ARRAY_I64``
     - ``0x13``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``ARRAY_F32``
     - ``0x14``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ARRAY_F64``
     - ``0x15``
     - :ref:`ByteEight <RT ByteEight>`

   * - ``ARRAY_STRING``
     - ``0x16``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ASYNCGENERATORMETHOD``
     - ``0x17``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``ASYNCMETHOD``
     - ``0x18``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``LITERALARRAY``
     - ``0x19``
     - :ref:`ByteFour <RT ByteFour>`

   * - ``NULLVALUE``
     - ``0xff``
     - :ref:`ByteOne <RT ByteOne>`

.. _RT Literal:

Literal Format
--------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 10 15 75
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``tag``
     - ``uint8_t``
     - Tag of a literal. A tag of a literal instructs how to interpret
       ``num_literals`` and ``data`` fields. A tag of a literal must be one of
       :ref:`Literal Tags <RT Literal Tags>`.

   * - ``data``
     - ``uint8_t[]``
     - Elements of a literal array. The number of elements is ``num_literals``.
       Different ``data`` encodings depend on the value of a ``tag``.

.. _RT ByteOne:

ByteOne Format
--------------

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t``
     - 1-byte value.

.. _RT ByteTwo:

ByteTwo Format
--------------

**Alignment**: 2 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[2]``
     - 2-byte value.

.. _RT ByteFour:

ByteFour Format
---------------

**Alignment**: 4 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[4]``
     - 4-byte value.

.. _RT ByteEight:

ByteEight Format
----------------

**Alignment**: 8 bytes

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``value``
     - ``uint8_t[8]``
     - 8-byte value.

.. _RT InstDebugInfo:

InstDebugInfo Format
--------------------

Debug information contains mapping between program counter of a method,
line numbers in source code, and information about local variables. The format
is derived from the DWARF Debugging Information Format, Version 3 (see subsection 6.2).
The mapping and local variable information are encoded in
the line number program, which is interpreted by the :ref:`State Machine <RT State Machine>`.
All constants the program refers to are moved into the :ref:`Constant Pool <RT Constant Pool>`
in order to deduplicate identical line number programs of different methods.

**Alignment**: None

**Format**:

.. list-table::
   :width: 100%
   :align: left
   :widths: 30 15 55
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``line_start``
     - ``uleb128``
     - The initial value of line register of the :ref:`State Machine <RT State Machine>`.

   * - ``num_parameters``
     - ``uleb128``
     - Number of method parameters.

   * - ``parameters``
     - ``uleb128[]``
     - Parameter names of a method. An array has ``num_parameters``
       elements. Each element is an offset to a :ref:`String <RT String>` or ``0`` if no name is available.

   * - ``constant_pool_size``
     - ``uleb128``
     - Size of constant pool in bytes.

   * - ``constant_pool``
     - ``uleb128[]``
     - :ref:`Constant pool <RT Constant pool>` data. Length is ``constant_pool_size`` bytes.

   * - ``line_number_program_idx``
     - ``uleb128``
     - Line number program index in the :ref:`LineNumberProgramIndex <RT LineNumberProgramIndex>`
       format. Programs have variable lengths, and end in ``DBG_END_SEQUENCE`` opcode.

.. _RT Constant Pool:

Constant Pool
-------------

Many methods have similar line number programs that differ in
variable names, variable types, and file names only. In order to deduplicate such
programs, all constants the program refers to are stored in the constant pool.
During program interpretation, the :ref:`State Machine <RT State Machine>` tracks a pointer to
the constant pool. When the :ref:`State Machine <RT State Machine>` interprets an instruction
that requires a constant argument, the machine reads memory for the value pointed to by
the constant pool pointer, and then increments the pointer. Thus,
a program has no explicit reference to constants, and can be deduplicated.

.. _RT State Machine:

State Machine
-------------

The aim of the state machine is to generate mapping between the program
counter, line numbers, and local variable information. The machine has
the following registers:

.. list-table::
   :width: 100%
   :align: left
   :widths: 22 20 58
   :header-rows: 1

   * - Name
     - Initial value
     - Description

   * - ``address``
     - 0
     - Program counter (refers to method instructions).
       Must always increase monotonically.

   * - ``line``
     - ``line_start`` from :ref:`InstDebugInfo <RT InstDebugInfo>`.
     - Unsigned integer which corresponds to a line number in source code.
       All lines are numbered beginning at 1, and the register must not have
       a value less than 1.

   * - ``file``
     - Value of ``SOURCE_FILE`` tag in ``class_data`` (see :ref:`Class <RT Class>`) or ``0``.
     - Offset to the name of a source file. If no such information is available
       (i.e., a ``SOURCE_FILE`` tag is absent in :ref:`Class <RT Class>`), then the register
       value is ``0``.

   * - ``prologue_end``
     - ``false``
     - Register indicates the current address where
       an entry breakpoint can be set for a method.

   * - ``epilogue_begin``
     - ``false``
     - Register indicates the current address where
       an exit breakpoint can be set for a method.

   * - ``constant_pool_ptr``
     - Address of the first byte of ``constant_pool`` i :ref:`InstDebugInfo <RT InstDebugInfo>`.
     - Pointer to the current constant value.

.. _RT Line Number Program:

Line Number Program Format
--------------------------

A line number program consists of instructions. Each instruction has one
byte opcode and optional arguments. Depending on opcode, an argument
value can be encoded into the instruction. Otherwise, the instruction
requires the value to be read from the constant pool.

.. list-table::
   :width: 100%
   :align: left
   :widths: 25 10 10 10 45
   :header-rows: 1

   * - Opcode
     - Value
     - Instruction Format
     - Constant Pool Args
     - Description

   * - ``END_SEQUENCE``
     - ``0x00``
     - 
     - 
     - Marks the end of line number program.

   * - ``ADVANCE_PC``
     - ``0x01``
     - 
     - ``uleb128``
     - Increments ``address`` register by the value that ``constant_pool_ptr``
       refers to without emitting a line.

   * - ``ADVANCE_LINE``
     - ``0x02``
     - 
     - ``sleb128``
     - Increments ``line`` register by the value that ``constant_pool_ptr``
       refers to without emitting a line.

   * - ``START_LOCAL``
     - ``0x03``
     - ``sleb128``
     - ``uleb128 uleb128``
     - Introduces a local variable with the name and type that
       ``constant_pool_ptr`` refers to at the current address. The number of a
       register that contains the variable is encoded in the instruction.
       Register value ``-1`` signifies the accumulator register. The name is an
       offset to a :ref:`String <RT String>`, and the type is an offset to a
       :ref:`ForeignClass <RT ForeignClass>` or to a :ref:`Class <RT Class>`.
       If corresponding information is missing, then the offset is ``0``.

   * - ``START_LOCAL_EXTENDED``
     - ``0x04``
     - ``sleb128``
     - ``uleb128 uleb128 uleb128``
     - Introduces a local variable with the name, type, and type signature that
       ``constant_pool_ptr`` refers to at the current address. The number of a
       register that contains the variable is encoded in the instruction.
       Register value ``-1`` signifies the accumulator register.
       The name is an offset to a :ref:`String <RT String>`, and the type is an offset
       to a :ref:`ForeignClass <RT ForeignClass>` or to a :ref:`Class <RT Class>`.
       If corresponding information is missing, then the offset is ``0``.

   * - ``END_LOCAL``
     - ``0x05``
     - ``sleb128``
     - 
     - Marks that local variable in a specified register is out of scope.
       The register number is encoded in the instruction. Register value ``-1``
       signifies the accumulator register.

   * - ``RESTART_LOCAL``
     - ``0x06``
     - ``sleb128``
     - 
     - Re-introduces a local variable at a specified register. The name and
       type are the same as the last local in the register. The number of the
       register is encoded in the instruction. Register value ``-1`` signifies
       the accumulator register.

   * - ``SET_PROLOGUE_END``
     - ``0x07``
     - 
     - 
     - Sets ``prologue_end`` register to ``true``. Any special opcodes clear
       ``prologue_end`` register.

   * - ``SET_EPILOGUE_BEGIN``
     - ``0x08``
     - 
     - 
     - Sets ``epilogue_end`` register to ``true``.  Any special opcodes clear
       ``epilogue_end`` register.

   * - ``SET_FILE``
     - ``0x09``
     - 
     - ``uleb128``
     - Sets ``file`` register to the value ``constant_pool_ptr`` refers to. The
       argument is an offset to a :ref:`String <RT String>` which represents the file
       name or ``0``.

   * - ``SET_SOURCE_CODE``
     - ``0x0a``
     - 
     - ``uleb128``
     - Sets ``source_code`` register to the value ``constant_pool_ptr``
       refers to. The argument is an offset to a :ref:`String <RT String>` which
       represents the source code or ``0``.

   * - ``SET_COLUMN``
     - ``0x0b``
     - 
     - ``uleb128``
     - Sets ``column`` register by the value that ``constant_pool_ptr`` refers to

   * - Special opcodes
     - ``0x0c .. 0xff``
     - 
     - 
     - 

Special opcodes:

:ref:`State Machine <RT State Machine>` interprets each special opcode as
follows (see DWARF Debugging Information Format subsection 6.2.5.1 Special
Opcodes):

1. Calculate the adjusted opcode:
   ``adjusted_opcode = opcode - OPCODE_BASE``.
2. Increment ``address`` register:
   ``address += adjusted_opcode / LINE_RANGE``.
3. Increment ``line`` register:
   ``line += LINE_BASE + (adjusted_opcode % LINE_RANGE)``.
4. Emit line number.
5. Set ``prologue_end`` register to ``false``.
6. Set ``epilogue_begin`` register to ``false``.

Where:

- ``OPCODE_BASE = 0x0c``: the first special opcode.
- ``LINE_BASE = -4``: the smallest line number increment.
- ``LINE_RANGE = 15``: the number of line increments presented.

.. _RT MethodHandle:

MethodHandle Format
-------------------

Alignment: none

Format:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 20 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``type``
     - ``uint8_t``
     - Type of a handle. Must be one of :ref:`Type Of MethodHandle <RT Type Of MethodHandle>`.

   * - ``offset``
     - ``uleb128``
     - Offset to an entity of a corresponding type. Type of an entity is
       determined depending on the handle type (see
       :ref:`Type of MethodHandle <RT Type of MethodHandle>`).

.. _RT Type of MethodHandle:

Type of MethodHandle
---------------------

The types available for a method handle are as follows:

.. list-table::
   :width: 100%
   :align: left
   :widths: 20 20 60
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``PUT_STATIC``
     - ``0x00``
     - Method handle refers to a static setter. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Field <RT Field>` or at a
       :ref:`ForeignField <RT ForeignField>`.

   * - ``GET_STATIC``
     - ``0x01``
     - Method handle refers to a static getter. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Field <RT Field>` or at a
       :ref:`ForeignField <RT ForeignField>`.

   * - ``PUT_INSTANCE``
     - ``0x02``
     - Method handle refers to an instance getter. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Field <RT Field>` or at a
       :ref:`ForeignField <RT ForeignField>`.

   * - ``GET_INSTANCE``
     - ``0x03``
     - Method handle refers to an instance setter. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Field <RT Field>` or at a
       :ref:`ForeignField <RT ForeignField>`.

   * - ``INVOKE_STATIC``
     - ``0x04``
     - Method handle refers to a static method. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Method <RT Method>` or at a
       :ref:`ForeignMethod <RT ForeignMethod>`.

   * - ``INVOKE_INSTANCE``
     - ``0x05``
     - Method handle refers to an instance method. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Method <RT Method>` or at a
       :ref:`ForeignMethod <RT ForeignMethod>`.

   * - ``INVOKE_CONSTRUCTOR``
     - ``0x06``
     - Method handle refers to a constructor. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Method <RT Method>` or at a
       :ref:`ForeignMethod <RT ForeignMethod>`.

   * - ``INVOKE_DIRECT``
     - ``0x07``
     - Method handle refers to a direct method. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Method <RT Method>` or at a
       :ref:`ForeignMethod <RT ForeignMethod>`.

   * - ``INVOKE_INTERFACE``
     - ``0x08``
     - Method handle refers to an interface method. Offset in
       :ref:`MethodHandle <RT MethodHandle>` must point at a :ref:`Method <RT Method>` or at a
       :ref:`ForeignMethod <RT ForeignMethod>`.

.. _RT Argument Types:

Argument Types
--------------

A bootstrap method can accept static arguments of the following types:

.. list-table::
   :width: 100%
   :align: left
   :widths: auto
   :header-rows: 1

   * - Name
     - Format
     - Description

   * - ``Integer``
     - ``0x00``
     - Corresponding argument has the :ref:`IntegerValue <RT IntegerValue>` encoding.

   * - ``Long``
     - ``0x01``
     - Corresponding argument has the :ref:`LongValue <RT LongValue>` encoding.

   * - ``Float``
     - ``0x02``
     - Corresponding argument has the :ref:`FloatValue <RT FloatValue>` encoding.

   * - ``Double``
     - ``0x03``
     - Corresponding argument has the :ref:`DoubleValue <RT DoubleValue>` encoding.

   * - ``String``
     - ``0x04``
     - Corresponding argument has the :ref:`StringValue <RT StringValue>` encoding.

   * - ``Class``
     - ``0x05``
     - Corresponding argument has the :ref:`ClassValue <RT ClassValue>` encoding.

   * - ``MethodHandle``
     - ``0x06``
     - Corresponding argument has the :ref:`MethodHandleValue <RT MethodHandleValue>`
       encoding.

   * - ``MethodType``
     - ``0x07``
     - Corresponding argument has the :ref:`MethodTypeValue <RT MethodTypeValue>`
       encoding.
