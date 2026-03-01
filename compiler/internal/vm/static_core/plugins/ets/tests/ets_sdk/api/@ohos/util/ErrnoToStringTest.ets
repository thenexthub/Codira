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

import {util} from "@ohos.util";

function main(): int {
    const suite = new arktest.ArkTestsuite("CommonUtil errnoToString API tests")

    suite.addTest('Verify that the string representation of an unknown system error 10 is returned.',
        testUtilerrnoToString001)
    suite.addTest('Verify that the string representation of an unknown system error 0 is returned.',
        testUtilerrnoToString002)
    suite.addTest('Verify that the string representation of a known system error -1 is returned.',
        testUtilerrnoToString003)
    suite.addTest('Verify that the string representation of an unknown system error 9 is returned.',
        testUtilerrnoToString004)
    suite.addTest('Verify that the string representation of an unknown system error 555 is returned.',
        testUtilerrnoToString005)
    suite.addTest('Verify that the string representation of a known system error -2 is returned.',
        testUtilerrnoToString006)
    suite.addTest('Verify that the string representation of a known system error -3 is returned.',
        testUtilerrnoToString007)
    suite.addTest('Verify that the string representation of a known system error -4 is returned.',
        testUtilerrnoToString008)
    suite.addTest('Verify that the string representation of a known system error -5 is returned.',
        testUtilerrnoToString009)
    suite.addTest('Verify that the string representation of a known system error -11 is returned.',
        testUtilerrnoToString010)
    suite.addTest('Verify that the string representation of a known system error -12 is returned.',
        testUtilerrnoToString011)
    suite.addTest('Verify that the string representation of a known system error -13 is returned.',
        testUtilerrnoToString012)
    suite.addTest('Verify that the string representation of a known system error -100 is returned.',
        testUtilerrnoToString013)

    return suite.run()
}

function testUtilerrnoToString001() {
    const errnum = 10;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'Unknown system error 10');
}

function testUtilerrnoToString002() {
    const errnum = 0;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'Unknown system error 0');
}

function testUtilerrnoToString003() {
    const errnum = -1;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'operation not permitted');
}

function testUtilerrnoToString004() {
    const errnum = 9;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'Unknown system error 9');
}

function testUtilerrnoToString005() {
    const errnum = 555;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'Unknown system error 555');
}

function testUtilerrnoToString006() {
    const errnum = -2;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'no such file or directory');
}

function testUtilerrnoToString007() {
    const errnum = -3;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'no such process');
}

function testUtilerrnoToString008() {
    const errnum = -4;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'interrupted system call');
}

function testUtilerrnoToString009() {
    const errnum = -5;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'i/o error');
}

function testUtilerrnoToString010() {
    const errnum = -11;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'resource temporarily unavailable');
}

function testUtilerrnoToString011() {
    const errnum = -12;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'not enough memory');
}

function testUtilerrnoToString012() {
    const errnum = -13;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'permission denied');
}

function testUtilerrnoToString013() {
    const errnum = -100;
    const result = util.errnoToString(errnum);
    arktest.assertEQ(result, 'network is down');
}
