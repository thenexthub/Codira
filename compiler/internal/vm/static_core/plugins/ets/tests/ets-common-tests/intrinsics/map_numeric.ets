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

type MapType = Map<Numeric, string>

function getMap(): MapType {
    let map: MapType = new MapType
    map.set(7.0, "A")
    return map
}

function testMapHas() {
    let map: MapType = getMap()
    // NOTE(dkofanov): #28334 irtoc intrinsic broken for integers other than 'int'
    // arktest.assertTrue(map.has((7).toByte()), "map.has((7).toByte())")
    arktest.assertTrue(map.has(7.0), "map.has(7.0)")
}

function testMapGet() {
    let map: MapType = getMap()
    // NOTE(dkofanov): #28334 irtoc intrinsic broken for integers other than 'int'
    // arktest.assertTrue(map.get((7).toByte()) == "A", "map.get((7).toByte())")
    arktest.assertTrue(map.get(7.0) == "A", "map.get(7.0)")
}

function testMapDelete() {
    // NOTE(dkofanov): #28334 irtoc intrinsic broken for integers other than 'int'
    // let map_1: MapType = getMap()
    // map_1.delete((7).toByte())
    // arktest.assertTrue(map_1.size == 0, "map.delete((7).toByte())")

    let map_2: MapType = getMap()
    map_2.delete(7.0)
    arktest.assertTrue(map_2.size == 0, "map.delete(7.0)")
}

type SetType = Set<Numeric>

function getSet(): SetType {
    let set: SetType = new SetType
    set.add(7.0)
    return set
}

function testSetHas() {
    let set: SetType = getSet()
    // NOTE(dkofanov): #28334 irtoc intrinsic broken for integers other than 'int'
    // arktest.assertTrue(set.has((7).toByte()), "set.has((7).toByte())")
    arktest.assertTrue(set.has(7.0), "set.has(7.0)")
}

function testSetDelete() {
    // NOTE(dkofanov): #28334 irtoc intrinsic broken for integers other than 'int'
    // let set_1: SetType = getSet()
    // set_1.delete((7).toByte())
    // arktest.assertTrue(set_1.size == 0, "set.delete((7).toByte())")

    let set_2: SetType = getSet()
    set_2.delete(7.0)
    arktest.assertTrue(set_2.size == 0, "set.delete(7.0)")
}

class A<T> {
    f: T
    constructor(p: T) {
        this.f = p
    }
    put(p: T) {
        this.f = p
    }
}

class B<T> extends A<T> {
    constructor(p: T) {
        super(p)
    }
    meth(): T {
        return this.f
    }
}

function testRecord() {
    let b: B<Record<Numeric, string>> = new B<Record<Numeric, string>>({
        12345.0: "1 2 3 4 5",
        67890.0: "6 7 8 9 0",
    });
    // NOTE(dkofanov): #28334 irtoc intrinsic broken for integers other than 'int'
    //arktest.assertTrue(b.meth()[(12345).toShort()] == "1 2 3 4 5")
    arktest.assertTrue(b.meth()[67890] == "6 7 8 9 0")
    // NOTE(dkofanov): #28334 irtoc intrinsic broken for integers other than 'int'
    //arktest.assertTrue(b.meth()[(11111).toShort()] == undefined)
    b.put({ 1.0: "A", 2.0: "B" })
    // NOTE(dkofanov): #28334 irtoc intrinsic broken for integers other than 'int'
    //arktest.assertTrue(b.meth()[(1).toByte()] == "A")
    //arktest.assertTrue(b.meth()[(2).toByte()] == "B")
}

function main() {
    let testSuite = new arktest.ArkTestsuite("Map/Set/Record 7/7.0");
    testSuite.addTest("Map test has", testMapHas);
    testSuite.addTest("Map test get", testMapGet);
    testSuite.addTest("Map test delete", testMapDelete);
    testSuite.addTest("Set test has", testSetHas);
    testSuite.addTest("Set test delete", testSetDelete);
    testSuite.addTest("Record test", testRecord);
    return testSuite.run()
}
