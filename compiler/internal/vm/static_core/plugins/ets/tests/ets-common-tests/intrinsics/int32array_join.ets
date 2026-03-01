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

const SINGLE: number[] = [1];
const ARRAY: Array<Double> = [Int.MAX_VALUE, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Int.MIN_VALUE];
const EMPTY_JOINED = '2147483647-10123456789-2147483648';
const COMMA_JOINED = '2147483647,-1,0,1,2,3,4,5,6,7,8,9,-2147483648';
const NSIGN_JOINED = '2147483647###-1###0###1###2###3###4###5###6###7###8###9###-2147483648';
const UTF16_JOINED = '2147483647 ΔΘΩ -1 ΔΘΩ 0 ΔΘΩ 1 ΔΘΩ 2 ΔΘΩ 3 ΔΘΩ 4 ΔΘΩ 5 ΔΘΩ 6 ΔΘΩ 7 ΔΘΩ 8 ΔΘΩ 9 ΔΘΩ -2147483648';
const UTF16_JOINED_1 = '2147483647Ω-1Ω0Ω1Ω2Ω3Ω4Ω5Ω6Ω7Ω8Ω9Ω-2147483648';

function joinEmpty(): void {
    let a: Int32Array = new Int32Array(0);
    let s = a.join();
    arktest.assertTrue(s.isEmpty());
}

function joinSingle(): void {
    let a: Int32Array = new Int32Array(SINGLE);
    arktest.assertEQ(a.join(), '1');
    arktest.assertEQ(a.join(','), '1');
}

function joinDefaultDelim(): void {
    let a: Int32Array = new Int32Array(ARRAY);
    arktest.assertEQ(a.join(), COMMA_JOINED);
}

function joinEmptyDelim(): void {
    let a: Int32Array = new Int32Array(ARRAY);
    arktest.assertEQ(a.join(''), EMPTY_JOINED);
}

function joinCommaDelim(): void {
    let a: Int32Array = new Int32Array(ARRAY);
    arktest.assertEQ(a.join(','), COMMA_JOINED);
}

function joinNsignDelim(): void {
    let a: Int32Array = new Int32Array(ARRAY);
    arktest.assertEQ(a.join('###'), NSIGN_JOINED);
}

function joinUtf16Delim(): void {
    let a: Int32Array = new Int32Array(ARRAY);
    arktest.assertEQ(a.join(' ΔΘΩ '), UTF16_JOINED);
}

function joinUtf16Delim1(): void {
    let a: Int32Array = new Int32Array(ARRAY);
    arktest.assertEQ(a.join('Ω'), UTF16_JOINED_1);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Int32Array.join() tests')
    suite.addTest('join an empty Int32Array', joinEmpty)
    suite.addTest('join a single-element Int32Array', joinSingle)
    suite.addTest('join a normal Int32Array using default delimiter', joinDefaultDelim)
    suite.addTest('join a normal Int32Array using empty delimiter', joinEmptyDelim)
    suite.addTest('join a normal Int32Array using comma delimiter', joinCommaDelim)
    suite.addTest('join a normal Int32Array using nsign delimiter', joinNsignDelim)
    suite.addTest('join a normal Int32Array using utf16 delimiter', joinUtf16Delim)
    suite.addTest('join a normal Int32Array using a single utf16 char delimiter', joinUtf16Delim1)
    return suite.run()
}
