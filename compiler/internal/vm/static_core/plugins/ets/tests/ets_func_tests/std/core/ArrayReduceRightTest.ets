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

function reduceRightEmptyTest() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.push(10);
        return prev + val;
    }
    let data: int[] = [];
    arktest.expectThrow(() => {
        data.reduceRight(func);
    }, (e: Error) => e instanceof TypeError);

    data = [];
    let res = data.reduceRight(func, 100);
    arktest.assertEQ(res, 100);
    arktest.assertEQ(data.toString(), '');
}

function reduceRightTest1() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.pop();
        return prev + val;
    };
    let data: int[] = [1, 2, 3, 4, 5];
    let res = data.reduceRight(func);
    arktest.assertEQ(res, 15);
    arktest.assertEQ(data.toString(), '1');

    data = [1, 2, 3, 4, 5];
    res = data.reduceRight(func, 100);
    arktest.assertEQ(res, 115);
    arktest.assertEQ(data.toString(), '');
}

function reduceRightTest2() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.shift();
        return prev + val;
    }
    let data: int[] = [1, 2, 3, 4, 5];
    let res = data.reduceRight(func);
    arktest.assertEQ(res, 21);
    arktest.assertEQ(data.toString(), '5');

    data = [1, 2, 3, 4, 5];
    res = data.reduceRight(func, 100);
    arktest.assertEQ(res, 125);
    arktest.assertEQ(data.toString(), '');
}

function reduceRightTest3() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        if (arr.length > 1) {
            arr.shift();
            arr.pop();
        }
        return prev + val;
    }
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.reduceRight(func);
    arktest.assertEQ(res, 20)
    arktest.assertEQ(data.toString(), '');

    data = [1, 2, 3, 4, 5, 6];
    res = data.reduceRight(func, 100);
    arktest.assertEQ(res, 115)
    arktest.assertEQ(data.toString(), '');
}

function reduceRightTest4() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.push(val + 10);
        return prev + val;
    }
    let data: int[] = [1, 2, 3, 4];
    let res = data.reduceRight(func);
    arktest.assertEQ(res, 10);
    arktest.assertEQ(data.toString(), '1,2,3,4,13,12,11');

    data = [1, 2, 3, 4];
    res = data.reduceRight(func, 100);
    arktest.assertEQ(res, 110);
    arktest.assertEQ(data.toString(), '1,2,3,4,14,13,12,11');
}

function reduceRightTest5() {
    let func = (prev: int, val: int, index: int, arr: int[]) => {
        arr.shift();
        arr.push(index + 10, index + 100);
        return prev + val;
    }
    let data: int[] = [-1, -1, -1, -1];
    let res = data.reduceRight(func);
    arktest.assertEQ(res, -4);
    arktest.assertEQ(data.toString(), '-1,12,102,11,101,10,100');

    data = [-1, -1, -1, -1];
    res = data.reduceRight(func, 100);
    arktest.assertEQ(res, 96);
    arktest.assertEQ(data.toString(), '13,103,12,102,11,101,10,100');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array length change tests');
    suite.addTest('reduceRight empty test', reduceRightEmptyTest);
    suite.addTest('reduceRight test 1', reduceRightTest1);
    suite.addTest('reduceRight test 2', reduceRightTest2);
    suite.addTest('reduceRight test 3', reduceRightTest3);
    suite.addTest('reduceRight test 4', reduceRightTest4);
    suite.addTest('reduceRight test 5', reduceRightTest5);
    return suite.run();
}
