# 2022-08-18-isa-changelog

This document describes change log with the following modifications:

* ISA refactoring
* Function Kind and Header index
* MethodId, StringId and LiteralArrayId
* LiteralArrayIndex refactoring

## ISA refactoring
The bytecode size and runtime performance have been suffering for a long time as the all the
ecmascript specific bytecode are prefixed and their opcode were encoded with two bytes.
1. We delete all original java specific opcodes and delete java specific opcode prefix.
2. We remove the prefix of ecmascript specific opcodes, such that most of the bytecode opcode can be encoded with one byte.
3. We add prefix "deprecated" and keep the many old isa as "deprecated"-prefixed opcodes (for compatibility). These prefixed opcode will be deleted once we do not need to concern compatibility.
4. We add prefix "throw" and make all throwing opcodes be prefixed by "throw".
5. We add prefix "wide" to support opcodes which need larger immediate number.
6. We adjust the format of some opcodes (about immediate number and accumulator), so that the bytecode can be more compact.
7. We change the semantics of some opcodes.
8. We add 8-bit or 16-bit imm as inline cache slot for some specific opcodes.

## Function Kind and Header index
As we merge some "define-function" opcodes as one opcode, in function we add one field which records the function kind,
such that runtime can distinguish the "define-function" operations of different kinds.
We also add header index in function such that runtime can access IndexHeader more efficiently.
We reuse the field 32-bit field `access_flags_` to encode Function Kind and Header index.
This will not introduce compatibility issue because the later 24-bit of `access_flags_` is unused indeed.
Now the layout is:

|<- 16-bit header index ->|<- 8-bit function kind ->|<- 8-bit original access flag ->|

## MethodId, StringId and LiteralArrayId
To adapt runtime design, we put string and literal array into the index header of methods,
such that the instructions can get consective indexes for methodId, stringId and literalarrayId.
This will help runtime to build constant pool more efficiently.
As the method number in a class is not limited, we release the constraint that all methodId,
stringId and literalarrayId in a class should be put in a same index header.
Instead, we only ask that all methodId, stringId and literalarrayId in a method should be put in one index header.
As we use 16-bit to encode methodId, stringId and literalarrayId, the number of these Ids in one method cannot exceed 65536 for now.
This released constraint suits for most of application scenarios.

## LiteralArrayIndex refactoring
1. We deprecate the usage of global literalarray index.
2. We use offset to reference literalarray.
3. In bytecode, we still use 16-bit literalarrayId rather than offset.
4. The layout of literalarrays can be random.
5. The literalarray which is referenced by TypeSummary contains the offsets of all type-literalarray.
