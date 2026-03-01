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

function testMapGetBool(): void {
    let map: Map<boolean, double> = new Map<boolean, double>()

    map.set(false, 1.5)

    arktest.assertEQ(map.size, 1)
    arktest.assertTrue(map.has(false))
    arktest.assertEQ(map.get(false), 1.5)
    arktest.assertEQ(map.get(true), undefined)

    map.delete(false)
    arktest.assertEQ(map.size, 0)
}

function testMapGetChar(): void {
    let map: Map<char, double> = new Map<char, double>()

    map.set(c'L', 1.5)
    map.set(c'Г', 1.5)
    map.set(c'你', 7.5)

    arktest.assertEQ(map.size, 3)
    arktest.assertTrue(map.has(c'Г'))
    arktest.assertTrue(map.has(c'L'))
    arktest.assertTrue(map.has(c'你'))
    arktest.assertEQ(map.get(c'L'), 1.5)
    arktest.assertEQ(map.get(c'Г'), 1.5)
    arktest.assertEQ(map.get(c'你'), 7.5)
    arktest.assertEQ(map.get(c'D'), undefined)
    arktest.assertEQ(map.get(c'Ъ'), undefined)

    map.delete(c'L')
    map.delete(c'Г')
    map.delete(c'你')
    arktest.assertEQ(map.size, 0)
}

function testMapGetByte(): void {
    let map: Map<byte, double> = new Map<byte, double>()

    map.set((1).toByte(), 1.5)
    map.set((7).toByte(), 7.5)

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has((1).toByte()))
    arktest.assertTrue(map.has((7).toByte()))
    arktest.assertEQ(map.get((1).toByte()), 1.5)
    arktest.assertEQ(map.get((7).toByte()), 7.5)
    arktest.assertEQ(map.get((2).toByte()), undefined)

    map.delete((1).toByte())
    map.delete((7).toByte())
    arktest.assertEQ(map.size, 0)
}

function testMapGetShort(): void {
    let map: Map<short, double> = new Map<short, double>()

    map.set((1).toShort(), 1.5)
    map.set((7).toShort(), 7.5)

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has((1).toShort()))
    arktest.assertTrue(map.has((7).toShort()))
    arktest.assertEQ(map.get((1).toShort()), 1.5)
    arktest.assertEQ(map.get((7).toShort()), 7.5)
    arktest.assertEQ(map.get((2).toShort()), undefined)

    map.delete((1).toShort())
    map.delete((7).toShort())
    arktest.assertEQ(map.size, 0)
}

function testMapGetInt(): void {
    let map: Map<int, double> = new Map<int, double>()

    map.set(1, 1.5)
    map.set(7, 7.5)

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has(1))
    arktest.assertTrue(map.has(7))
    arktest.assertEQ(map.get(1), 1.5)
    arktest.assertEQ(map.get(7), 7.5)
    arktest.assertEQ(map.get(2), undefined)

    map.delete(1)
    map.delete(7)
    arktest.assertEQ(map.size, 0)
}

function testMapGetLong(): void {
    let map: Map<long, double> = new Map<long, double>()

    map.set(1, 1.5)
    map.set(7, 7.5)

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has(1))
    arktest.assertTrue(map.has(7))
    arktest.assertEQ(map.get(1), 1.5)
    arktest.assertEQ(map.get(7), 7.5)
    arktest.assertEQ(map.get(2), undefined)

    map.delete(1)
    map.delete(7)
    arktest.assertEQ(map.size, 0)
}

function testMapGetFloat(): void {
    let map: Map<float, double> = new Map<float, double>()

    map.set(1, 1.5)
    map.set(7.2f, 7.5)

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has(1))
    arktest.assertTrue(map.has(7.2f))
    arktest.assertEQ(map.get(1), 1.5)
    arktest.assertEQ(map.get(7.2f), 7.5)
    arktest.assertEQ(map.get(2), undefined)
    arktest.assertEQ(map.get(2.5f), undefined)

    map.delete(1)
    map.delete(7.2f)
    arktest.assertEQ(map.size, 0)
}

function testMapGetDouble(): void {
    let map: Map<double, double> = new Map<double, double>()

    map.set(1, 1.5)
    map.set(7.2, 7.5)

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has(1))
    arktest.assertTrue(map.has(7.2))
    arktest.assertEQ(map.get(1), 1.5)
    arktest.assertEQ(map.get(7.2), 7.5)
    arktest.assertEQ(map.get(2), undefined)
    arktest.assertEQ(map.get(2.5), undefined)

    map.delete(1)
    map.delete(7.2)
    arktest.assertEQ(map.size, 0)
}

function testMapGetNumber(): void {
    let map: Map<double, number> = new Map<double, number>()

    map.set(0, 0.5)
    map.set(1, 1.5)
    map.set(7.2, 7.5)

    arktest.assertEQ(map.size, 3)
    arktest.assertTrue(map.has(0))
    arktest.assertTrue(map.has(1))
    arktest.assertTrue(map.has(7.2))
    arktest.assertEQ(map.get(0), 0.5)
    arktest.assertEQ(map.get(1), 1.5)
    arktest.assertEQ(map.get(7.2), 7.5)
    arktest.assertEQ(map.get(2), undefined)
    arktest.assertEQ(map.get(2.5), undefined)

    map.delete(0)
    map.delete(1)
    map.delete(7.2)
    arktest.assertEQ(map.size, 0)
}

function testMapGetString(): void {
    let map: Map<string, number> = new Map<string, number>()

    arktest.assertEQ(map.get(""), undefined)

    map.set("", 0.5)
    map.set("abc", 0.5)
    map.set("defgh", 1.5)

    arktest.assertEQ(map.size, 3)
    arktest.assertTrue(map.has(""))
    arktest.assertTrue(map.has("abc"))
    arktest.assertTrue(map.has("defgh"))
    arktest.assertEQ(map.get(""), 0.5)
    arktest.assertEQ(map.get("abc"), 0.5)
    arktest.assertEQ(map.get("defgh"), 1.5)
    arktest.assertEQ(map.get("erty"), undefined)

    map.delete("")
    map.delete("abc")
    map.delete("defgh")
    arktest.assertEQ(map.size, 0)
}

class ObjKey {
    m_k1: int
    m_k2: int

    constructor(k1: int, k2: int) {
        this.m_k1 = k1
        this.m_k2 = k2
    }
}

function testMapGetObj(): void {
    let map: Map<ObjKey, string> = new Map<ObjKey, string>()
    let k1_1: ObjKey = new ObjKey(5, 6)
    let k1_2: ObjKey = new ObjKey(5, 6)
    let k2: ObjKey = new ObjKey(5, 8)
    let k3: ObjKey = new ObjKey(3, 9)

    map.set(k1_1, "k1_1")
    map.set(k2, "k2")

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has(k1_1))
    arktest.assertEQ(map.get(k1_1), "k1_1")
    arktest.assertEQ(map.get(k1_2), undefined)
    arktest.assertTrue(map.has(k2))
    arktest.assertEQ(map.get(k2), "k2")
    arktest.assertEQ(map.get(k3), undefined)

    map.delete(k1_1)
    map.delete(k2)
    arktest.assertEQ(map.size, 0)
}

function testMapGetSpec(): void {
    const map: Map<Object | null | undefined, string> = new Map<Object | null | undefined, string>();

    arktest.assertEQ(map.get(null), undefined)
    arktest.assertEQ(map.get(undefined), undefined)

    map.set(null, "null");

    arktest.assertEQ(map.size, 1)
    arktest.assertTrue(map.has(null))
    arktest.assertEQ(map.get(null), "null")
    arktest.assertEQ(map.get(undefined), undefined)

    map.set(undefined, "undefined");

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has(null))
    arktest.assertEQ(map.get(null), "null")
    arktest.assertEQ(map.get(undefined), "undefined")

    map.delete(null);

    arktest.assertEQ(map.size, 1)
    arktest.assertTrue(map.has(undefined))
    arktest.assertEQ(map.get(null), undefined)
    arktest.assertEQ(map.get(undefined), "undefined")

    map.delete(undefined);

    arktest.assertEQ(map.size, 0)
    arktest.assertEQ(map.get(null), undefined)
    arktest.assertEQ(map.get(undefined), undefined)
}

function testMapSpecialNumbers(): void {
    let map: Map<double, string> = new Map<double, string>()

    const specialNumbers = [
        Number.NaN,
        Number.POSITIVE_INFINITY,
        Number.NEGATIVE_INFINITY,
        Number.MAX_VALUE,
        Number.MIN_VALUE,
    ]

    specialNumbers.forEach((num, i) => {
        map.set(num, `value_${i}`)
    })

    specialNumbers.forEach((num, i) => {
        arktest.assertTrue(map.has(num))
        arktest.assertEQ(map.get(num), `value_${i}`)
    })

    arktest.assertTrue(map.has(Number.NaN))
    arktest.assertEQ(map.get(Number.NaN), "value_0")

    arktest.assertEQ(map.size, specialNumbers.length)
}

function testMapTypeStrictness(): void {
    let intMap: Map<int, string> = new Map<int, string>()
    let doubleMap: Map<double, string> = new Map<double, string>()

    intMap.set(5, "int_value")
    doubleMap.set(5.0, "double_value")

    arktest.assertEQ(intMap.get(5), "int_value")
    arktest.assertEQ(doubleMap.get(5.0), "double_value")
    arktest.assertEQ(intMap.get((5.0).toInt()), "int_value")
    arktest.assertEQ(doubleMap.get((5).toDouble()), "double_value")
    arktest.assertTrue(intMap.has((5.1).toInt()))
    arktest.assertTrue(doubleMap.has((5).toDouble()))
}

class Key {
    id: int
    constructor(id: int) {
        this.id = id
    }
}

function testMapObjectIdentity(): void {
    let map: Map<Key, string> = new Map<Key, string>()
    let key1 = new Key(1)
    let key2 = new Key(1)
    map.set(key1, "value1")

    arktest.assertTrue(map.has(key1))
    arktest.assertFalse(map.has(key2))
    arktest.assertEQ(map.get(key2), undefined)
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("Map");
    testSuite.addTest("Map test bool", testMapGetBool);
    testSuite.addTest("Map test char", testMapGetChar);
    testSuite.addTest("Map test byte", testMapGetByte);
    testSuite.addTest("Map test short", testMapGetShort);
    testSuite.addTest("Map test int", testMapGetInt);
    testSuite.addTest("Map test long", testMapGetLong);
    testSuite.addTest("Map test float", testMapGetFloat);
    testSuite.addTest("Map test double", testMapGetDouble);
    testSuite.addTest("Map test number", testMapGetNumber);
    testSuite.addTest("Map test string", testMapGetString);
    testSuite.addTest("Map test Obj", testMapGetObj);
    testSuite.addTest("Map test Spec", testMapGetSpec);
    testSuite.addTest("Map test special numbers handling", testMapSpecialNumbers)
    testSuite.addTest("Map test type strictness", testMapTypeStrictness)
    testSuite.addTest("Map test object identity", testMapObjectIdentity)
    return testSuite.run()
}
