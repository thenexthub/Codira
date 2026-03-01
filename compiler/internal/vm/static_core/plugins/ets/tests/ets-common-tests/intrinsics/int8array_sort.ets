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
const UNSORTED: Array<Double> = [5, 3, 127, 2, 4, -1, 99, 10, 7, 0, 45, 1, 88, -127, 37];
const SORTED = [-127, -1, 0, 1, 2, 3, 4, 5, 7, 10, 37, 45, 88, 99, 127];
const SORTED_UNALIGNED = [-127, -1, 0, 1, 2, 3, 4, 7, 10, 37, 45, 88, 99, 127];
const SINGLE: number[] = [88];

function fillBuffer(buf: ArrayBuffer): void {
    arktest.assertEQ(Byte.BYTE_SIZE, Int8Array.BYTES_PER_ELEMENT);
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        buf.set(i, UNSORTED[i].toByte());
    }
}

function checkSorted(a: Int8Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        arktest.assertEQ(a[i], SORTED[i]);
    }
}

function checkSortedUnaligned(a: Int8Array): void {
    for (let i = 0; i < ARRAY_SIZE - 1; ++i) {
        arktest.assertEQ(a[i], SORTED_UNALIGNED[i]);
    }
}

function sortEmpty(): void {
    let empty: Int8Array = new Int8Array();
    empty.sort();
}

function sortSingle(): void {
    let single: Int8Array = new Int8Array(SINGLE);
    single.sort();
    arktest.assertEQ(single[0], SINGLE[0]);
}

function sortNormal(): void {
    let a: Int8Array = new Int8Array(UNSORTED);
    a.sort();
    checkSorted(a);
}

function sortUnaligned(): void {
    // Introduce 1-byte offset to test the case when underlying array data
    // does not start at 64-bit aligned address
    let ab: ArrayBuffer = new ArrayBuffer(ARRAY_SIZE);
    let aUnaligned: Int8Array = new Int8Array(ab, 1 /* 1-byte offset */);
    fillBuffer(ab);
    aUnaligned.sort();
    checkSortedUnaligned(aUnaligned);
}

function main(): int {
    arktest.assertEQ(UNSORTED.length, SORTED.length);
    const suite = new arktest.ArkTestsuite('Int8Array.sort() tests')
    suite.addTest('sort an empty Int8Array', sortEmpty)
    suite.addTest('sort a single-element Int8Array', sortSingle)
    suite.addTest('sort a normal Int8Array', sortNormal)
    suite.addTest('sort a normal Int8Array backed with unaligned ArrayBuffer', sortUnaligned)
    return suite.run()
}
