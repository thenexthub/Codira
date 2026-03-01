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

function testMapSetBool(): void {
    let map: Map<boolean, double> = new Map<boolean, double>()

    map.set(false, 1.5)
    map.set(true, 2.5)

    arktest.assertEQ(map.size, 2)
    arktest.assertTrue(map.has(false))
    arktest.assertTrue(map.has(true))
    arktest.assertEQ(map.get(false), 1.5)
    arktest.assertEQ(map.get(true), 2.5)

    map.set(false, 3.0)
    arktest.assertEQ(map.get(false), 3.0)
}

function testMapSetInt(): void {
    let map: Map<int, string> = new Map<int, string>()

    let size = 1000
    for (let i = 0; i < size; i++) {
        map.set(i, `value_${i}`)
    }

    arktest.assertEQ(map.size, size)

    for (let i = 0; i < size; i++) {
        arktest.assertTrue(map.has(i))
        arktest.assertEQ(map.get(i), `value_${i}`)
    }

    map.set(500, "new_value")
    arktest.assertEQ(map.get(500), "new_value")
}

function testMapSetString(): void {
    let map: Map<string, int> = new Map<string, int>()

    let size = 1000
    for (let i = 0; i < size; i++) {
        map.set(`key_${i}`, i)
    }

    arktest.assertEQ(map.size, size)

    for (let i = 0; i < size; i++) {
        arktest.assertTrue(map.has(`key_${i}`))
        arktest.assertEQ(map.get(`key_${i}`), i)
    }

    map.set("duplicate_key", 1)
    map.set("duplicate_key", 2)
    arktest.assertEQ(map.get("duplicate_key"), 2)
}

class CustomKey {
    id: int
    name: string

    constructor(id: int, name: string) {
        this.id = id
        this.name = name
    }
}

function testMapSetObject(): void {
    let map: Map<CustomKey, string> = new Map<CustomKey, string>()

    let key1 = new CustomKey(1, "key1")
    let key2 = new CustomKey(2, "key2")

    map.set(key1, "value1")
    map.set(key2, "value2")

    arktest.assertEQ(map.size, 2)

    arktest.assertTrue(map.has(key1))
    arktest.assertTrue(map.has(key2))
    arktest.assertEQ(map.get(key1), "value1")
    arktest.assertEQ(map.get(key2), "value2")

    let key1Copy = new CustomKey(1, "key1")
    arktest.assertFalse(map.has(key1Copy))
}

function testMapSetRehashStress(): void {
    let map: Map<int, int> = new Map<int, int>()

    const size = 10000
    for (let i = 0; i < size; i++) {
        map.set(i, i * 2)
    }

    arktest.assertEQ(map.size, size)

    for (let i = 0; i < size; i++) {
        arktest.assertTrue(map.has(i))
        arktest.assertEQ(map.get(i), i * 2)
    }

    for (let i = 0; i < size; i++) {
        map.set(i, i * 3)
    }

    arktest.assertEQ(map.size, size)

    for (let i = 0; i < size; i++) {
        arktest.assertEQ(map.get(i), i * 3)
    }
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite("Map.set tests")
    testSuite.addTest("Map.set bool", testMapSetBool)
    testSuite.addTest("Map.set int (rehash check)", testMapSetInt)
    testSuite.addTest("Map.set string (rehash check)", testMapSetString)
    testSuite.addTest("Map.set object keys", testMapSetObject)
    testSuite.addTest("Map.set rehash stress test", testMapSetRehashStress)
    return testSuite.run()
}
