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

function testMapForBigInt1(): void {
    let map1: Map<BigInt, BigInt> = new Map<BigInt, BigInt>()
    let a: BigInt = new BigInt(6666666n)
    let b: BigInt = new BigInt(9999999n)
    map1.set(a, a)
    map1.set(b, b)
    let c: BigInt = new BigInt(6666666n)
    arktest.assertTrue(map1.has(c))
    arktest.assertTrue(map1.size === 2)
    map1.set(c, c)
    arktest.assertTrue(map1.size === 2)
    map1.delete(c);
    arktest.assertTrue(map1.size === 1)
    arktest.assertTrue(map1.get(b) === b)
}

function testMapForBigInt2(): void {
    let map1: Map<BigInt, BigInt> = new Map<BigInt, BigInt>([[6666666n, 6666666n],[9999999n, 9999999n]])
    arktest.assertTrue(map1.size === 2)
    map1.set(6666666n, 6666666n)
    let c: BigInt = new BigInt(6666666n)
    arktest.assertTrue(map1.has(c))
    arktest.assertTrue(map1.size === 2)
    map1.set(c, c)
    arktest.assertTrue(map1.size === 2)
    map1.delete(c);
    arktest.assertTrue(map1.size === 1)
}

function testSetForBigInt1(): void {
    let set1: Set<BigInt> = new Set<BigInt>([6666666n, 9999999n])
    arktest.assertTrue(set1.has(6666666n))
    arktest.assertTrue(set1.size === 2)
    set1.delete(6666666n)
    arktest.assertTrue(set1.size === 1)
    set1.add(6666666n)
    arktest.assertTrue(set1.size === 2)
    arktest.assertTrue(set1.has(6666666n))
}

function testSetForBigInt2(): void {
    let set1: Set<BigInt> = new Set<BigInt>()
    let a: BigInt = new BigInt(666666666666666666666666n)
    let b: BigInt = new BigInt(999999999999999999999999n)
    set1.add(a)
    set1.add(b)
    arktest.assertTrue(set1.has(a))
    let c: BigInt = new BigInt(666666666666666666666666n)
    arktest.assertTrue(set1.has(c))
    arktest.assertTrue(set1.size === 2)
    set1.delete(a)
    arktest.assertTrue(set1.size === 1)
    set1.add(a)
    arktest.assertTrue(set1.size === 2)
    set1.delete(c)
    arktest.assertTrue(set1.size === 1)
    set1.add(c)
    arktest.assertTrue(set1.size === 2)
    arktest.assertTrue(set1.has(a))
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Map Set BigInt")

    suite.addTest("Map use BigInt key1", testMapForBigInt1)
    suite.addTest("Map use BigInt key2", testMapForBigInt2)
    suite.addTest("Set use BigInt key1", testSetForBigInt1)
    suite.addTest("Set use BigInt key2", testSetForBigInt2)

    return suite.run()
}
