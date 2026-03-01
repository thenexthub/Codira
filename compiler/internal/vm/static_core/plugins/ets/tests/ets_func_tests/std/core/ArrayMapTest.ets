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

function mapEmptyTest() {
    let data: int[] = [];
    let res = data.map<string>((val, index, arr) => {
        arr.push(10);
        return String(val);
    });
    arktest.assertEQ(res.toString(), '');
    arktest.assertEQ(data.toString(), '');
}

function mapTest1() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.map<string>((val, index, arr) => {
        arr.pop();
        return String(val);
    });
    arktest.assertEQ(res.toString(), '1,2,3,,,');
    arktest.assertEQ(data.toString(), '1,2,3');
}

function mapTest2() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.map<string>((val, index, arr) => {
        arr.shift();
        return String(val);
    });
    arktest.assertEQ(res.toString(), '1,3,5,,,');
    arktest.assertEQ(data.toString(), '4,5,6');
}

function mapTest3() {
    let data: int[] = [1, 2, 3, 4, 5, 6];
    let res = data.map<string>((val, index, arr) => {
        if (arr.length > 1) {
            arr.shift();
            arr.pop();
        }
        return String(val);
    });
    arktest.assertEQ(res.toString(), '1,3,,,,');
    arktest.assertEQ(data.toString(), '3,4');
}

function mapTest4() {
    let data: int[] = [1, 2, 3, 4];
    let res = data.map<string>((val, index, arr) => {
        arr.push(val + 10);
        return String(val);
    });
    arktest.assertEQ(res.toString(), '1,2,3,4');
    arktest.assertEQ(data.toString(), '1,2,3,4,11,12,13,14');
}

function mapTest5() {
    let data: int[] = [-1, -1, -1, -1];
    let res = data.map<string>((val, index, arr) => {
        arr.shift();
        arr.push(index + 10, index + 15);
        return String(val);
    });
    arktest.assertEQ(res.toString(), '-1,-1,10,11');
    arktest.assertEQ(data.toString(), '10,15,11,16,12,17,13,18');
}

function main(): int {
    const suite = new arktest.ArkTestsuite('Array length change tests');
    suite.addTest('map empty test', mapEmptyTest);
    suite.addTest('map test 1', mapTest1);
    suite.addTest('map test 2', mapTest2);
    suite.addTest('map test 3', mapTest3);
    suite.addTest('map test 4', mapTest4);
    suite.addTest('map test 5', mapTest5);
    return suite.run();
}
