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

function someEmptyTest() {
    let data: int[] = [];
    let res = data.some((val, index, arr) => { return val != 0; });
    arktest.assertFalse(res);
    arktest.assertEQ(data.toString(), '');
}

function someTest1() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.some((val, index, arr) => {
        arr.pop();
        return val % 3 == 0;
    });
    arktest.assertTrue(res);
    arktest.assertEQ(data.toString(), '1,2,3');
}

function someTest2() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.some((val, index, arr) => {
        arr.shift();
        return val > 5;
    });
    arktest.assertFalse(res);
    arktest.assertEQ(data.toString(), '4,5,6');
}

function someTest3() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.some((val, index, arr) => {
        if (arr.length > 1) {
            arr.shift();
            arr.pop();
        }
        return val == 3;
    });
    arktest.assertTrue(res);
    arktest.assertEQ(data.toString(), '3,4');
}

function someTest4() {
    let data: int[] = [1, 2, 3, 4];
    let res = data.some((val, index, arr) => {
        arr.push(val + 10);
        return val == 4;
    });
    arktest.assertTrue(res);
    arktest.assertEQ(data.toString(), '1,2,3,4,11,12,13,14');
}

function someTest5() {
    let data: int[] = [-1, -1, -1, -1];
    let res = data.some((val, index, arr) => {
        arr.shift();
        arr.push(index + 10, index + 100);
        return val > 100;
    });
    arktest.assertFalse(res);
    arktest.assertEQ(data.toString(), '10,100,11,101,12,102,13,103');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array length change tests');
    suite.addTest('some empty test', someEmptyTest);
    suite.addTest('some test 1', someTest1);
    suite.addTest('some test 2', someTest2);
    suite.addTest('some test 3', someTest3);
    suite.addTest('some test 4', someTest4);
    suite.addTest('some test 5', someTest5);
    return suite.run();
}
