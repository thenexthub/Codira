/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License")
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

type KeyType = Number
type ValType = String

type TestMap = Map<KeyType, ValType>
type InitArray = Array<[KeyType, ValType]>

function print(str: string) {
    console.log(str)
}

function print(map: TestMap) {
    print(map.toString())
}

function buildVal(key: KeyType, suffix: string = ""): ValType {
    return "v" + key.toString() + suffix
}

function setMap(map: TestMap, from: int, to: int, suffix: string = "") {
    for (let key = from; key <= to; ++key) {
        map.set(key, buildVal(key))
    }        
}

function setArray(arr: InitArray, from: int, to: int, suffix: string = "") {
    for (let key = from; key <= to; ++key) {
        arr.push([key, buildVal(key)])
    }        
}

function check(key: KeyType, val: ValType): boolean {
    return val === buildVal(key);
}

////////////////////////////////////////////////////////////////

function testClearInForEach_0(from: int, to: int) {
    let map = new TestMap()
    setMap(map, from, to)
    let cnt: int = 0
    map.forEach((v, k, map) => {
        ++cnt
        map.clear()
    });
    arktest.assertEQ(cnt, (to >= from ? 1 : 0), "ClearInForEach_0(" + from + ", " + to + ")")
}

function testClearInForEach() {
    testClearInForEach_0(0, -1)
    testClearInForEach_0(0, 0)
    testClearInForEach_0(0, 1)
    testClearInForEach_0(0, 3)
    testClearInForEach_0(0, 5)
    testClearInForEach_0(1, 5)
    testClearInForEach_0(0, 555)
}

////////////////////////////////////////////////////////////////

function testSetAfterClearInForEach_0(clear_inside: boolean, step: int, init_from: int, init_to: int, inside_from: int, inside_to: int, qty: int) {
    let map = new TestMap()
    setMap(map, init_from, init_to)
    let cnt: int = 0
    map.forEach((v, k, map) => {
        ++cnt
        if (!clear_inside) {
            map.clear()
        }
        if (cnt === step) {
            if (clear_inside) {
                map.clear()
            }
            setMap(map, inside_from, inside_to, "_")
        }
    });
    arktest.assertEQ(cnt, qty, "SetAfterClearInForEach_0(" + clear_inside + ", " + step + ", " +
        init_from + ", " + init_to + ", " + inside_from + ", " + inside_to + ", " + qty + ")")
}

function testSetAfterClearInForEach_1(step: int, initial_capacity: int, init_from: int, init_to: int, inside_from: int, inside_to: int, qty: int) {
    let map = new TestMap(initial_capacity)
    setMap(map, init_from, init_to)
    let cnt: int = 0
    map.forEach((v, k, map) => {
        ++cnt
        if (cnt === step) {
            map.clear()
            setMap(map, inside_from, inside_to, "_")
        }
    });
    arktest.assertEQ(cnt, qty, "SetAfterClearInForEach_1(" + step + ", " + initial_capacity + ", " +
        init_from + ", " + init_to + ", " + inside_from + ", " + inside_to + ", " + qty + ")")
}

function testSetAfterClearInForEach() {
    testSetAfterClearInForEach_0(false, 1, 0, 3, 1, 1, 2)
    testSetAfterClearInForEach_0(false, 1, 0, 3, 1, 5, 2)
    testSetAfterClearInForEach_0(false, 1, 0, 3, 1, 555, 2)
    testSetAfterClearInForEach_0(true, 2, 0, 3, 1, 1, 3)
    testSetAfterClearInForEach_0(true, 2, 0, 3, 1, 55, 57)
    testSetAfterClearInForEach_1(10, 0, 0, 35, 101, 350, 260)
    testSetAfterClearInForEach_1(20, 175, 0, 35, 201, 550, 370)
    testSetAfterClearInForEach_1(30, 601, 0, 35, 301, 750, 480)
}

////////////////////////////////////////////////////////////////

function testClearIterator_0(from: int, to: int): void {
    let map = new TestMap()
    setMap(map, from, to)
    let cnt: int = 0
    let iter = map.keys()
    for (const k of iter) {
        ++cnt
        map.clear()
    }
    arktest.assertEQ(cnt, (to >= from ? 1 : 0), "ClearIterator_0(" + from + ", " + to + ")")
}

function testClearIterator() {
    testClearIterator_0(0, -1)
    testClearIterator_0(0, 0)
    testClearIterator_0(0, 1)
    testClearIterator_0(0, 3)
    testClearIterator_0(0, 5)
    testClearIterator_0(1, 5)
    testClearIterator_0(0, 555)
}

////////////////////////////////////////////////////////////////

function testDeleteInForEach_0(from: int, to: int, step: int, key: KeyType, qty: int): void {
    let map = new TestMap()
    setMap(map, from, to)
    let cnt: int = 0
    map.forEach((v, k, map) => {
        ++cnt
        if (cnt === step) {
            map.delete(key)
        }
    });
    arktest.assertEQ(cnt, qty, "DeleteInForEach_0(" + from + ", " + to + ", " + step + ", " + key + ", " + qty + ")")
}

function testDeleteInForEach() {
    testDeleteInForEach_0(0, 3, 1, 0, 4)
    testDeleteInForEach_0(0, 33, 1, 0, 34)
    testDeleteInForEach_0(0, 3, 4, 3, 4)
    testDeleteInForEach_0(0, 3, 1, 2, 3)
    testDeleteInForEach_0(0, 3, 2, 0, 4)
}

////////////////////////////////////////////////////////////////

function testDeleteInIterator_0(from: int, to: int, step: int, key: KeyType, qty: int): void {
    let map = new TestMap()
    setMap(map, from, to)
    let cnt: int = 0
    let iter = map.keys()
    for (const k of iter) {
        ++cnt
        if (cnt === step) {
            map.delete(key)
        }
    }
    arktest.assertEQ(cnt, qty, "DeleteInIterator_0(" + from + ", " + to + ", " + step + ", " + key + ", " + qty + ")")
}

function testDeleteInIterator() {
    testDeleteInIterator_0(0, 3, 1, 0, 4)
    testDeleteInIterator_0(0, 33, 1, 0, 34)
    testDeleteInIterator_0(0, 3, 4, 3, 4)
}

////////////////////////////////////////////////////////////////

function testSetInForEach_0(init_from: int, init_to: int, step: int, inside_from: int, inside_to: int, qty: int): void {
    let map = new TestMap()
    setMap(map, init_from, init_to)
    let cnt: int = 0
    map.forEach((v, k, map) => {
        ++cnt
        if (cnt === step) {
            setMap(map, inside_from, inside_to, "_")
        }
    });
    arktest.assertEQ(cnt, qty, "SetInForEach_0(" + init_from + ", " + init_to + ", " +
        step + ", " + inside_from + ", " + inside_to + ", " + qty + ")")
}

function testSetInForEach() {
    testSetInForEach_0(0, 2, 1, 4, 4, 4)
    testSetInForEach_0(0, 2, 3, 3, 3, 4)
    testSetInForEach_0(0, 2, 3, 1, 2, 3)
}

////////////////////////////////////////////////////////////////

function testSetInIterator_0(init_from: int, init_to: int, step: int, inside_from: int, inside_to: int, qty: int): void {
    let map = new TestMap()
    setMap(map, init_from, init_to)
    let cnt: int = 0
    let iter = map.keys()
    for (const k of iter) {
        ++cnt
        if (cnt === step) {
            setMap(map, inside_from, inside_to, "_")
        }
    }
    arktest.assertEQ(cnt, qty, "SetInIterator_0(" + init_from + ", " + init_to + ", " +
        step + ", " + inside_from + ", " + inside_to + ", " + qty + ")")
}

function testSetInIterator() {
    testSetInIterator_0(0, 2, 1, 4, 4, 4)
    testSetInIterator_0(0, 2, 3, 3, 3, 4)
    testSetInIterator_0(0, 2, 3, 1, 2, 3)
}

////////////////////////////////////////////////////////////////

function testMultipleDeleteForEach_0(init_qty: int, step: int, res_qty: int): void {
    let map = new TestMap()
    setMap(map, 1, init_qty)
    let cnt: int = 0
    map.forEach((v, k, map) => {
        ++cnt
        if (cnt <= step) {
            arktest.assertEQ(k, cnt) 
            arktest.assertEQ(v, buildVal(k))
        }
        
        if (cnt === step) {
            for (let kk: int = 1; kk <= init_qty; ++kk) {
                if (kk % 100 != 0)
                    map.delete(kk)
            }
        }

        if (cnt > step) {
            arktest.assertEQ(k % 100, 0) 
            arktest.assertEQ(v, buildVal(k)) 
        }
    });
    arktest.assertEQ(cnt, res_qty, "testMultipleDeleteForEach_0(" + init_qty + ", " + step + ", " + res_qty + ")")
}

function testMultipleDeleteForEach() {
    testMultipleDeleteForEach_0(1000, 100, 100 + 9)
    testMultipleDeleteForEach_0(1000, 99, 99 + 10)
    testMultipleDeleteForEach_0(1000, 101, 101 + 9)
}
////////////////////////////////////////////////////////////////

function testMultipleDeleteAndSetForEach_0(init_qty: int, step: int, initial_capacity: int, res_qty: int): void {
    let arr = new InitArray()
    setArray(arr, 1, initial_capacity)
    let map = new TestMap(arr)
    setMap(map, initial_capacity + 1, init_qty)
    let cnt: int = 0
    map.forEach((v, k, map) => {
        ++cnt
        if (cnt <= step) {
            arktest.assertEQ(k, cnt)
            arktest.assertEQ(v, buildVal(k))
        }

        if (cnt === step) {
            for (let kk: int = 1; kk <= init_qty; ++kk) {
                if (kk % 50 != 0)
                    map.delete(kk)
                else
                    map.set(init_qty + kk, buildVal(init_qty + kk))
            }
        }

        if (cnt > step) {
            arktest.assertEQ(k % 50, 0)
            arktest.assertEQ(v, buildVal(k))
        }
    });
    arktest.assertEQ(cnt, res_qty, "testMultipleDeleteAndSetForEach_0(" + init_qty + ", " + step + ", " +
        initial_capacity + ", " + res_qty + ")")
}

function testMultipleDeleteAndSetForEach() {
    testMultipleDeleteAndSetForEach_0(1000, 100, 0, 100 + 38)
    testMultipleDeleteAndSetForEach_0(1000, 99, 125, 99 + 39)
    testMultipleDeleteAndSetForEach_0(1000, 101, 500, 101 + 38)
}

////////////////////////////////////////////////////////////////

function testSetAfterEnd() {
    let map1 = new TestMap()
    map1.set(1, buildVal(1))
    let iter1 = map1.keys()
    let next1_1 = iter1.next()
    arktest.assertEQ(next1_1.value, 1.0) 
    arktest.assertFalse(next1_1.done) 
    map1.set(2, buildVal(2))
    let next1_2 = iter1.next()
    arktest.assertEQ(next1_2.value, 2.0) 
    arktest.assertFalse(next1_2.done) 
    let next1_3 = iter1.next()
    arktest.assertEQ(next1_3.value, undefined) 
    arktest.assertTrue(next1_3.done) 

    let map2 = new TestMap()
    map2.set(1, buildVal(1))
    let iter2 = map2.keys()
    let next2_1 = iter2.next()
    arktest.assertEQ(next2_1.value, 1.0) 
    arktest.assertFalse(next2_1.done) 
    let next2_2 = iter2.next()
    arktest.assertEQ(next2_2.value, undefined) 
    arktest.assertTrue(next2_2.done) 
    map2.set(2, buildVal(2))
    let next2_3 = iter2.next()
    arktest.assertEQ(next2_3.value, undefined) 
    arktest.assertTrue(next2_3.done) 
}

////////////////////////////////////////////////////////////////

function getHashCodeByValue64(o: Object): long {
    if (o instanceof Long) {
        return o.toLong()
    }
    if (o instanceof Int) {
        return o.toLong()
    }

    return Runtime.getHashCodeByValue(o)
}

function testHashInt_0(key: int, res_idx: int): void {
    let idx = 0
    const keyHash64: long = getHashCodeByValue64(key as object)
    const keyHash: int = keyHash64.toInt()
    const t = keyHash >> 31
    idx = (keyHash ^ t) - t

    arktest.assertEQ(key, keyHash64, "testHashInt_0(" + key + ", " + res_idx + ") for hash")
    arktest.assertEQ(idx, res_idx, "testHashInt_0(" + key + ", " + res_idx + ") for result")
}

function testHashLong_0(key: long, res_idx: int): void {
    let idx = 0
    const keyHash64: long = getHashCodeByValue64(key as object)
    const keyHash: int = keyHash64.toInt()
    const t = keyHash >> 31
    idx = (keyHash ^ t) - t

    arktest.assertEQ(key, keyHash64, "testHashLong_0(" + key + ", " + res_idx + ") for hash")
    arktest.assertEQ(idx, res_idx, "testHashLong_0(" + key + ", " + res_idx + ") for result")
}

function testHash() {
    // NOTE: verifies the behavior of the bucket index calculation algorithm in the
    // scenario where Runtime.getHashCodeByValue() (EtsEscompatMap::GetHashCode())
    // returns a 64-bit signed hash code
    testHashInt_0( 0, 0)
    testHashInt_0( -3, 3)
    testHashInt_0(-12357, 12357)
    testHashLong_0(-0x7ffffffeffffffab, 0x00000055) // -9223372032559808427, 85
    testHashLong_0(-0x7ffffffe7fffffab, 0x7FFFFFAB) // -9223372030412324779, 2147483563
    testHashLong_0(0x1000000000000055, 0x00000055) // 1152921504606847061, 85
    testHashLong_0(0x1000000080000055, 0x7FFFFFAB) // 1152921506754330709, 2147483563
}

////////////////////////////////////////////////////////////////

function main(): int {
    const suite = new arktest.ArkTestsuite("Map opt about iterator")

    suite.addTest("Map test clear in foreach", testClearInForEach)
    suite.addTest("Map test set after clear in foreach", testSetAfterClearInForEach)
    suite.addTest("Map test clear in iterator", testClearIterator)
    suite.addTest("Map test delete in foreach", testDeleteInForEach)
    suite.addTest("Map test delete in iterator", testDeleteInIterator)
    suite.addTest("Map test set in foreach", testSetInForEach)
    suite.addTest("Map test set in iterator", testSetInIterator)
    suite.addTest("Multiple delete in foreach", testMultipleDeleteForEach)
    suite.addTest("Multiple delete and set in foreach", testMultipleDeleteAndSetForEach)
    suite.addTest("Set after end", testSetAfterEnd)
    suite.addTest("Signed 64-bit Hash", testHash)

    return suite.run()
}