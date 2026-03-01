# Compiled Code Metainfo

## Overview

Metainfo is an information that aims to provide reg-to-stack mapping for virtual registers. It is needed for stack
unwinding process to restore CFrame at specific PC.

When native code calls runtime or another code that can call runtime, we must provide approach to restore virtual
registers which are live during this call instruction. Since all virtual regisetrs should be saved on the stack before
we call the runtime, we can save information in which stack slot specific vreg is live.

Metainfo is placed together with compiled code:
```
 +-------------+
 | CodePrefix  |
 |             +-------------------+
 |             |  magic            |
 |             |  code_info_offset |
 |             |  code_info_size   |
 +-------------+-------------------+
 |             | <-- Method::CompiledCodeEntrypoint
 | Code        |
 |             |
 +-------------+-----------------+
 | CodeInfo    | CodeInfoHeader  |
 |             |-----------------+----------------------+
 |             |                 |  StackMap            |
 |             |                 |  InlineInfo          |
 |             |                 |  Roots Reg Mask      |
 |             |                 |  Roots Stack Mask    |
 |             |   Bit Tables    |  Method indexes      |
 |             |                 |  VRegs mask          |
 |             |                 |  VRegs map           |
 |             |                 |  VRegs catalogue     |
 |             |                 |  Implicit Nullchecks |
 |             |                 |  Constants           |
 |-------------+-----------------+----------------------+
```

## Bit table

Columns width is in a bits.

First row is a BitTable's header, that describe rows count and columns width.

```
+------------+----------------+----------------+----------------+
| Rows count | Column 0 width |      . . .     | Column N width |
+------------+----------------+----------------+----------------+
```
Header is followed by data, which is a rows with fixed length. Row length is equal to sum of columns width.

Column width can't be greater than 32 bits, because `BitTableBuilder` class, that aims to build bit tables,
use `uint32_t` for a single table element.

Example:
```
 Rows                           
 count         Columns
+-----+-----+-----+-------+-----+
|  5  |  2  |  0  |  15   |  8  |  ; Header
+-----+-----+-----+-------+-----+
      |  2  |  -  | 31547 |  23 |  ; 0 row
      |  1  |  -  |    12 | 241 |  ; 1 row
      |  1  |  -  |   128 |   1 |  ; 2 row
      |  2  |  -  |     0 |  24 |  ; 3 row
      |  0  |  -  |  4587 |   0 |  ; 4 row
      +-----+-----+-------+-----+
```
Here, we have 4 columns and 5 rows. Number of rows is defeined in the first digit in a header,
number of columns is determined in compile time.

Row size is 25 bits, that is sum of columns width: 2 + 0 + 15 + 8.
Column width is determined by maximum value in a table, e.g. for 2th column it is zero row, that has value 31547. This value fits in 15 bits.

So, the size of this table's data is 25 * 5 = 125 bits = 15.625 bytes.

## Bitmap table

Bitmap table is a Bit table with one column, that doesn't have 32-bits limitation for the width.

## Numbers packing

For number compressisng following approach is used.

The first four bits determine the variable length of the encoded number:
- Values 0..11 represent the result as-is, with no further following bits.
- Values 12..15 mean the result is in the next 8/16/24/32-bits respectively.

Example for numbers (2, 0, 15, 254874):
```
+---------+---------+---------+---------+---------+---------+
| 0 byte  | 1 byte  | 2 byte  | 3 byte  | 4 byte  | 5 byte  |
+---------+---------+---------+---------+---------+---------+
|  2 |  0 | 12 | 14 |    15   |            254874           |
+---------+---------+---------+---------+---------+---------+
```

## Code header

Code headers describes structure of the compiled code and metainfo.

| Field | Description |
|-----|----|
| PROPERTIES | Properties of the code info (e.g. frame size) |
| CALLEE_REG_MASK | Specifies registers mask that is saved in the method |
| CALLEE_FP_REG_MASK | Specifies fp registers mask that is saved in the method |
| TABLE_MASK | Bit mask of existing bit tables |
| VREGS_COUNT | Number of virtual registers in reg map |

## Tables

### 1. StackMap

| Field | Description |
|-----|----|
| PROPERTIES | Define properties of the stackmap, currently, it contains only one flag: is_osr |
| NATIVE_PC | Native address to which this stackmap corresponds |
| BYTECODE_PC | Bytecode address to which this stackmap corresponds |
| ROOTS_REG_MASK_INDEX | Mask of the CPU registers that hold managed objects |
| ROOTS_STACK_MASK_INDEX | Mask of the stack slots that hold managed objects |
| INLINE_INFO_INDEX | Inline information for the stackmap |
| VREG_MASK_INDEX | Mask of the virtual registers, that are modified from the last stackmap to the current one |
| VREG_MAP_INDEX | Map of the virtual registers, that are modified from the last stackmap to the current one |

> NOTE: fields with `_INDEX` in the end of the name contain only index of the record in the corresponding table.

### 2. InlineInfo

| Field | Description |
|-----|----|
| IS_LAST | Whether this inlined method is a last, i.e. is it a leaf |
| BYTECODE_PC | Bytecode address to which this inline info corresponds |
| METHOD_ID_INDEX | Index of the method id in the Methods table |
| METHOD_HI | Hi 32-bit part of the method pointer (actual only for jit) |
| METHOD_LOW | Low 32-bit part of the method pointer (actual only for jit)  |
| VREGS_COUNT | Number of virtual registers, that belongs to this inlined method |

Stackmap and all its inlined infos use same Virtual registers map, but separate it via `VREGS_COUNT` field of inline infoes.

Example for inline chain `method 0` -> `inlines 1` -> `inlines 3`:
```
-------------
vreg 0 
vreg 1  ; method 0: CodeInfoHeader::VREGS_COUNT=3
vreg 2
---------------
vreg 3  ; method 1: InlineInfo::VREGS_COUNT=1
---------------
vreg 4
vreg 5  ; method 3: InlineInfo::VREGS_COUNT=2
```

### 3. Roots Reg Mask

This is a Bitmap table, where column is a bit mask, that determines which CPU register holds a managed object.

### 4. Roots Stack Mask

This is a Bitmap table, where column is a bit mask, that determines which stack slot holds a managed object.

### 5. Method indexes

Holds single column - method index within a pandafile. It was moved to the separate table due to better deduplication.

### 6. VRegs mask

This is a Bitmap table, where column is a bit mask, that determines which virtual register is modified in the StackMap.

### 7. VRegs map

Holds single column - index of the VReg description in the `VRegs catalogue` table.

### 8. VRegs catalogue

| Field | Description |
|-----|----|
| INFO | Virtual register description |
| VALUE | Virtual register value |

Virtual register description has the following fields:
- **Location** - where vreg is stored: stack slot, CPU register or constant.
- **Type** - type of the value: OBJECT, INT32, INT64, FLOAT32, FLOAT64, BOOL.
- **IsAccumulator** - whethre vreg is accumulator.
- **Index** - index of the virtual register.

Value of the `VALUE` field depends on the value of `Location` field:
- CPU register: number of CPU register
- stack slot: number of the slot within a frame
- constant: index of a row within `Constants` table

### 9. Implicit nullchecks

This is a table with offsets: removed NullCheck instruction position and corresponding SlowPath position.
The table helps the signal handler to find the SlowPath address to continue execution after a segmentation fault.

| Field | Description |
|-----|----|
| INST_NATIVE_PC | NullCheck instruction position. |
| SLOW_PATH_NATIVE_PC | NullCheck SlowPath position. |

### 10. Constants

This table contains only one column - constant value.