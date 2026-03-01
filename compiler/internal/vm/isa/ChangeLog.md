File format and ISA ChangeLog
=============================

    + Removed builtins and API for template-based generation for builtins
    + ISAPI: removed acc_none, acc_read, acc_write properties in favor of Instruction::acc_none?,
      Instruction::acc_read?, Instruction::acc_write? properties
    + Added x_oom exception for lda.str

* 0.0.0.2
    + Removed compatibility check based on the isa checksum.
    + Introduced new backward compatibility policy.
