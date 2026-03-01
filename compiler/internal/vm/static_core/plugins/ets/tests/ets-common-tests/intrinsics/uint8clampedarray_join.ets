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
const ARRAY: Array<Double> = [0,1,2,3,4,5,6,7,8,9,255];
const EMPTY_JOINED = "0123456789255";
const COMMA_JOINED = "0,1,2,3,4,5,6,7,8,9,255";
const NSIGN_JOINED = "0###1###2###3###4###5###6###7###8###9###255";
const UTF16_JOINED = "0 ΔΘΩ 1 ΔΘΩ 2 ΔΘΩ 3 ΔΘΩ 4 ΔΘΩ 5 ΔΘΩ 6 ΔΘΩ 7 ΔΘΩ 8 ΔΘΩ 9 ΔΘΩ 255";
const UTF16_JOINED_1 = "0Ω1Ω2Ω3Ω4Ω5Ω6Ω7Ω8Ω9Ω255";

function joinEmpty(): void {
    let a: Uint8ClampedArray = new Uint8ClampedArray(0);
    let s = a.join();
    arktest.assertTrue(s.isEmpty());
}

function joinSingle(): void {
    let a: Uint8ClampedArray = new Uint8ClampedArray(SINGLE);
    arktest.assertEQ(a.join(), "1");
    arktest.assertEQ(a.join(","), "1");
}

function joinDefaultDelim(): void {
    let a: Uint8ClampedArray = new Uint8ClampedArray(ARRAY);
    arktest.assertEQ(a.join(), COMMA_JOINED);
}

function joinEmptyDelim(): void {
    let a: Uint8ClampedArray = new Uint8ClampedArray(ARRAY);
    arktest.assertEQ(a.join(""), EMPTY_JOINED);
}

function joinCommaDelim(): void {
    let a: Uint8ClampedArray = new Uint8ClampedArray(ARRAY);
    arktest.assertEQ(a.join(","), COMMA_JOINED);
}

function joinNsignDelim(): void {
    let a: Uint8ClampedArray = new Uint8ClampedArray(ARRAY);
    arktest.assertEQ(a.join("###"), NSIGN_JOINED);
}

function joinUtf16Delim(): void {
    let a: Uint8ClampedArray = new Uint8ClampedArray(ARRAY);
    arktest.assertEQ(a.join(" ΔΘΩ "), UTF16_JOINED);
}

function joinUtf16Delim1(): void {
    let a: Uint8ClampedArray = new Uint8ClampedArray(ARRAY);
    arktest.assertEQ(a.join("Ω"), UTF16_JOINED_1);
}

function main(): int {
    const suite = new arktest.ArkTestsuite("Uint8ClampedArray.join() tests")
    suite.addTest("join an empty Uint8ClampedArray", joinEmpty)
    suite.addTest("join a single-element Uint8ClampedArray", joinSingle)
    suite.addTest("join a normal Uint8ClampedArray using default delimiter", joinDefaultDelim)
    suite.addTest("join a normal Uint8ClampedArray using empty delimiter", joinEmptyDelim)
    suite.addTest("join a normal Uint8ClampedArray using comma delimiter", joinCommaDelim)
    suite.addTest("join a normal Uint8ClampedArray using nsign delimiter", joinNsignDelim)
    suite.addTest("join a normal Uint8ClampedArray using utf16 delimiter", joinUtf16Delim)
    suite.addTest("join a normal Uint8ClampedArray using a single utf16 char delimiter", joinUtf16Delim1)
    return suite.run()
}
