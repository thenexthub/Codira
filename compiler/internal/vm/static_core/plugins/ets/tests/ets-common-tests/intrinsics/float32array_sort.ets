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
const AB_SIZE = ARRAY_SIZE * Float.BYTE_SIZE;
const UNSORTED: Array<Double> = [
    Float.MAX_VALUE.toDouble(), 0, 100.5, 3.375, 1.125, -1.125,
    Float.POSITIVE_INFINITY.toDouble(), 10.5, 7.375, 45.125, 1.375, 
    Float.NEGATIVE_INFINITY.toDouble(),
    Float.MIN_VALUE.toDouble(), 36.625, 123.375];
const SORTED = [
    Float.NEGATIVE_INFINITY, -1.125, 0,
    Float.MIN_VALUE, 1.125, 1.375, 3.375, 7.375, 10.5, 36.625, 45.125, 100.5, 123.375,
    Float.MAX_VALUE, Float.POSITIVE_INFINITY];
const SORTED_UNALIGNED = [
    Float.NEGATIVE_INFINITY, -1.125, 0,
    Float.MIN_VALUE, 1.125, 1.375, 3.375, 7.375, 10.5, 36.625, 45.125, 100.5, 123.375, Float.POSITIVE_INFINITY];
const UNSORTED_WITH_NAN: double[] = [
    Float.MAX_VALUE.toDouble(), 0.0, 100.5,
    Float.NaN.toDouble(), 1.125, -1.125,
    Float.POSITIVE_INFINITY.toDouble(), 10.5, 7.375, 45.125, 1.375,
    Float.NEGATIVE_INFINITY.toDouble(), Float.MIN_VALUE.toDouble(), 36.625, 123.375];
const SORTED_WITH_NAN: double[] = [
    Float.NEGATIVE_INFINITY.toDouble(), -1.125, 0.0, 100.5,
    Float.MAX_VALUE.toDouble(), Float.NaN.toDouble(), Float.MIN_VALUE.toDouble(),
    1.125, 1.375, 7.375, 10.5, 36.625, 45.125, 123.375, Float.POSITIVE_INFINITY.toDouble()];
const SINGLE: number[] = [88.0];

function fillBuffer(buf: ArrayBuffer): void {
    arktest.assertEQ(Float.BYTE_SIZE, Float32Array.BYTES_PER_ELEMENT);
    arktest.assertEQ(Float.BYTE_SIZE, 4);
    for (let i = 0; i < AB_SIZE; ++i) {
        if ((i % Float.BYTE_SIZE) == 0) {
            let v: int = Float.bitCastToInt(UNSORTED[i / Float.BYTE_SIZE].toFloat());
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

function checkSorted(a: Float32Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        arktest.assertEQ(a[i], SORTED[i]);
    }
}

function checkSortedWithNaN(a: Float32Array): void {
    for (let i = 0; i < ARRAY_SIZE; ++i) {
        if (a[i] != SORTED_WITH_NAN[i].toFloat()) {
            if (Float.isNaN(a[i].toFloat()) && Float.isNaN(SORTED_WITH_NAN[i].toFloat())) {
                continue;
            }
        }
        arktest.assertEQ(a[i], SORTED_WITH_NAN[i].toFloat());
    }
}

function checkSortedUnaligned(a: Float32Array): void {
    for (let i = 0; i < ARRAY_SIZE - 1; ++i) {
        arktest.assertEQ(a[i], SORTED_UNALIGNED[i]);
    }
}

function sortEmpty(): void {
    let empty: Float32Array = new Float32Array();
    empty.sort();
}

function sortSingle(): void {
    let single: Float32Array = new Float32Array(SINGLE);
    single.sort();
    arktest.assertEQ(single[0], SINGLE[0]);
}

function sortNormal(): void {
    let a: Float32Array = new Float32Array(UNSORTED);
    a.sort();
    checkSorted(a);
}

function sortWithNaN(): void {
    let a2: Float32Array = new Float32Array(UNSORTED_WITH_NAN);
    a2.sort();
    checkSortedWithNaN(a2);
}

function sortUnaligned(): void {
    // Introduce 4-byte offset to test the case when underlying array data
    // does not start at 64-bit aligned address
    let sab: ArrayBuffer = new ArrayBuffer(AB_SIZE);
    let saUnaligned: Float32Array = new Float32Array(sab, 4 /* 4-byte offset */);
    fillBuffer(sab);
    saUnaligned.sort();
    checkSortedUnaligned(saUnaligned);
}

function main(): int {
    arktest.assertEQ(UNSORTED.length, SORTED.length);
    const suite = new arktest.ArkTestsuite('Float32Array.sort() tests')
    suite.addTest('sort an empty Float32Array', sortEmpty)
    suite.addTest('sort a single-element Float32Array', sortSingle)
    suite.addTest('sort a normal Float32Array', sortNormal)
    suite.addTest('sort a normal Float32Array backed with unaligned ArrayBuffer', sortUnaligned)
    return suite.run()
}
