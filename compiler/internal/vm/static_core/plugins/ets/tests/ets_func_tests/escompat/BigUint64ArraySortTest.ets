/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function sortTest() {
    let arr = [18446744073709551615n, 9223372036854775808n, 9223372036854775807n, 9223372036854775806n, 20n, 10n, 1n]
    let arr1 = new BigUint64Array(arr);
    arr1.sort();
    let arr2 = new BigUint64Array(arr);
    arr2.sort((a, b) => a - b);

    let len = arr.length;
    for (let i = 0; i < len; i++) {
        arktest.assertEQ(arr[len - i - 1], arr1[i]);
        arktest.assertEQ(arr[len - i - 1], arr2[i]);
    }
}

function sortTestWithCustomComparator() {
    let arr = new BigUint64Array([100n, 50n, 200n, 25n]);
    arr.sort((a, b) => {
        if (a < b) {
            return -1;
        }
        if (a > b) {
            return 1;
        }
        return 0;
    });
    arktest.assertEQ(arr[0], 25n);
    arktest.assertEQ(arr[1], 50n);
    arktest.assertEQ(arr[2], 100n);
    arktest.assertEQ(arr[3], 200n);
}

function sortTestEmpty() {
    let arr = new BigUint64Array(0);
    arr.sort();
    arktest.assertEQ(arr.length, 0);
}

function sortTestSingleElement() {
    let arr = new BigUint64Array([42n]);
    arr.sort();
    arktest.assertEQ(arr.length, 1);
    arktest.assertEQ(arr[0], 42n);
}

function sortTestMaxValues() {
    let arr = new BigUint64Array([0n, 18446744073709551615n, 1n]);
    arr.sort();
    arktest.assertEQ(arr[0], 0n);
    arktest.assertEQ(arr[1], 1n);
    arktest.assertEQ(arr[2], 18446744073709551615n);
}

function main(): int {
    const suite = new arktest.ArkTestsuite('BigUint64Array sort');
    suite.addTest('sortTest', sortTest);
    suite.addTest('sortTestWithCustomComparator', sortTestWithCustomComparator);
    suite.addTest('sortTestEmpty', sortTestEmpty);
    suite.addTest('sortTestSingleElement', sortTestSingleElement);
    suite.addTest('sortTestMaxValues', sortTestMaxValues);
    return suite.run();
}
