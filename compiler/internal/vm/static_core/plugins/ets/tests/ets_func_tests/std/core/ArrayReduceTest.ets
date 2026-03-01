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

function reduceEmptyTest() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.push(10);
        return prev + val;
    }
    let data: int[] = [];
    arktest.expectThrow(() => {
        data.reduce(func);
    }, (e: Error) => e instanceof TypeError);

    data = [];
    let res = data.reduce(func, 100);
    arktest.assertEQ(res, 100);
    arktest.assertEQ(data.toString(), '');
}

function reduceTest1() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.pop();
        return prev + val;
    };
    let data: int[] = [1, 2, 3, 4, 5];
    let res = data.reduce(func);
    arktest.assertEQ(res, 6);
    arktest.assertEQ(data.toString(), '1,2,3');

    data = [1, 2, 3, 4, 5];
    res = data.reduce(func, 100);
    arktest.assertEQ(res, 106);
    arktest.assertEQ(data.toString(), '1,2');
}

function reduceTest2() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.shift();
        return prev + val;
    }
    let data: int[] = [1, 2, 3, 4, 5];
    let res = data.reduce(func);
    arktest.assertEQ(res, 7);
    arktest.assertEQ(data.toString(), '3,4,5');

    data = [1, 2, 3, 4, 5];
    res = data.reduce(func, 100);
    arktest.assertEQ(res, 109);
    arktest.assertEQ(data.toString(), '4,5');
}

function reduceTest3() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        if (arr.length > 1) {
            arr.shift();
            arr.pop();
        }
        return prev + val;
    }
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.reduce(func);
    arktest.assertEQ(res, 7)
    arktest.assertEQ(data.toString(), '3,4');

    data = [1, 2, 3, 4, 5, 6];
    res = data.reduce(func, 100);
    arktest.assertEQ(res, 104)
    arktest.assertEQ(data.toString(), '3,4');

}

function reduceTest4() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.push(val + 10);
        return prev + val;
    }
    let data: int[] = [1, 2, 3, 4];
    let res = data.reduce(func);
    arktest.assertEQ(res, 10);
    arktest.assertEQ(data.toString(), '1,2,3,4,12,13,14');

    data = [1, 2, 3, 4];
    res = data.reduce(func, 100);
    arktest.assertEQ(res, 110);
    arktest.assertEQ(data.toString(), '1,2,3,4,11,12,13,14');
}

function reduceTest5() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.shift();
        arr.push(index + 10, index + 100);
        return prev + val;
    }
    let data: int[] = [-1, -1, -1, -1];
    let res = data.reduce(func);
    arktest.assertEQ(res, 98);
    arktest.assertEQ(data.toString(), '-1,11,101,12,102,13,103');

    data = [-1, -1, -1, -1];
    res = data.reduce(func, 100);
    arktest.assertEQ(res, 119);
    arktest.assertEQ(data.toString(), '10,100,11,101,12,102,13,103');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array length change tests');
    suite.addTest('reduce empty test', reduceEmptyTest);
    suite.addTest('reduce test 1', reduceTest1);
    suite.addTest('reduce test 2', reduceTest2);
    suite.addTest('reduce test 3', reduceTest3);
    suite.addTest('reduce test 4', reduceTest4);
    suite.addTest('reduce test 5', reduceTest5);
    return suite.run();
}
