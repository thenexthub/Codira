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
const UNSORTED: Array<Double> = [5, 3, 127, 2, 4, 255, 99, 10, 7, 0, 45, 1, 88, 6, 4294967295 as long];
const SORTED = [0, 1, 2, 3, 4, 5, 6, 7, 10, 45, 88, 99, 127, 255, 4294967295 as long];
const SORTED_UNALIGNED = [0, 1, 2, 3, 4, 6, 7, 10, 45, 88, 99, 127, 255, 4294967295 as long];
const SINGLE: number[] = [88];

function fillBuffer(buf: ArrayBuffer): void {
    arktest.assertEQ(Int.BYTE_SIZE, Uint32Array.BYTES_PER_ELEMENT);
    arktest.assertEQ(Int.BYTE_SIZE, 4);
    for (let i = 0; i < AB_SIZE; ++i) {
        if ((i % Int.BYTE_SIZE) == 0) {
            let v: long = UNSORTED[i / Int.BYTE_SIZE] as long;
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

function checkSorted(a: Uint32Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        arktest.assertEQ(a[i], SORTED[i] as long);
    }
}

function checkSortedUnaligned(a: Uint32Array): void {
    for (let i = 0; i < ARRAY_SIZE - 1; ++i) {
        arktest.assertEQ(a[i], SORTED_UNALIGNED[i] as long);
    }
}

function sortEmpty(): void {
    let empty: Uint32Array = new Uint32Array();
    empty.sort();
}

function sortSingle(): void {
    let single: Uint32Array = new Uint32Array(SINGLE);
    single.sort();
    arktest.assertEQ(single[0], SINGLE[0]);
}

function sortNormal(): void {
    let a: Uint32Array = new Uint32Array(UNSORTED);
    a.sort();
    checkSorted(a);
}

function sortUnaligned(): void {
    // Introduce 4-byte offset to test the case when underlying array data
    // does not start at 64-bit aligned address
    let ab: ArrayBuffer = new ArrayBuffer(AB_SIZE);
    let aUnaligned: Uint32Array = new Uint32Array(ab, 4 /* 4-byte offset */);
    fillBuffer(ab);
    aUnaligned.sort();
    checkSortedUnaligned(aUnaligned);
}

function main(): int {
    arktest.assertEQ(UNSORTED.length, SORTED.length);
    const suite = new arktest.ArkTestsuite('Uint32Array.sort() tests')
    suite.addTest('sort an empty Uint32Array', sortEmpty)
    suite.addTest('sort a single-element Uint32Array', sortSingle)
    suite.addTest('sort a normal Uint32Array', sortNormal)
    suite.addTest('sort a normal Uint32Array backed with unaligned ArrayBuffer', sortUnaligned)
    return suite.run()
}
