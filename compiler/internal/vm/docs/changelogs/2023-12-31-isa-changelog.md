# 2023-12-31-isa-changelog

This document describes change log with the following modifications:

* Version
* Bytecode instructions

## Version
We update version from 9.0.0.0 to  11.0.2.0

## Bytecode instructions
To support the latest class features in ECMAScript2022 and [sendable class](https://gitcode.com/openharmony/docs/blob/5bce26d25ceed2417ed862817c97c205593c94d5/zh-cn/application-dev/arkts-utils/arkts-sendable.md), 12 bytecode instructions are added.

1. To support public fields which will use define semantic instead of set semantic, the following bytecode instructions are added:
    - definefieldbyname
    - callruntime.definefieldbyvalue
    - callruntime.definefieldbyindex

2. To support private properties in class and access to them, the following bytecode instructions are added:
    - callruntime.createprivateproperty
    - callruntime.defineprivateproperty
    - ldprivateproperty
    - stprivateproperty

3. To support `#x in obj` syntax, the following bytecode instruction is added:
    - testin

4. To calculate computed property name and get a value which can be used as class property key, the following bytecode instruction is added:
    - callruntime.topropertykey

5. To improve the performance of class field initialization, the following bytecode instruction is added:
    - callruntime.callinit

6. To support sendable class, the following bytecode instructions are added:
    - callruntime.definesendableclass
    - callruntime.ldsendableclass
