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

function checkNumberArray(src: Array<number>, expected: Array<number>): void {
    arktest.assertEQ(src.length, expected.length, 'Unexpected array length');
    for (let i: int = 0; i < src.length; i++) {
        arktest.assertEQ(src[i], expected[i], 'Unexpected element with index ' + i + ': expected ' + expected[i]
            + ' but was ' + src[i]);
    }
}

function checkStringArray(src: Array<string>, expected: Array<string>): void {
    arktest.assertEQ(src.length, expected.length, 'Unexpected array length');
    for (let i: int = 0; i < src.length; i++) {
        arktest.assertEQ(src[i], expected[i], 'Unexpected element with index ' + i + ': expected \'' + expected[i]
            + '\' but was \'' + src[i] + '\'');
    }
}

function testArraysOfNumbers() {
    let empty: Array<number> = new Array<number>();
    checkNumberArray(empty.concat(empty), new Array<number>());

    let arr1: Array<number> = Array.of(1.0, 2, 3);
    checkNumberArray(empty.concat(arr1), arr1);
    checkNumberArray(arr1.concat(), arr1);
    checkNumberArray(arr1.concat(empty), arr1);
    checkNumberArray(arr1.concat(empty, empty), arr1);
    checkNumberArray(arr1.concat(empty, empty, empty), arr1);
    checkNumberArray(arr1.concat(arr1), Array.of(1.0, 2, 3, 1, 2, 3));
    checkNumberArray(arr1.concat(arr1, arr1), Array.of(1.0, 2, 3, 1, 2, 3, 1, 2, 3));

    let arr2: Array<number> = Array.of(3.0, 3, 3);
    checkNumberArray(arr2.concat(arr2), Array.of(3.0, 3, 3, 3, 3, 3));
    checkNumberArray(arr1.concat(arr2), Array.of(1.0, 2, 3, 3, 3, 3));
    checkNumberArray(arr2.concat(arr1), Array.of(3.0, 3, 3, 1, 2, 3));
    checkNumberArray(arr2.concat(arr1, arr2), Array.of(3.0, 3, 3, 1, 2, 3, 3, 3, 3));
    checkNumberArray(arr2.concat(empty, arr1, arr2), Array.of(3.0, 3, 3, 1, 2, 3, 3, 3, 3));
    checkNumberArray(arr2.concat(arr1, empty, arr2), Array.of(3.0, 3, 3, 1, 2, 3, 3, 3, 3));
    checkNumberArray(arr2.concat(arr1, arr2, empty), Array.of(3.0, 3, 3, 1, 2, 3, 3, 3, 3));

    let arr3: Array<number> = Array.of(4.0, 5, 6);
    checkNumberArray(arr1.concat(arr3), Array.of(1.0, 2, 3, 4, 5, 6));
    checkNumberArray(arr1.concat(arr3, arr3, arr3), Array.of(1.0, 2, 3, 4, 5, 6, 4, 5, 6, 4, 5, 6));
}

function testArraysOfStrings() {
    let empty: Array<string> = new Array<string>();
    checkStringArray(empty.concat(empty), new Array<string>());

    let arr1: Array<string> = Array.of(String("1"), "2", "3");
    checkStringArray(empty.concat(arr1), arr1);
    checkStringArray(arr1.concat(), arr1);
    checkStringArray(arr1.concat(empty), arr1);
    checkStringArray(arr1.concat(empty, empty), arr1);
    checkStringArray(arr1.concat(empty, empty, empty), arr1);
    checkStringArray(arr1.concat(arr1), Array.of(String("1"), "2", "3", "1", "2", "3"));
    checkStringArray(arr1.concat(arr1, arr1), Array.of(String("1"), "2", "3", "1", "2", "3", "1", "2", "3"));
    checkStringArray(arr1.concat(arr1, arr1, empty), Array.of(String("1"), "2", "3", "1", "2", "3", "1", "2", "3"));
    checkStringArray(arr1.concat(arr1, empty, arr1), Array.of(String("1"), "2", "3", "1", "2", "3", "1", "2", "3"));
    checkStringArray(arr1.concat(empty, arr1, arr1), Array.of(String("1"), "2", "3", "1", "2", "3", "1", "2", "3"));

    let arr2: Array<string> = Array.of(String(""), "", "");
    checkStringArray(arr2.concat(empty), Array.of(String(""), "", ""));
    checkStringArray(arr2.concat(arr2), Array.of(String(""), "", "", "", "", ""));
    checkStringArray(arr1.concat(arr2), Array.of(String("1"), "2", "3", "", "", ""));
    checkStringArray(arr2.concat(arr1), Array.of(String(""), "", "", "1", "2", "3"));
    checkStringArray(arr2.concat(arr1, arr2), Array.of(String(""), "", "", "1", "2", "3", "", "", ""));

    let arr3: Array<string> = Array.of(String("4"), "5", "6");
    checkStringArray(arr1.concat(arr3), Array.of(String("1"), "2", "3", "4", "5", "6"));
    checkStringArray(arr1.concat(empty, arr3), Array.of(String("1"), "2", "3", "4", "5", "6"));
    checkStringArray(arr1.concat(arr3, empty), Array.of(String("1"), "2", "3", "4", "5", "6"));
    checkStringArray(arr1.concat(arr3, arr3, arr3),
        Array.of(String("1"), "2", "3", "4", "5", "6", "4", "5", "6", "4", "5", "6"));
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('Array');
    testSuite.addTest('concat for arrays of numbers', testArraysOfNumbers);
    testSuite.addTest('concat for arrays of strings', testArraysOfStrings);
    return testSuite.run();
}
