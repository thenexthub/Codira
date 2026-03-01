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

function findLastIndexEmptyTest() {
    let data: int[] = [];
    let res = data.findLastIndex((val, index, arr) => { return val != 0; });
    arktest.assertEQ(res, -1);
    arktest.assertEQ(data.toString(), '');
}

function findLastIndexTest1() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.findLastIndex((val, index, arr) => {
        arr.pop();
        return val == 4;
    });
    arktest.assertEQ(res, 3);
    arktest.assertEQ(data.toString(), '1,2,3');
}

function findLastIndexTest2() {
    let data: int[] = [1, 2, 3, 4, 6, 4, 3];
    let res = data.findLastIndex((val, index, arr) => {
        arr.shift();
        arr.shift();
        return val > 5;
    });
    arktest.assertEQ(res, -1);
    arktest.assertEQ(data.toString(), '3,4,6,4,3');
}

function findLastIndexTest3() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.findLastIndex((val, index, arr) => {
        if (arr.length > 1) {
            arr.shift();
            arr.pop();
        }
        return val == 3;
    });
    arktest.assertEQ(res, -1);
    arktest.assertEQ(data.toString(), '2,3,4,5');
}

function findLastIndexTest4() {
    let data: int[] = [1, 2, 3, 4];
    let res = data.findLastIndex((val, index, arr) => {
        arr.push(val + 10);
        return val == 4;
    });
    arktest.assertEQ(res, 3);
    arktest.assertEQ(data.toString(), '1,2,3,4,14');
}

function findLastIndexTest5() {
    let data: int[] = [-1, -1, -1, -1];
    let res = data.findLastIndex((val, index, arr) => {
        arr.shift();
        arr.push(index + 10, index + 100);
        return val > 100;
    });
    arktest.assertEQ(res, -1);
    arktest.assertEQ(data.toString(), '13,103,12,102,11,101,10,100');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array length change tests');
    suite.addTest('findLastIndex empty test', findLastIndexEmptyTest);
    suite.addTest('findLastIndex test 1', findLastIndexTest1);
    suite.addTest('findLastIndex test 2', findLastIndexTest2);
    suite.addTest('findLastIndex test 3', findLastIndexTest3);
    suite.addTest('findLastIndex test 4', findLastIndexTest4);
    suite.addTest('findLastIndex test 5', findLastIndexTest5);
    return suite.run();
}
