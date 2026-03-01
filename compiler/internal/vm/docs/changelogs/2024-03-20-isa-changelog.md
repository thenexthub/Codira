# 2024-03-20-isa-changelog

This document describes change log with the following modifications:

* Version
* Bytecode instructions

## Version
We update version from 11.0.2.0 to  12.0.0.0

## Bytecode instructions
To support lazy loading of module variables in sendable class, the following bytecode instructions are added:

- callruntime.ldsendableexternalmodulevar
- callruntime.wideldsendableexternalmodulevar