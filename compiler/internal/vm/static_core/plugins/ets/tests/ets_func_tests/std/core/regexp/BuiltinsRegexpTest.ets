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
    const suite = new arktest.ArkTestsuite("builtinsregexp Tests");
    suite.addTest("test builtinsregexpTest1", builtinsregexpTest1);
    suite.addTest("test builtinsregexpTest2", builtinsregexpTest2);
    suite.addTest("test builtinsregexpTest3", builtinsregexpTest3);
    suite.addTest("test builtinsregexpTest4", builtinsregexpTest4);
    suite.addTest("test builtinsregexpTest5", builtinsregexpTest5);
    suite.addTest("test builtinsregexpTest6", builtinsregexpTest6);
    return suite.run();
}

function builtinsregexpTest1(): void {
    const url =
        'https://designcloud.uiplus.com/tool//materialServer/upload/images/20210608_5V0J5lVh4xVNYx0AUE.jpg';
    let result = new RegExp("(?<=\\/)\\w+(.jpg)$").match(url);
    if (result != null) {
        arktest.assertEQ(result[0], "20210608_5V0J5lVh4xVNYx0AUE.jpg");
        arktest.assertEQ(result[1], ".jpg");
    }
}

function builtinsregexpTest2(): void {
    let result = new RegExp("(a)").match("abc");
    if (result != null) {
        arktest.assertEQ(result.length, 2);
        arktest.assertEQ(result[0], "a");
        arktest.assertEQ(result[1], "a");
    }
}

function builtinsregexpTest3(): void {
    let pattern = new RegExp("[A-Z]{3}", "g");
    let testStr = "AAAA";
    let res1 = testStr.match(pattern);
    if (res1 != null) {
        arktest.assertEQ(res1[0], "AAA");
        res1[0] = "BB";
    }
    let res2 = testStr.match(pattern);
    if (res2 != null) {
        arktest.assertEQ(res2[0], "AAA");
        res2[0] = "C";
    }
    let res3 = testStr.match(pattern);
    if (res3 != null) {
        arktest.assertEQ(res3[0], "AAA");
    }
}

function builtinsregexpTest4(): void {
    let result = new RegExp("\\/(?:\\B)", "gm").test("/");
    arktest.assertEQ(result, true);
}

function builtinsregexpTest5(): void {
    let result = new RegExp("(\\d+)", "g").exec("Test1234");
    if (result != null) {
        arktest.assertEQ(result[0], "1234");
    }
}

function builtinsregexpTest6(): void {
    let result = new RegExp("(\\w+)@(\\w+)\\.(\\w+)", "g").exec("Contact me at support@example.com for assistance.");
    if (result != null) {
        arktest.assertEQ(result[0], "support@example.com");
        arktest.assertEQ(result.index, 14);
    }
}
