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
const UNSORTED: Array<Double> = [5, 3, Long.MAX_VALUE, 2, 4, -1, 99, 10, 7, 0, 45, 1,88, Long.MIN_VALUE, 37];
const SORTED = [Long.MIN_VALUE, -1, 0, 1, 2, 3, 4, 5, 7, 10, 37, 45, 88, 99, Long.MAX_VALUE];
const SINGLE: number[] = [88];

function checkSorted(a: BigInt64Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        arktest.assertEQ(a[i].getLong(), SORTED[i] as long);
    }
}

function sortEmpty(): void {
    let empty: BigInt64Array = new BigInt64Array();
    empty.sort();
}

function sortSingle(): void {
    let single: BigInt64Array = new BigInt64Array(SINGLE);
    single.sort();
    arktest.assertEQ(single[0].getLong(), SINGLE[0].toLong());
}

function sortNormal(): void {
    let a: BigInt64Array = new BigInt64Array(UNSORTED);
    a.sort();
    checkSorted(a);
}

function main(): int {
    arktest.assertEQ(UNSORTED.length, SORTED.length);
    const suite = new arktest.ArkTestsuite('BigInt64Array.sort() tests')
    suite.addTest('sort an empty BigInt64Array', sortEmpty)
    suite.addTest('sort a single-element BigInt64Array', sortSingle)
    suite.addTest('sort a normal BigInt64Array', sortNormal)
    return suite.run();
}
