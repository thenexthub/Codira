# 2024-05-24-isa-changelog

This document describes change log with the following modifications:

* Version
* Bytecode instructions

## Version
We update version from 12.0.2.0 to  12.0.3.0

## Bytecode instructions
1. To improve the performance of "istrue" and "isfalse" bytecode instructions, the following two bytecode instructions are added as an improved version with ic slot:
    - callruntime.istrue
    - callruntime.isfalse

2. There is a bug in generating "definefieldbyname" bytecode's ic slot number when ic slot number overflows, which leads to a runtime error. To avoid this, the following bytecode instruction is added to replace the above one:
    - definepropertybyname
