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

const BYTE_LENGTH = 128

let ab = new ArrayBuffer(BYTE_LENGTH)
let bigint64array = new BigInt64Array(ab)

function testAddBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        let old = Atomics.add(ta, j, 12n)
        arktest.assertEQ(old, 0n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 12n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testAndBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 3n)

        let old = Atomics.and(ta, j, 1n)
        arktest.assertEQ(old, 3n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 1n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testCompareExchangeBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 7n)
        let old = Atomics.compareExchange(ta, j, 7n, 12n)
        arktest.assertEQ(old, 7n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 12n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testExchangeBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        let old = Atomics.exchange(ta, j, 12n)
        arktest.assertEQ(old, 0n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 12n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testLoadBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 12n)

        for (let k = 0; k < arraySize; k++) {
            let expected = (k === j) ? 12n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), expected)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testOrBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 2n)

        let old = Atomics.or(ta, j, 1n)
        arktest.assertEQ(old, 2n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 3n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testStoreBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        let re = Atomics.store(ta, j, 12n)
        arktest.assertEQ(re, 12n)

        for (let k = 0; k < arraySize; k++) {
            let expected = (k === j) ? 12n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), expected)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testSubBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 48n)

        let old = Atomics.sub(ta, j, 12n)
        arktest.assertEQ(old, 48n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 36n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function testXorBigInt() {
    let ta = bigint64array
    let arraySize = ta.length

    for (let j = 0; j < arraySize; j++) {
        Atomics.store(ta, j, 5n)

        let old = Atomics.xor(ta, j, 1n)
        arktest.assertEQ(old, 5n)

        for (let k = 0; k < arraySize; k++) {
            let re = (k === j) ? 4n : 0n
            arktest.assertEQ(Atomics.load(ta as BigInt64Array, k), re)
        }

        Atomics.store(ta, j, 0n)
    }
}

function main() {
    arktest.assertEQ(bigint64array.byteOffset, 0)
    const suite = new arktest.ArkTestsuite('Function Atomics base_operation_bigint tests')
    suite.addTest('Function testAddBigInt', testAddBigInt)
    suite.addTest('Function testAndBigInt', testAndBigInt)
    suite.addTest('Function testCompareExchangeBigInt', testCompareExchangeBigInt)
    suite.addTest('Function testExchangeBigInt', testExchangeBigInt)
    suite.addTest('Function testLoadBigInt', testLoadBigInt)
    suite.addTest('Function testOrBigInt', testOrBigInt)
    suite.addTest('Function testStoreBigInt', testStoreBigInt)
    suite.addTest('Function testSubBigInt', testSubBigInt)
    suite.addTest('Function testXorBigInt', testXorBigInt)
    return suite.run()
}