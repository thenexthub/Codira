# 2024-08-29-isa&&file_format-changelog

This document describes change log with the following modifications:

* Version
* Bytecode instructions
* Header
* LiteralArrayIdx

## Version
Bytecode version is updated from 12.0.6.0 to 13.0.0.0.

## Bytecode instructions
The behavior of default derived constructor has changed since ES2021, refering to [class definition evaluation](https://262.ecma-international.org/11.0/#sec-runtime-semantics-classdefinitionevaluation) chapter in ES2020 and [default constructor functions](https://262.ecma-international.org/12.0/#sec-default-constructor-functions) chapter in ES2021. The following bytecode instruction is added to apply this change:

- callruntime.supercallforwardallargs

## Header
The [literalarrays_size] & [literalarrays_idx_off] in header is set to an invalid value (0xffffffff) since version 13.0.0.0.

## LiteralArrayIdx
The corresponding [literalarrays_idx] offsets-array in file format is deleted since version 13.0.0.0.
