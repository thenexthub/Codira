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
    const suite = new arktest.ArkTestsuite('Sting API test for SDK');
    suite.addTest('stringGetTest1', stringGetTest1);
    suite.addTest('stringGetTest2', stringGetTest2);
    suite.addTest('stringGetTest5', stringGetTest5);
    suite.addTest('stringGetTest6', stringGetTest6);
    suite.addTest('stringGetTest7', stringGetTest7);
    return suite.run()
}

function stringGetTest1() {
    let str: string = 'abcd'
    arktest.assertEQ(str[0], 'a')
    arktest.assertEQ(str[1], 'b')
    arktest.assertEQ(str[2], 'c')
    arktest.assertEQ(str[3], 'd')
}

function stringGetTest2() {
    let str: string = 'abcd'
    arktest.assertEQ(typeof str[0], 'string')
    arktest.assertEQ(typeof str[1], 'string')
    arktest.assertEQ(typeof str[2], 'string')
    arktest.assertEQ(typeof str[3], 'string')
}

function stringGetTest5() {
    const str: string = '\0\u2028😀中文';

    arktest.assertEQ(str[0], '\0');
    arktest.assertEQ(str[1], '\u2028');

    arktest.assertEQ(str.length, 6);
    arktest.assertEQ(str[2], '\uD83D');
    arktest.assertEQ(str[3], '\uDE00');

    arktest.assertEQ(str[4], '中');
    arktest.assertEQ(str[5], '文');
}

function stringGetTest6(): void {
    const str: string = "Aαあ😀🌍";

    arktest.assertEQ(str.length, 7);
    arktest.assertEQ(str[0], 'A');
    arktest.assertEQ(str[1], 'α');
    arktest.assertEQ(str[2], 'あ');
    arktest.assertEQ(str[3]+str[4], '😀');
    arktest.assertEQ(str.slice(5,7), '🌍');
}

function stringGetTest7() {
    let str: string = 'abcd'
    let errorFlag = 0;
    try {
        console.log(str[-1]);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            errorFlag = 1;
        } else {
            errorFlag = 2;
        }
    }
    arktest.assertEQ(1, errorFlag);

    try {
        console.log(str[4]);
    } catch (e) {
        if (e instanceof StringIndexOutOfBoundsError) {
            errorFlag = 1;
        } else {
            errorFlag = 2;
        }
    }
    arktest.assertEQ(1, errorFlag);
}