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

function main(): int {
    const suite = new arktest.ArkTestsuite("RegExpExec Tests");
    suite.addTest("test RegExp.exec1", testRegExpExec1);
    suite.addTest("test RegExp.exec2", testRegExpExec2);
    suite.addTest("test RegExp.exec3", testRegExpExec3);
    suite.addTest("test RegExp.exec4", testRegExpExec4);
    suite.addTest("test RegExp.exec5", testRegExpExec5);
    suite.addTest("test RegExp.exec6", testRegExpExec6);
    suite.addTest("test RegExp.exec7", testRegExpExec7);
    suite.addTest("test RegExp.exec8", testRegExpExec8);

    return suite.run();
}

function testRegExpExec1(): void {
    let arr = new RegExp("\x00").exec("\u0000");
    arktest.assertNE(arr, null);
    if (arr != null) {
        arktest.assertEQ(arr[0]!.codePointAt(0), 0, "testRegExpExec1");
    }
}

function testRegExpExec2(): void {
    let arr = new RegExp("ab\x00g").exec("ab\u0000grfd");
    arktest.assertNE(arr, null);
    if (arr != null) {
        arktest.assertEQ(arr[0]!.codePointAt(0), 97, "testRegExpExec2");
        arktest.assertEQ(arr[0]!.codePointAt(1), 98, "testRegExpExec2");
        arktest.assertEQ(arr[0]!.codePointAt(2), 0, "testRegExpExec2");
        arktest.assertEQ(arr[0]!.codePointAt(3), 103, "testRegExpExec2");
    }
}

function testRegExpExec3(): void {
    let arr = new RegExp("ab").exec("ab\u0000grfd");
    arktest.assertNE(arr, null);
    if (arr != null) {
        arktest.assertEQ(arr[0]!.codePointAt(0), 97, "testRegExpExec3");
        arktest.assertEQ(arr[0]!.codePointAt(1), 98, "testRegExpExec3");
    }
}

function testRegExpExec4(): void {
    let arr = new RegExp("ab\x00g").exec("abgrfddfs");
    arktest.assertEQ(arr, null, "testRegExpExec4");
}

function testRegExpExec5(): void {
    let arr = new RegExp("\x00").exec("");
    arktest.assertEQ(arr, null, "testRegExpExec5");
}

function testRegExpExec6(): void {
    let arr = new RegExp("").exec("\u0000");
    arktest.assertNE(arr, null);
    if (arr != null) {
        arktest.assertEQ(arr[0]!.length, 0, "testRegExpExec6");
    }
}

function testRegExpExec7(): void {
    let arr = new RegExp("").exec("");
    arktest.assertNE(arr, null);
    if (arr != null) {
        arktest.assertEQ(arr[0]!.length, 0, "testRegExpExec7");
    }
}

function testRegExpExec8(): void {
    let arr = new RegExp("(?<name>)b").exec("test b");
    arktest.assertNE(arr, null);
    if (arr != null) {
        arktest.assertEQ(arr[0]!, "b", "testRegExpExec8");
        arktest.assertEQ(arr[1]!, "", "testRegExpExec8");
    }
}