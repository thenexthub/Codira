# Avoid re-calculating the address of array data for consequent array accesses

## Overview

Since accessing an array element is done via an object and an element index,
the address of the actual array data has to be infered from the object address.


## Rationality

Having multiple access to the same array on a control flow path results in
in the unnecessary repetitive calculation of the data payload address.
This means code bloating and possible performance degradation, so, avoiding
the recalculation would reduce code size and bring some performance
improvement.


## Algorithm

Detect multiple array accesses to an array that belong to one basic block or
a chain of basic blocks that comprise a single control flow path and are not
interspersed by instructions that call runtime. Calculate the payload address
and use it in the form of the `AddI` instruction of the `ptr` type in further
array acesses instead of object address. Replace the `{Store, Load}Array`
instructions with the low-level `{Store, Load}` intructions.

Do not process array accesses that are not inside a loop as optimizing
non-repetetive sequences of instructions will not bring a measurable performance
difference.

## Examples

Machine code before the transformation:

```
  # [inst]     7.ref  NewArray 181               v8(r21), v6(r2), v5 -> r21 (v31, v24, v17, v20, v27, v33)        bc: 0x00000002
  # [inst]    24.i32  StoreArray                 v7(r21), v23(r19), v11p(r19)                                     bc: 0x00000013
    00ac: add x16, x21, #0x10 // (16)
    00b0: str w19, [x16, w19, uxtw #2]
  # [inst]    31.i32  StoreArray                 v7(r21), v30(r23), v44(r24)                                      bc: 0x0000001f
    00bc: add x16, x21, #0x10 // (16)
    00c0: str w24, [x16, w23, uxtw #2]
```

Machine code after the transformation:

```
  # [inst]     7.ref  NewArray 181               v8(r21), v6(r2), v5 -> r21 (v47, v17, v20, v27, v33)             bc: 0x00000002
  # [inst]    47.ptr  AddI                       v7(r21), 0x10 -> r25 (v49, v48)                                  bc: 0x00000002
    00ac: add w25, w21, #0x10 // (16)
  # [inst]    48.i32  Store 8                    v47(r25), v23(r19), v11p(r19)
    00b0: str w19, [x25, w19, uxtw #2]
  # [inst]    49.i32  Store 8                    v47(r25), v30(r23), v44(r24)
    00bc: str w24, [x25, w23, uxtw #2]
```
