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


// TODO: these FixedArray<int> algorithms could be re-done using Generics
//       for better coverage after F<T implements Comparable<T>>
//       will be implemented in stdlib

// TODO: lambda doesn't work currently
// type inplaceSortFunc = (x: FixedArray<int>) => void;

const maxInt: int = 2147483647;
const minInt: int = -2147483648;

const tests: FixedArray<FixedArray<FixedArray<int>>> = [
    [[8, 3, 1, 2, 3, 1, 0], [0, 1, 1, 2, 3, 3, 8]],
    [[], []],
    [[1], [1]],
    [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0]],
    [[4+4, 3, +1, 2, +3, 1, -0, +0, -1], [-1, 0, 0, 1, 1, 2, 3, 3, 8]],
    [
        [maxInt, -minInt, maxInt, -minInt],
        [-minInt, -minInt, maxInt, maxInt]
    ]
];

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name);
        return 0;
    }
    console.println("FAILED: " + name);
    return 1;
}

function bubble_sort(arr: FixedArray<int>): void {
        for (let i = 0; i + 1 < arr.length; ++i) {
        for (let j = 0; j + 1 < arr.length - i; ++j) {
            if (arr[j + 1] < arr[j]) {
                swap(arr, j, j + 1);
            }
        }
    }
}

function shaker_sort(arr: FixedArray<int>): void {
    if (arr.length == 0) {
        return;
    }
    let left: int = 0;
    let right: int = arr.length - 1;
    while (left <= right) {
        for (let i: int = right; i > left; --i) {
            if (arr[i - 1] > arr[i]) {
                swap(arr, i - 1, i);
            }
        }
        ++left;
        for (let i: int = left; i < right; ++i) {
            if (arr[i] > arr[i + 1]) {
                swap(arr, i, i + 1);
            }
        }
        --right;
    }
}

function comb_sort(arr: FixedArray<int>): void {
    const factor: float = 1.2473309f;
    let step: int = arr.length - 1;
    while (step >= 1) {
        for (let i: int = 0; i + step < arr.length; ++i) {
            if (arr[i] > arr[i + step]) {
                swap(arr, i, i + step);
            }
        }
        step = (step / factor).toInt();
    }
    for (let i: int = 0; i + 1 < arr.length; ++i) {
        for (let j = 0; j + 1 < arr.length - i; ++j) {
            if (arr[j + 1] < arr[j]) {
                swap(arr, j, j + 1);
            }
        }
    }
}

function insertion_sort(arr: FixedArray<int>): void {
    for (let i: int = 1; i < arr.length; ++i) {
        let x = arr[i];
        let j = i;
        while (j > 0 && arr[j - 1] > x) {
            arr[j] = arr[j - 1];
            --j;
        }
        arr[j] = x;
    }
}

function cmp(x: FixedArray<int>, y: FixedArray<int>): boolean {
    if (x.length != y.length) {
        return false;
    }
    for (let i: int = 0; i < x.length; i++) {
        if (x[i] != y[i]) {
            return false;
        }
    }
    return true;
}

function quick_sort(arr: FixedArray<int>): FixedArray<int> {
    if (arr.length <= 1) {
        return arr;
    }
    let pivot = arr[0];
    let leftArr: FixedArray<int> = [];
    let rightArr: FixedArray<int> = [];
    for (let i: int = 1; i < arr.length; i++) {
        if (arr[i] < pivot) {
            leftArr = concat(leftArr, [arr[i]]);
        } else {
            rightArr = concat(rightArr, [arr[i]]);
        }
    }
    let tmp = quick_sort(leftArr);
    tmp = concat(tmp, [pivot]);
    return concat(tmp, quick_sort(rightArr));
}

function test_quick_sort(data: FixedArray<FixedArray<int>>): boolean {
    const unsorted: FixedArray<int> = copyOf(data[0]);
    let result: FixedArray<int> = quick_sort(unsorted);
    return cmp(result, data[1]);
}

function test_bubble_sort(data: FixedArray<FixedArray<int>>): boolean {
    let unsorted: FixedArray<int> = copyOf(data[0]);
    bubble_sort(unsorted);
    return cmp(unsorted, data[1]);
}

function test_shaker_sort(data: FixedArray<FixedArray<int>>): boolean {
    let unsorted: FixedArray<int> = copyOf(data[0]);
    shaker_sort(unsorted);
    return cmp(unsorted, data[1]);
}

function test_comb_sort(data: FixedArray<FixedArray<int>>): boolean {
    let unsorted: FixedArray<int> = copyOf(data[0]);
    comb_sort(unsorted);
    return cmp(unsorted, data[1]);
}

function test_insertion_sort(data: FixedArray<FixedArray<int>>): boolean {
    let unsorted: FixedArray<int> = copyOf(data[0]);
    insertion_sort(unsorted);
    return cmp(unsorted, data[1]);
}

function test_stdlib_sort(data: FixedArray<FixedArray<int>>): boolean {
    let unsorted: FixedArray<int> = copyOf(data[0]);
    sort(unsorted);
    return cmp(unsorted, data[1]);
}

function main(): int {
    let failures: int = 0;
    for (let i=0; i<tests.length; i++) {
        failures += test(test_quick_sort(tests[i]), "Quick Sort " + i);
        failures += test(test_bubble_sort(tests[i]), "Bubble Sort " + i);
        failures += test(test_shaker_sort(tests[i]), "Shaker Sort " + i);
        failures += test(test_comb_sort(tests[i]), "Comb Sort " + i);
        failures += test(test_insertion_sort(tests[i]), "Insert Sort " + i);
        failures += test(test_stdlib_sort(tests[i]), "StdLib Sort " + i);
    }
    if (failures == 0) {
        console.println("PASSED: All tests run");
    } else {
        console.println("FAILED: All tests run");
    }
    return failures;
}
