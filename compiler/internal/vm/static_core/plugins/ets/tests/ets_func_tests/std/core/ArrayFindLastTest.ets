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

function findLastEmptyTest() {
    let data: int[] = [];
    let res = data.findLast((val, index, arr) => { return val != 0; });
    arktest.assertTrue(res === undefined);
    arktest.assertEQ(data.toString(), '');
}

function findLastTest1() {
    let data: int[] = [1, 2, 3, 4, 5];
    let res = data.findLast((val, index, arr) => {
        arr.pop();
        return val % 2 == 0;
    });
    arktest.assertEQ(res, 4);
    arktest.assertEQ(data.toString(), '1,2,3');
}

function findLastTest2() {
    let data: int[] = [6, 5, 4, 3];
    let res = data.findLast((val, index, arr) => {
        arr.shift();
        return val > 5;
    });
    arktest.assertTrue(res === undefined);
    arktest.assertEQ(data.length, 0);
}

function findLastTest3() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.findLast((val, index, arr) => {
        if (arr.length > 1) {
            arr.shift();
            arr.pop();
        }
        return val == 3;
    });
    arktest.assertTrue(res === undefined);
    arktest.assertEQ(data.toString(), '2,3,4,5');
}

function findLastTest4() {
    let data: int[] = [1, 2, 3, 4];
    let res = data.findLast((val, index, arr) => {
        arr.push(val + 10);
        return val == 3;
    });
    arktest.assertEQ(res, 3);
    arktest.assertEQ(data.toString(), '1,2,3,4,14,13');
}

function findLastTest5() {
    let data: int[] = [-1, -1, -1, -1];
    let res = data.findLast((val, index, arr) => {
        arr.shift();
        arr.push(index + 10, index + 100);
        return val > 100;
    });
    arktest.assertTrue(res === undefined);
    arktest.assertEQ(data.toString(), '13,103,12,102,11,101,10,100');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array length change tests');
    suite.addTest('findLast empty test', findLastEmptyTest);
    suite.addTest('findLast test 1', findLastTest1);
    suite.addTest('findLast test 2', findLastTest2);
    suite.addTest('findLast test 3', findLastTest3);
    suite.addTest('findLast test 4', findLastTest4);
    suite.addTest('findLast test 5', findLastTest5);
    return suite.run();
}
