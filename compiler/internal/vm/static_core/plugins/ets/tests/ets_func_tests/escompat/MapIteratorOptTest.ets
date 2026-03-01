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

function testClearInForEach(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    map.forEach((value, key, map) => {
        map.clear()
        num = num + 1
    });
    arktest.assertEQ(num, 1)
}

function testSetAfterClearInForEach(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    map.forEach((value, key, map) => {
        map.clear()
        if (num === 0) {
            map.set(1.0, "one")
        }
        num = num + 1
    });
    arktest.assertEQ(num, 2)
}


function testClearIterator(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    let iter = map.keys()
    for (const key of iter) {
        map.clear()
        num = num + 1
    }
    arktest.assertEQ(num, 1)

}

function testDeleteCurrentInForEach(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    map.forEach((value, key, map) => {
        map.delete(0.0)
        num = num + 1
    });
    arktest.assertEQ(num, 4)
}

function testDeleteCurrentIterator(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    let iter = map.keys()
    for (const key of iter) {
        map.delete(0.0)
        num = num + 1
    }
    arktest.assertEQ(num, 4)
}

function testDeleteInForEach(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    map.forEach((value, key, map) => {
        map.delete(2.0)
        num = num + 1
    });
    arktest.assertEQ(num, 3)
}

function testDeleteInIterator(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    let iter = map.keys()
    for (const key of iter) {
        map.delete(2.0)
        num = num + 1
    }
    arktest.assertEQ(num, 3)
}

function testSetInForEach(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    map.forEach((value, key, map) => {
        map.set(4.0, "four")
        num = num + 1
    });
    arktest.assertEQ(num, 5)
}

function testSetInIterator(): void {
    let map = new Map<Number, String>()
    let num: int = 0
    map.set(0.0, "zero")
    map.set(1.0, "one")
    map.set(2.0, "two")
    map.set(3.0, "three")
    let iter = map.keys()
    for (const key of iter) {
        map.set(4.0, "four")
        num = num + 1
    }
    arktest.assertEQ(num, 5)
}


function main(): int {
    const suite = new arktest.ArkTestsuite("Map opt about iterator")

    suite.addTest("Map test clear in foreach", testClearInForEach)
    suite.addTest("Map test set after clear in foreach", testSetAfterClearInForEach)
    suite.addTest("Map test clear in iterator", testClearIterator)
    suite.addTest("Map test delete current in foreach", testDeleteCurrentInForEach)
    suite.addTest("Map test delete current in iterator", testDeleteCurrentIterator)
    suite.addTest("Map test delete in foreach", testDeleteInForEach)
    suite.addTest("Map test delete in iterator", testDeleteInIterator)
    suite.addTest("Map test set in foreach", testSetInForEach)
    suite.addTest("Map test set in iterator", testSetInIterator)
    return suite.run()
}
