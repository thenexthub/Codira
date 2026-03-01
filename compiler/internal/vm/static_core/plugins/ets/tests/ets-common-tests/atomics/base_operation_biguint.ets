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

const BYTE_LENGTH = 128

let ab = new ArrayBuffer(BYTE_LENGTH)
let biguint64array = new BigUint64Array(ab)
let largeValue = BigInt('18446744073709551615') // 0xffffffffffffffff Max value of BigUint64

function testAddBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, largeValue)

        let old = Atomics.add(ta, j, 0n)
        arktest.assertEQ(old, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? largeValue : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testAndBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, largeValue)

        let old = Atomics.and(ta, j, 1n)
        arktest.assertEQ(old, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 1n : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testCompareExchangeBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, largeValue)
        let old = Atomics.compareExchange(ta, j, largeValue, 12n)
        arktest.assertEQ(old, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 12n : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testExchangeBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, largeValue)
        let old = Atomics.exchange(ta, j, 12n)
        arktest.assertEQ(old, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 12n : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testLoadBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let expected = (k === j) ? largeValue : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), expected)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testOrBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, largeValue)

        let old = Atomics.or(ta, j, 1n)
        arktest.assertEQ(old, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? largeValue : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testStoreBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        let re = Atomics.store(ta, j, largeValue)
        arktest.assertEQ(re, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let expected = (k === j) ? largeValue : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), expected)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testSubBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, largeValue)

        let old = Atomics.sub(ta, j, largeValue - 36n)
        arktest.assertEQ(old, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 36n : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testXorBigUint() {
    let ta = biguint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, largeValue)

        let old = Atomics.xor(ta, j, 1n)
        arktest.assertEQ(old, largeValue)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? (largeValue -1n) : 0n
            arktest.assertEQ(Atomics.load(ta as BigUint64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function main() {
    arktest.assertEQ(biguint64array.byteOffset, 0)
    const suite = new arktest.ArkTestsuite('Function Atomics base_operation_biguint tests')
    suite.addTest('Function testAddBigUint', testAddBigUint)
    suite.addTest('Function testAndBigUint', testAndBigUint)
    suite.addTest('Function testCompareExchangeBigUint', testCompareExchangeBigUint)
    suite.addTest('Function testExchangeBigUint', testExchangeBigUint)
    suite.addTest('Function testLoadBigUint', testLoadBigUint)
    suite.addTest('Function testOrBigUint', testOrBigUint)
    suite.addTest('Function testStoreBigUint', testStoreBigUint)
    suite.addTest('Function testSubBigUint', testSubBigUint)
    suite.addTest('Function testXorBigUint', testXorBigUint)
    return suite.run()
}