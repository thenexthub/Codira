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

const ARRAY_SIZE = 15;
const AB_SIZE = ARRAY_SIZE * Int.BYTE_SIZE;
const UNSORTED: Array<Double> = [5, 3, Int.MAX_VALUE, 2, 4, -1, 99, 10, 7, 0, 45, 1, 88, Int.MIN_VALUE, 37];
const SORTED = [Int.MIN_VALUE, -1, 0, 1, 2, 3, 4, 5, 7, 10, 37, 45, 88, 99, Int.MAX_VALUE];
const SORTED_UNALIGNED = [Int.MIN_VALUE, -1, 0, 1, 2, 3, 4, 7, 10, 37, 45, 88, 99,Int.MAX_VALUE];
const SINGLE: number[] = [88];

function fillBuffer(buf: ArrayBuffer): void {
    arktest.assertEQ(Int.BYTE_SIZE, Int32Array.BYTES_PER_ELEMENT);
    arktest.assertEQ(Int.BYTE_SIZE, 4);
    for (let i = 0; i < AB_SIZE; ++i) {
        if ((i % Int.BYTE_SIZE) == 0) {
            let v: int = UNSORTED[i / Int.BYTE_SIZE].toInt();
            if (Runtime.isLittleEndianPlatform()) {
                buf.set(i, v.toByte());
                buf.set(i + 1, (v >>> 8).toByte());
                buf.set(i + 2, (v >>> 16).toByte());
                buf.set(i + 3, (v >>> 24).toByte());
            } else {
                buf.set(i, (v >>> 24).toByte());
                buf.set(i + 1, (v >>> 16).toByte());
                buf.set(i + 2, (v >>> 8).toByte());
                buf.set(i + 3, v.toByte());
            }
        }
    }
}

function checkSorted(a: Int32Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        arktest.assertEQ(a[i], SORTED[i]);
    }
}

function checkSortedUnaligned(a: Int32Array): void {
    for (let i = 0; i < ARRAY_SIZE - 1; ++i) {
        arktest.assertEQ(a[i], SORTED_UNALIGNED[i]);
    }
}

function sortEmpty(): void {
    let empty: Int32Array = new Int32Array();
    empty.sort();
}

function sortSingle(): void {
    let single: Int32Array = new Int32Array(SINGLE);
    single.sort();
    arktest.assertEQ(single[0], SINGLE[0]);
}

function sortNormal(): void {
    let a: Int32Array = new Int32Array(UNSORTED);
    a.sort();
    checkSorted(a);
}

function sortUnaligned(): void {
    // Introduce 4-byte offset to test the case when underlying array data
    // does not start at 64-bit aligned address
    let ab: ArrayBuffer = new ArrayBuffer(AB_SIZE);
    let aUnaligned: Int32Array = new Int32Array(ab, 4 /* 4-byte offset */);
    fillBuffer(ab);
    aUnaligned.sort();
    checkSortedUnaligned(aUnaligned);
}

function main(): int {
    arktest.assertEQ(UNSORTED.length, SORTED.length);
    const suite = new arktest.ArkTestsuite('Int32Array.sort() tests')
    suite.addTest('sort an empty Int32Array', sortEmpty)
    suite.addTest('sort a single-element Int32Array', sortSingle)
    suite.addTest('sort a normal Int32Array', sortNormal)
    suite.addTest('sort a normal Int32Array backed with unaligned ArrayBuffer', sortUnaligned)
    return suite.run()
}
