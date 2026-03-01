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

function testSetBool(): void {
    let set: Set<boolean> = new Set<boolean>

    set.add(false)

    arktest.assertEQ(set.size, 1)
    arktest.assertTrue(set.has(false))

    set.delete(false)
    set.delete(false)
    arktest.assertEQ(set.size, 0)
}

function testSetChar(): void {
    let set: Set<char> = new Set<char>

    set.add(c'L')
    set.add(c'Г')
    set.add(c'你')

    arktest.assertEQ(set.size, 3)
    arktest.assertTrue(set.has(c'L'))
    arktest.assertTrue(set.has(c'Г'))
    arktest.assertTrue(set.has(c'你'))

    set.delete(c'L')
    set.delete(c'Г')
    set.delete(c'你')
    set.delete(c'你')
    arktest.assertEQ(set.size, 0)
}

function testSetByte(): void {
    let set: Set<byte> = new Set<byte>

    set.add((1).toByte())
    set.add((7).toByte())

    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has((1).toByte()))
    arktest.assertTrue(set.has((7).toByte()))

    set.delete((1).toByte())
    set.delete((7).toByte())
    set.delete((7).toByte())
    arktest.assertEQ(set.size, 0)
}

function testSetShort(): void {
    let set: Set<short> = new Set<short>

    set.add((1).toShort())
    set.add((7).toShort())

    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has((1).toShort()))
    arktest.assertTrue(set.has((7).toShort()))

    set.delete((1).toShort())
    set.delete((7).toShort())
    set.delete((7).toShort())
    arktest.assertEQ(set.size, 0)
}

function testSetInt(): void {
    let set: Set<int> = new Set<int>

    set.add(1)
    set.add(7)

    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has(1))
    arktest.assertTrue(set.has(7))

    set.delete(1)
    set.delete(7)
    set.delete(7)
    arktest.assertEQ(set.size, 0)
}

function testSetLong(): void {
    let set: Set<long> = new Set<long>

    set.add(1)
    set.add(7)

    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has(1))
    arktest.assertTrue(set.has(7))

    set.delete(1)
    set.delete(7)
    set.delete(7)
    arktest.assertEQ(set.size, 0)
}

function testSetFloat(): void {
    let set: Set<float> = new Set<float>

    set.add(1)
    set.add(7.2f)

    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has(1))
    arktest.assertTrue(set.has(7.2f))

    set.delete(1)
    set.delete(7.2f)
    set.delete(7.2f)
    arktest.assertEQ(set.size, 0)
}

function testSetDouble(): void {
    let set: Set<double> = new Set<double>

    set.add(1)
    set.add(7.2)

    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has(1))
    arktest.assertTrue(set.has(7.2))

    set.delete(1)
    set.delete(7.2)
    set.delete(7.2)
    arktest.assertEQ(set.size, 0)
}

function testSetNumber(): void {
    let set: Set<double> = new Set<double>

    set.add(0)
    set.add(1)
    set.add(7.2)

    arktest.assertEQ(set.size, 3)
    arktest.assertTrue(set.has(0))
    arktest.assertTrue(set.has(1))
    arktest.assertTrue(set.has(7.2))

    set.delete(0)
    set.delete(1)
    set.delete(7.2)
    arktest.assertEQ(set.size, 0)
}

function testSetString(): void {
    let set: Set<string> = new Set<string>

    set.add("")
    set.add("abc")
    set.add("defgh")

    arktest.assertEQ(set.size, 3)
    arktest.assertTrue(set.has(""))
    arktest.assertTrue(set.has("abc"))
    arktest.assertTrue(set.has("defgh"))

    set.delete("")
    set.delete("abc")
    set.delete("defgh")
    set.delete("defgh")
    arktest.assertEQ(set.size, 0)
}

class ObjKey {
    m_k1: int
    m_k2: int

    constructor(k1: int, k2: int) {
        this.m_k1 = k1
        this.m_k2 = k2
    }
}

function testSetObj(): void {
    let set: Set<ObjKey> = new Set<ObjKey>
    let k1_1: ObjKey = new ObjKey(5, 6)
    let k1_2: ObjKey = new ObjKey(5, 6)
    let k2: ObjKey = new ObjKey(5, 8)
    let k3: ObjKey = new ObjKey(3, 9)

    set.add(k1_1)
    set.add(k2)

    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has(k1_1))
    arktest.assertTrue(set.has(k2))

    set.delete(k1_1)
    set.delete(k2)
    set.delete(k2)
    arktest.assertEQ(set.size, 0)
}

function testSetSpec(): void {
    const set: Set<Object | null | undefined> = new Set<Object | null | undefined>;

    set.add(null)

    arktest.assertEQ(set.size, 1)
    arktest.assertTrue(set.has(null))

    set.add(undefined)

    arktest.assertEQ(set.size, 2)
    arktest.assertTrue(set.has(null))

    set.delete(null);

    arktest.assertEQ(set.size, 1)
    arktest.assertTrue(set.has(undefined))

    set.delete(undefined);
    set.delete(undefined);

    arktest.assertEQ(set.size, 0)
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("Set");
    testSuite.addTest("Set: test bool", testSetBool);
    testSuite.addTest("Set: test char", testSetChar);
    testSuite.addTest("Set: test byte", testSetByte);
    testSuite.addTest("Set: test short", testSetShort);
    testSuite.addTest("Set: test int", testSetInt);
    testSuite.addTest("Set: test long", testSetLong);
    testSuite.addTest("Set: test float", testSetFloat);
    testSuite.addTest("Set: test double", testSetDouble);
    testSuite.addTest("Set: test number", testSetNumber);
    testSuite.addTest("Set: test string", testSetString);
    testSuite.addTest("Set: test Obj", testSetObj);
    testSuite.addTest("Set: test Spec", testSetSpec);
    return testSuite.run()
}
