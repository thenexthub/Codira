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
const AB_SIZE = ARRAY_SIZE * Short.BYTE_SIZE;
const UNSORTED: Array<Double> = [5, 3, 127, 2, 4, 255, 99, 10, 7, 65535 as int, 45, 1, 88, 6, 0];
const SORTED = [0, 1, 2, 3, 4, 5, 6, 7, 10, 45, 88, 99, 127, 255, 65535 as int];
const SORTED_UNALIGNED = [0, 1, 2, 3, 4, 6, 7, 10, 45, 88, 99, 127, 255, 65535 as int];
const SINGLE: number[] = [88];

function fillBuffer(buf: ArrayBuffer): void {
    arktest.assertEQ(Short.BYTE_SIZE, Uint16Array.BYTES_PER_ELEMENT);
    arktest.assertEQ(Short.BYTE_SIZE, 2);
    for (let i = 0; i < AB_SIZE; ++i) {
        if ((i % Short.BYTE_SIZE) == 0) {
            let v: int = UNSORTED[i / 2].toInt();
            if (Runtime.isLittleEndianPlatform()) {
                buf.set(i, v.toByte());
                buf.set(i + 1, (v >>> 8).toByte());
            } else {
                buf.set(i, (v >>> 8).toByte());
                buf.set(i + 1, v.toByte());
            }
        }
    }
}

function checkSorted(a: Uint16Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        arktest.assertEQ(a[i], SORTED[i]);
    }
}

function checkSortedUnaligned(a: Uint16Array): void {
    for (let i = 0; i < ARRAY_SIZE - 1; ++i) {
        arktest.assertEQ(a[i], SORTED_UNALIGNED[i]);
    }
}

function sortEmpty(): void {
    let empty: Uint16Array = new Uint16Array();
    empty.sort();
}

function sortSingle(): void {
    let single: Uint16Array = new Uint16Array(SINGLE);
    single.sort();
    arktest.assertEQ(single[0], SINGLE[0]);
}

function sortNormal(): void {
    let a: Uint16Array = new Uint16Array(UNSORTED);
    a.sort();
    checkSorted(a);
}

function sortUnaligned(): void {
    // Introduce 2-byte offset to test the case when underlying array data
    // does not start at 64-bit aligned address
    let ab: ArrayBuffer = new ArrayBuffer(AB_SIZE);
    let aUnaligned: Uint16Array = new Uint16Array(ab, 2 /* 2-byte offset */);
    fillBuffer(ab);
    aUnaligned.sort();
    checkSortedUnaligned(aUnaligned);
}

function main(): int {
    arktest.assertEQ(UNSORTED.length, SORTED.length);
    const suite = new arktest.ArkTestsuite('Uint16Array.sort() tests')
    suite.addTest('sort an empty Uint16Array', sortEmpty)
    suite.addTest('sort a single-element Uint16Array', sortSingle)
    suite.addTest('sort a normal Uint16Array', sortNormal)
    suite.addTest('sort a normal Uint16Array backed with unaligned ArrayBuffer', sortUnaligned)
    return suite.run()
}
