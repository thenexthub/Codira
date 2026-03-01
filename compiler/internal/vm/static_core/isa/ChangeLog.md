File format and ISA ChangeLog
=============================

* 0.0.0.6
    + Added export table for faster reading from abc
    + Changed header - 2 new items for supporting table

* 0.0.0.5
    + Removed calli.dyn and return.dyn opcodes

* 0.0.0.4
    + Changed memory opcodes to not handle volatile fields

* 0.0.0.3
    + Added more variants of arithmetic opcodes
    + Removed builtins and API for template-based generation for builtins
    + ISAPI: removed acc_none, acc_read, acc_write properties in favor of Instruction::acc_none?,
      Instruction::acc_read?, Instruction::acc_write? properties
    + Added x_oom exception for lda.str

* 0.0.0.2
    + Removed compatibility check based on the isa checksum.
    + Introduced new backward compatibility policy.
