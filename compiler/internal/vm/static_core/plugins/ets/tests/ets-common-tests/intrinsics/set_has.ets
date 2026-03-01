/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

function testSetHasBool(): void {
    let setInstance: Set<boolean> = new Set<boolean>()

    setInstance.add(false)

    arktest.assertTrue(setInstance.has(false))

    arktest.assertFalse(setInstance.has(true))
}

function testSetHasChar(): void {
    let setInstance: Set<char> = new Set<char>()

    setInstance.add(c'L')
    setInstance.add(c'Г')
    setInstance.add(c'你')

    arktest.assertTrue(setInstance.has(c'L'))
    arktest.assertTrue(setInstance.has(c'Г'))
    arktest.assertTrue(setInstance.has(c'你'))

    arktest.assertFalse(setInstance.has(c'D'))
    arktest.assertFalse(setInstance.has(c'Ъ'))
}

function testSetHasByte(): void {
    let setInstance: Set<byte> = new Set<byte>()

    setInstance.add((1).toByte())
    setInstance.add((7).toByte())

    arktest.assertTrue(setInstance.has((1).toByte()))
    arktest.assertTrue(setInstance.has((7).toByte()))

    arktest.assertFalse(setInstance.has((2).toByte()))
}

function testSetHasShort(): void {
    let setInstance: Set<short> = new Set<short>()

    setInstance.add((1).toShort())
    setInstance.add((7).toShort())

    arktest.assertTrue(setInstance.has((1).toShort()))
    arktest.assertTrue(setInstance.has((7).toShort()))

    arktest.assertFalse(setInstance.has((2).toShort()))
}

function testSetHasInt(): void {
    let setInstance: Set<int> = new Set<int>()
    setInstance.add(1)
    setInstance.add(7)

    arktest.assertTrue(setInstance.has(1))
    arktest.assertTrue(setInstance.has(1))
    arktest.assertTrue(setInstance.has(7))

    arktest.assertFalse(setInstance.has(2))
}

function testSetHasLong(): void {
    let setInstance: Set<long> = new Set<long>()

    setInstance.add(1250999896455) // 0x12345678987
    setInstance.add(8131499327779) // 0x76543210123

    arktest.assertTrue(setInstance.has(1250999896455))
    arktest.assertTrue(setInstance.has(8131499327779))

    arktest.assertFalse(setInstance.has(2222222222222))
}

function testSetHasBigint(): void {
    let setInstance: Set<bigint> = new Set<bigint>()

    let key_1: bigint = 10000000000n
    let key_2: bigint = 70000000000n

    setInstance.add(key_1)
    setInstance.add(key_2)

    arktest.assertTrue(setInstance.size == 2)
    arktest.assertTrue(setInstance.has(key_1))
    arktest.assertTrue(setInstance.has(key_2))
    arktest.assertFalse(setInstance.has(20000000000n))
    arktest.assertTrue(setInstance.has(key_1))
    arktest.assertTrue(setInstance.has(key_2))
    arktest.assertEQ(setInstance.has(20000000000n), false)

    setInstance.delete(key_1)
    setInstance.delete(key_2)
    arktest.assertEQ(setInstance.size, 0)
}

function testSetHasFloat(): void {
    let setInstance: Set<float> = new Set<float>()

    setInstance.add(1)
    setInstance.add(7.2f)

    arktest.assertTrue(setInstance.has(1))
    arktest.assertTrue(setInstance.has(7.2f))

    arktest.assertFalse(setInstance.has(2))
    arktest.assertFalse(setInstance.has(2.5f))
}

function testSetHasDouble(): void {
    let setInstance: Set<double> = new Set<double>()

    setInstance.add(1)
    setInstance.add(7.2)

    arktest.assertTrue(setInstance.has(1))
    arktest.assertTrue(setInstance.has(7.2))

    arktest.assertFalse(setInstance.has(2))
    arktest.assertFalse(setInstance.has(2.5))
}

function testSetHasNumber(): void {
    let setInstance: Set<double> = new Set<double>()

    setInstance.add(0)
    setInstance.add(1)
    setInstance.add(7.2)

    arktest.assertTrue(setInstance.has(0))
    arktest.assertTrue(setInstance.has(1))
    arktest.assertTrue(setInstance.has(7.2))

    arktest.assertFalse(setInstance.has(2))
    arktest.assertFalse(setInstance.has(2.5))
}

function testSetHasString(): void {
    let setInstance: Set<string> = new Set<string>()

    arktest.assertFalse(setInstance.has(""))

    setInstance.add("")
    setInstance.add("abc")
    setInstance.add("defgh")

    arktest.assertTrue(setInstance.has(""))
    arktest.assertTrue(setInstance.has("abc"))
    arktest.assertTrue(setInstance.has("defgh"))

    arktest.assertFalse(setInstance.has("erty"))
}

class ObjKey {
    m_k1: int
    m_k2: int

    constructor(k1: int, k2: int) {
        this.m_k1 = k1
        this.m_k2 = k2
    }
}

function testSetHasObj(): void {
    let setInstance: Set<ObjKey> = new Set<ObjKey>()
    let k1_1: ObjKey = new ObjKey(5, 6)
    let k1_2: ObjKey = new ObjKey(5, 6)
    let k2: ObjKey = new ObjKey(5, 8)
    let k3: ObjKey = new ObjKey(3, 9)

    setInstance.add(k1_1)
    setInstance.add(k2)

    arktest.assertTrue(setInstance.has(k1_1))
    arktest.assertFalse(setInstance.has(k1_2))
    arktest.assertTrue(setInstance.has(k2))
    arktest.assertFalse(setInstance.has(k3))
}

function testSetHasSpec(): void {
    const setInstance: Set<Object | null | undefined> = new Set<Object | null | undefined>();

    arktest.assertFalse(setInstance.has(null))
    arktest.assertFalse(setInstance.has(undefined))

    setInstance.add(null);

    arktest.assertTrue(setInstance.has(null))
    arktest.assertFalse(setInstance.has(undefined))

    setInstance.add(undefined);

    arktest.assertTrue(setInstance.has(null))
    arktest.assertTrue(setInstance.has(undefined))

    setInstance.delete(null);

    arktest.assertFalse(setInstance.has(null))
    arktest.assertTrue(setInstance.has(undefined))

    setInstance.delete(undefined);

    arktest.assertFalse(setInstance.has(null))
    arktest.assertFalse(setInstance.has(undefined))
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("Set");
    testSuite.addTest("has test bool", testSetHasBool);
    testSuite.addTest("has test char", testSetHasChar);
    testSuite.addTest("has test byte", testSetHasByte);
    testSuite.addTest("has test short", testSetHasShort);
    testSuite.addTest("has test int", testSetHasInt);
    testSuite.addTest("has test long", testSetHasLong);
    testSuite.addTest("has test bigint", testSetHasBigint);
    testSuite.addTest("has test float", testSetHasFloat);
    testSuite.addTest("has test double", testSetHasDouble);
    testSuite.addTest("has test number", testSetHasNumber);
    testSuite.addTest("has test string", testSetHasString);
    testSuite.addTest("has test Obj", testSetHasObj);
    testSuite.addTest("has test Spec", testSetHasSpec);
    return testSuite.run()
}
