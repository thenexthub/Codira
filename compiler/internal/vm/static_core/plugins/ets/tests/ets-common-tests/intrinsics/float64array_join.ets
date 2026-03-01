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
const ARRAY: Double[] = [
    Double.MIN_VALUE, Double.NaN, -1.1, 0.0, 0.2, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0,
    8.0, 9.0, Double.POSITIVE_INFINITY, Double.NEGATIVE_INFINITY, Double.MAX_VALUE];
const EMPTY_JOINED = '5e-324NaN-1.100.2123456789Infinity-Infinity1.7976931348623157e+308';
const COMMA_JOINED = '5e-324,NaN,-1.1,0,0.2,1,2,3,4,5,6,7,8,9,Infinity,-Infinity,1.7976931348623157e+308';
const NSIGN_JOINED = '5e-324###NaN###-1.1###0###0.2###1###2###3###4###5###6###7###8###9###Infinity###-Infinity###1.7976931348623157e+308';
const UTF16_JOINED = '5e-324 ΔΘΩ NaN ΔΘΩ -1.1 ΔΘΩ 0 ΔΘΩ 0.2 ΔΘΩ 1 ΔΘΩ 2 ΔΘΩ 3 ΔΘΩ 4 ΔΘΩ 5 ΔΘΩ 6 ΔΘΩ 7 ΔΘΩ 8 ΔΘΩ 9 ΔΘΩ Infinity ΔΘΩ -Infinity ΔΘΩ 1.7976931348623157e+308';
const UTF16_JOINED_1 = '5e-324ΩNaNΩ-1.1Ω0Ω0.2Ω1Ω2Ω3Ω4Ω5Ω6Ω7Ω8Ω9ΩInfinityΩ-InfinityΩ1.7976931348623157e+308';

function joinEmpty(): void {
    let a: Float64Array = new Float64Array(0);
    let s = a.join();
    arktest.assertTrue(s.isEmpty());
}

function joinSingle(): void {
    let a: Float64Array = new Float64Array(SINGLE);
    arktest.assertEQ(a.join(), '1');
    arktest.assertEQ(a.join(','), '1');
}

function joinDefaultDelim(): void {
    let a: Float64Array = new Float64Array(ARRAY);
    arktest.assertEQ(a.join(), COMMA_JOINED);
}

function joinEmptyDelim(): void {
    let a: Float64Array = new Float64Array(ARRAY);
    arktest.assertEQ(a.join(''), EMPTY_JOINED);
}

function joinCommaDelim(): void {
    let a: Float64Array = new Float64Array(ARRAY);
    arktest.assertEQ(a.join(','), COMMA_JOINED);
}

function joinNsignDelim(): void {
    let a: Float64Array = new Float64Array(ARRAY);
    arktest.assertEQ(a.join('###'), NSIGN_JOINED);
}

function joinUtf16Delim(): void {
    let a: Float64Array = new Float64Array(ARRAY);
    arktest.assertEQ(a.join(' ΔΘΩ '), UTF16_JOINED);
}

function joinUtf16Delim1(): void {
    let a: Float64Array = new Float64Array(ARRAY);
    arktest.assertEQ(a.join('Ω'), UTF16_JOINED_1);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Float64Array.join() tests')
    suite.addTest('join an empty Float64Array', joinEmpty)
    suite.addTest('join a single-element Float64Array', joinSingle)
    suite.addTest('join a normal Float64Array using default delimiter', joinDefaultDelim)
    suite.addTest('join a normal Float64Array using empty delimiter', joinEmptyDelim)
    suite.addTest('join a normal Float64Array using comma delimiter', joinCommaDelim)
    suite.addTest('join a normal Float64Array using nsign delimiter', joinNsignDelim)
    suite.addTest('join a normal Float64Array using utf16 delimiter', joinUtf16Delim)
    suite.addTest('join a normal Float64Array using a single utf16 char delimiter', joinUtf16Delim1)
    return suite.run()
}
