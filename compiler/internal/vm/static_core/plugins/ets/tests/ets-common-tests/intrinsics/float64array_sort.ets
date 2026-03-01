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
const UNSORTED: Array<Double> = [
    Double.MAX_VALUE, 0, 100.5, 3.375, 1.125, -1.125,
    Double.POSITIVE_INFINITY, 10.5, 7.375, 45.125, 1.375,
    Double.NEGATIVE_INFINITY, Double.MIN_VALUE, 36.625, 123.375];
const SORTED = [
    Double.NEGATIVE_INFINITY, -1.125,0,
    Double.MIN_VALUE, 1.125, 1.375, 3.375, 7.375, 10.5, 36.625, 45.125, 100.5, 123.375,
    Double.MAX_VALUE, Double.POSITIVE_INFINITY];
const SORTED_UNALIGNED = [
    Double.NEGATIVE_INFINITY, -1.125, 0,
    Double.MIN_VALUE, 1.125, 1.375, 3.375, 7.375, 10.5, 36.625, 45.125, 100.5, 123.375,
    Double.POSITIVE_INFINITY];
const UNSORTED_WITH_NAN = [
    Double.MAX_VALUE, 0.0, 100.5, Double.NaN, 1.125, -1.125,
    Double.POSITIVE_INFINITY, 10.5, 7.375, 45.125, 1.375,
    Double.NEGATIVE_INFINITY, Double.MIN_VALUE, 36.625, 123.375];
const SORTED_WITH_NAN = [
    Double.NEGATIVE_INFINITY, -1.125, 0.0, 100.5,
    Double.MAX_VALUE, Double.NaN, Double.MIN_VALUE, 1.125, 1.375, 7.375, 10.5, 36.625, 45.125, 123.375,
    Double.POSITIVE_INFINITY];
const SINGLE: number[] = [88.0];

function checkSorted(a: Float64Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        arktest.assertEQ(a[i], SORTED[i]);
    }
}

function checkSortedWithNaN(a: Float64Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        if (a[i] != SORTED_WITH_NAN[i] as double) {
            if (Double.isNaN(a[i] as double) && Double.isNaN(SORTED_WITH_NAN[i] as double)) {
                continue;
            }
        }
        arktest.assertEQ(a[i], SORTED_WITH_NAN[i] as double);
    }
}

function sortEmpty(): void {
    let empty: Float64Array = new Float64Array();
    empty.sort();
}

function sortSingle(): void {
    let single: Float64Array = new Float64Array(SINGLE);
    single.sort();
    arktest.assertEQ(single[0], SINGLE[0]);
}

function sortNormal(): void {
    let a: Float64Array = new Float64Array(UNSORTED);
    a.sort();
    checkSorted(a);
}

function sortWithNaN(): void {
    let a2: Float64Array = new Float64Array(UNSORTED_WITH_NAN);
    a2.sort();
    checkSortedWithNaN(a2);
}

function main(): int {
    arktest.assertEQ(UNSORTED.length, SORTED.length);
    const suite = new arktest.ArkTestsuite('Float64Array.sort() tests')
    suite.addTest('sort an empty Float64Array', sortEmpty)
    suite.addTest('sort a single-element Float64Array', sortSingle)
    suite.addTest('sort a normal Float64Array', sortNormal)
    return suite.run()
}
