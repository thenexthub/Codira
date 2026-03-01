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

function forEachEmptyTest() {
    let data: int[] = [];
    data.forEach((val, index, arr) => {
        arr.push(10);
    });
    arktest.assertEQ(data.toString(), '');
}

function forEachTest1() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res: int[] = [];
    data.forEach((val, index, arr) => {
        arr.pop();
        res.push(val);
    });
    arktest.assertEQ(res.toString(), '1,2,3');
    arktest.assertEQ(data.toString(), '1,2,3');
}

function forEachTest2() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res: int[] = [];
    data.forEach((val, index, arr) => {
        arr.shift();
        res.push(val);
    });
    arktest.assertEQ(res.toString(), '1,3,5');
    arktest.assertEQ(data.toString(), '4,5,6');
}

function forEachTest3() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res: int[] = [];
    data.forEach((val, index, arr) => {
        if (arr.length > 1) {
            arr.shift();
            arr.pop();
        }
        res.push(val);
    });
    arktest.assertEQ(res.toString(), '1,3');
    arktest.assertEQ(data.toString(), '3,4');
}

function forEachTest4() {
    let data: int[] = [1, 2, 3, 4];
    let res: int[] = [];
    data.forEach((val, index, arr) => {
        arr.push(val + 10);
        res.push(val);
    });
    arktest.assertEQ(res.toString(), '1,2,3,4');
    arktest.assertEQ(data.toString(), '1,2,3,4,11,12,13,14');
}

function forEachTest5() {
    let data: int[] = [-1, -1, -1, -1];
    let res: int[] = [];
    data.forEach((val, index, arr) => {
        arr.shift();
        arr.push(index + 10, index + 15);
        res.push(val);
    });
    arktest.assertEQ(res.toString(), '-1,-1,10,11');
    arktest.assertEQ(data.toString(), '10,15,11,16,12,17,13,18');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array length change tests');
    suite.addTest('forEach empty test', forEachEmptyTest);
    suite.addTest('forEach test 1', forEachTest1);
    suite.addTest('forEach test 2', forEachTest2);
    suite.addTest('forEach test 3', forEachTest3);
    suite.addTest('forEach test 4', forEachTest4);
    suite.addTest('forEach test 5', forEachTest5);
    return suite.run();
}
