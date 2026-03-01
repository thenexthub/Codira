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
const ARRAY: Array<Double> = [Float.MIN_VALUE, Float.NaN, -1, 0, 0.2, 1, 2, 3, 4, 5, 6, 7, 8, 9, Float.POSITIVE_INFINITY, Float.NEGATIVE_INFINITY, Float.MAX_VALUE];
const EMPTY_JOINED = "1.401298464324817e-45NaN-100.20000000298023224123456789Infinity-Infinity3.4028234663852886e+38";
const COMMA_JOINED = "1.401298464324817e-45,NaN,-1,0,0.20000000298023224,1,2,3,4,5,6,7,8,9,Infinity,-Infinity,3.4028234663852886e+38";
const NSIGN_JOINED = "1.401298464324817e-45###NaN###-1###0###0.20000000298023224###1###2###3###4###5###6###7###8###9###Infinity###-Infinity###3.4028234663852886e+38";
const UTF16_JOINED = "1.401298464324817e-45 ΔΘΩ NaN ΔΘΩ -1 ΔΘΩ 0 ΔΘΩ 0.20000000298023224 ΔΘΩ 1 ΔΘΩ 2 ΔΘΩ 3 ΔΘΩ 4 ΔΘΩ 5 ΔΘΩ 6 ΔΘΩ 7 ΔΘΩ 8 ΔΘΩ 9 ΔΘΩ Infinity ΔΘΩ -Infinity ΔΘΩ 3.4028234663852886e+38";
const UTF16_JOINED_1 = "1.401298464324817e-45ΩNaNΩ-1Ω0Ω0.20000000298023224Ω1Ω2Ω3Ω4Ω5Ω6Ω7Ω8Ω9ΩInfinityΩ-InfinityΩ3.4028234663852886e+38";

function joinEmpty(): void {
    let a: Float32Array = new Float32Array(0);
    let s = a.join();
    arktest.assertTrue(s.isEmpty());
}

function joinSingle(): void {
    let a: Float32Array = new Float32Array(SINGLE);
    arktest.assertEQ(a.join(), "1");
    arktest.assertEQ(a.join(","), "1");
}

function joinDefaultDelim(): void {
    let a: Float32Array = new Float32Array(ARRAY);
    arktest.assertEQ(a.join(), COMMA_JOINED);
}

function joinEmptyDelim(): void {
    let a: Float32Array = new Float32Array(ARRAY);
    arktest.assertEQ(a.join(""), EMPTY_JOINED);
}

function joinCommaDelim(): void {
    let a: Float32Array = new Float32Array(ARRAY);
    arktest.assertEQ(a.join(","), COMMA_JOINED);
}

function joinNsignDelim(): void {
    let a: Float32Array = new Float32Array(ARRAY);
    arktest.assertEQ(a.join("###"), NSIGN_JOINED);
}

function joinUtf16Delim(): void {
    let a: Float32Array = new Float32Array(ARRAY);
    arktest.assertEQ(a.join(" ΔΘΩ "), UTF16_JOINED);
}

function joinUtf16Delim1(): void {
    let a: Float32Array = new Float32Array(ARRAY);
    arktest.assertEQ(a.join("Ω"), UTF16_JOINED_1);
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Float32Array.join() tests")
    suite.addTest("join an empty Float32Array", joinEmpty)
    suite.addTest("join a single-element Float32Array", joinSingle)
    suite.addTest("join a normal Float32Array using default delimiter", joinDefaultDelim)
    suite.addTest("join a normal Float32Array using empty delimiter", joinEmptyDelim)
    suite.addTest("join a normal Float32Array using comma delimiter", joinCommaDelim)
    suite.addTest("join a normal Float32Array using nsign delimiter", joinNsignDelim)
    suite.addTest("join a normal Float32Array using utf16 delimiter", joinUtf16Delim)
    suite.addTest("join a normal Float32Array using a single utf16 char delimiter", joinUtf16Delim1)
    return suite.run()
}
