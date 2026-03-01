/*
* Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/*---
desc: The keyword keyof is applied to the class or interface type (see Classes and
  Interfaces). The resultant new type is a union of names (as string literal types)
  of all accessible members of the class or interface type.
name: keyof_special_keywords
---*/

class A {
    any: number
}

interface B {
    any: byte
}

class C {
    Never: short
}

interface D {
    Never: never|float
}

class E {
    Void: null
}

interface F {
    Void: undefined
}

class G {
    Null: null
}

interface H {
    Null: undefined|Byte
}

class I {
    Undefined: Number
}

interface J {
    Undefined: char
}

class K {
    record: Long
}

interface L {
    record: Record<string, number>
}

class M {
    error: Integral = 1
}

interface N {
    error: string | never
}


//'any' first letter lowercase keyword
type KeysOfA = keyof A
type KeysOfB = keyof B
//'Never' first letter uppercase keyword
type KeysOfC = keyof C
type KeysOfD = keyof D
//'Void' first letter uppercase keyword
type KeysOfE = keyof E
type KeysOfF = keyof F
//'Null' first letter uppercase keyword
type KeysOfG = keyof G
type KeysOfH = keyof H
//'Undefined' first letter uppercase keyword
type KeysOfI = keyof I
type KeysOfJ = keyof J
//'record' first letter lowercase keyword
type KeysOfK = keyof K
type KeysOfL = keyof L
//'error' first letter lowercase keyword
type KeysOfM = keyof M
type KeysOfN = keyof N


function main() {
    //Check lowercase first letter keyword 'any' as type from 'keyof' class and interface
    const anyClassVar: KeysOfA = 'any'
    const anyInterfaceVar: KeysOfB = 'any'
    arktest.assertEQ(typeof anyClassVar, "string")
    arktest.assertEQ(typeof anyInterfaceVar, "string")

    //Check uppercase first letter keyword 'Never' as type from 'keyof' class and interface
    const neverClassVar: KeysOfC = 'Never'
    const neverInterfaceVar: KeysOfD = 'Never'
    arktest.assertEQ(typeof neverClassVar, "string")
    arktest.assertEQ(typeof neverInterfaceVar, "string")

    //Check uppercase first letter keyword 'Void' as type from 'keyof' class and interface
    const voidClassVar: KeysOfE = 'Void'
    const voidInterfaceVar: KeysOfF = 'Void'
    arktest.assertEQ(typeof voidClassVar, "string")
    arktest.assertEQ(typeof voidInterfaceVar, "string")

    //Check uppercase first letter keyword 'Null' as type from 'keyof' class and interface
    const nullClassVar: KeysOfG = 'Null'
    const nullInterfaceVar: KeysOfH = 'Null'
    arktest.assertEQ(typeof nullClassVar, "string")
    arktest.assertEQ(typeof nullInterfaceVar, "string")

    //Check uppercase first letter keyword 'Undefined' as type from 'keyof' class and interface
    const undefinedClassVar: KeysOfI = 'Undefined'
    const undefinedInterfaceVar: KeysOfJ = 'Undefined'
    arktest.assertEQ(typeof undefinedClassVar, "string")
    arktest.assertEQ(typeof undefinedInterfaceVar, "string")

    //Check lowercase first letter keyword 'record' as type from 'keyof' class and interface
    const recordClassVar: KeysOfL = 'record'
    const recordInterfaceVar: KeysOfK = 'record'
    arktest.assertEQ(typeof recordClassVar, "string")
    arktest.assertEQ(typeof recordInterfaceVar, "string")

    //Check lowercase first letter keyword 'error' as type from 'keyof' class and interface
    const errorClassVar: KeysOfM = 'error'
    const errorInterfaceVar: KeysOfN = 'error'
    arktest.assertEQ(typeof errorClassVar, "string")
    arktest.assertEQ(typeof errorInterfaceVar, "string")
}
