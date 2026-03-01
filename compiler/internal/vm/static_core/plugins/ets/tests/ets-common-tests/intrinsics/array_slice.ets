/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

function checkElem<T>(elem: T, expected: T, idx: int): void {
    arktest.assertEQ(elem, expected, 'Unexpected element with index ' + idx + ': expected \'' + expected
        + '\' but was \'' + elem + '\'');
}

function checkArray<T extends Number | String>(data: ArrayLike<T>, expected: ArrayLike<T>, start?: int,
                                               end?: int): void {
    arktest.assertEQ(data.length, expected.length, 'Unexpected array length')
    const resolvedStart: int = start ?? 0;
    arktest.assertLE(0, resolvedStart);
    const resolvedEnd: int = end ?? expected.length;
    arktest.assertLE(resolvedEnd, expected.length);
    for (let i: int = resolvedStart; i < resolvedEnd; i++) {
        checkElem(data[i], expected[i], i);
    }
}

function checkArray(data: FixedArray<int>, expected: FixedArray<int>): void {
    arktest.assertEQ(data.length, expected.length, 'Unexpected array length')
    for (let i: int = 0; i < data.length; i++) {
        checkElem(data[i], expected[i], i);
    }
}

function testArrayOfString() {
    checkArray(new Array<string>().slice(), new Array<string>());
    checkArray(Array.of("hello").slice(), Array.of("hello"));
    let arr1: Array<string> = Array.of(String("hello"), String("world"));
    let arr2: Array<string> = arr1.slice();
    checkArray(arr2, arr1);
    arktest.assertNE(arr1, arr2, "slice() must return a copy of the array");
}

function testArrayOfStringWithGC() {
    let arr1: Array<string> = Array.of(String("hello"), String("world"), String("hello"), String("world"),
        String("mars"));
    let arr2: Array<string> = arr1.slice();
    checkArray(arr2, arr1);
    arktest.assertNE(arr1, arr2, "slice() must return a copy of the array");
    GC.startGC(GC.Cause.FULL);
    GC.startGC(GC.Cause.FULL);
}

function testFixedArrayOfInt() {
    let empty: FixedArray<int> = new FixedArray<int>(0);
    checkArray(slice(empty), new FixedArray<int>(0));
    let arr1: FixedArray<int> = [-10, -10];
    checkArray(slice(arr1), [-10, -10]);
    let arr2: FixedArray<int> = [5, 6];
    let arr3: FixedArray<int> = slice(arr2);
    checkArray(arr3, arr2);
    arktest.assertNE(arr3, arr2, "slice() must return a copy of the array");
}

function createNewStringArray(size: int): Array<string> {
    let src: FixedArray<string> = new FixedArray<string>(1000);
    for (let i = 0; i < src.length; i++) {
        src[i] = "s" + (i + 1);
    }
    let arr: Array<string> = new Array<string>(size);
    for (let i = 0; i < arr.length; i++) {
        arr[i] = src[(i + 1) % src.length];
    }
    return arr;
}

function resetStringArray(arr: Array<string>) {
    let empty: string = "";
    for (let i: int = 0; i < arr.length; i++) {
        arr[i] = empty;
    }
}

function testLargeArrayOfString() {
    let arr1: Array<string> = createNewStringArray(200003);
    let arr2: Array<string> = arr1.slice();
    checkArray(arr2, arr1, 0, 10);
    checkArray(arr2, arr1, arr1.length - 10, arr1.length);
    checkArray(arr2, arr1, arr1.length / 2 - 5, arr1.length / 2 + 5);
    arktest.assertNE(arr1, arr2, "slice() must return a copy of the array");

    resetStringArray(arr1);
    arr1 = arr2.slice()
    checkArray(arr1, arr2, 0, 10);
    checkArray(arr1, arr2, arr2.length - 10, arr2.length);
    checkArray(arr1, arr2, arr2.length / 2 - 5, arr2.length / 2 + 5);
    arktest.assertNE(arr2, arr1, "slice() must return a copy of the array");
}

function testArrayOfStringSinglePageOf4Bytes() {
    // PageSize is 4096 bytes, when sizeof(ObjectPointerType) == 4, we have 1024 object pointers in one page
    let arr1: Array<string> = createNewStringArray(1024 + 1);
    let arr2: Array<string> = arr1.slice();
    checkArray(arr2, arr1);

    resetStringArray(arr2);
    arr1 = createNewStringArray(1024);
    arr2 = arr1.slice();
    checkArray(arr2, arr1);

    resetStringArray(arr2);
    arr1 = createNewStringArray(1024 - 1);
    arr2 = arr1.slice();
    checkArray(arr2, arr1);
}

function testArrayOfStringSinglePageOf8Bytes() {
    // PageSize is 4096 bytes, when sizeof(ObjectPointerType) == 8, we have 512 object pointers in one page
    let arr1: Array<string> = createNewStringArray(512 + 1);
    let arr2: Array<string> = arr1.slice();
    checkArray(arr2, arr1);

    resetStringArray(arr2);
    arr1 = createNewStringArray(512);
    arr2 = arr1.slice();
    checkArray(arr2, arr1);

    resetStringArray(arr2);
    arr1 = createNewStringArray(512 - 1);
    arr2 = arr1.slice();
    checkArray(arr2, arr1);
}

function testArrayOfStringStartIndex() {
    checkArray(Array.of("hello").slice(0), Array.of("hello"));
    let arr1: Array<string> = Array.of(String("hello"), String("world"));
    let arr2: Array<string> = arr1.slice(0);
    checkArray(arr2, arr1);
    arktest.assertNE(arr1, arr2, "slice() must return a copy of the array");
    let arr3: Array<string> = arr1.slice(1);
    checkArray(arr3, Array.of(String("world")));
    let arr4: Array<string> = arr1.slice(2);
    checkArray(arr4, new Array<string>());
    let arr5: Array<string> = arr1.slice(3);
    checkArray(arr5, new Array<string>());
    let arr6: Array<string> = arr1.slice(-1);
    checkArray(arr6, Array.of(String("world")));
    let arr7: Array<string> = arr1.slice(-2);
    checkArray(arr7, Array.of(String("hello"), String("world")));
    arktest.assertNE(arr1, arr7, "slice() must return a copy of the array");
    let arr8: Array<string> = arr1.slice(-arr1.length);
    checkArray(arr8, Array.of(String("hello"), String("world")));
    arktest.assertNE(arr1, arr8, "slice() must return a copy of the array");
    let arr9: Array<string> = arr1.slice(-(arr1.length + 1));
    checkArray(arr9, Array.of(String("hello"), String("world")));
    arktest.assertNE(arr1, arr9, "slice() must return a copy of the array");
}

function testFixedArrayOfIntStartIndex() {let arr1: FixedArray<int> = [42];
    checkArray(slice(arr1, 0), arr1);
    let arr2: FixedArray<int> = [42, -1];
    let arr3: FixedArray<int> = slice(arr2, 0);
    checkArray(arr3, arr2);
    arktest.assertNE(arr2, arr3, "slice() must return a copy of the array");
    let arr4: FixedArray<int> = slice(arr2, 1);
    checkArray(arr4, [-1]);
    let arr5: FixedArray<int> = slice(arr2, 2);
    checkArray(arr5, new FixedArray<int>(0));
    let arr6: FixedArray<int> = slice(arr2, 3);
    checkArray(arr6, new FixedArray<int>(0));
    let arr7: FixedArray<int> = slice(arr2, -1);
    checkArray(arr7, [-1]);
    let arr8: FixedArray<int> = slice(arr2, -2);
    checkArray(arr8, [42, -1]);
    arktest.assertNE(arr2, arr8, "slice() must return a copy of the array");
    let arr9: FixedArray<int> = slice(arr2, -arr2.length);
    checkArray(arr9, [42, -1]);
    arktest.assertNE(arr2, arr9, "slice() must return a copy of the array");
    let arr10: FixedArray<int> = slice(arr2, -(arr2.length + 1));
    checkArray(arr10, [42, -1]);
    arktest.assertNE(arr2, arr10, "slice() must return a copy of the array");
}

function testArrayOfStringStartIndexEndIndex() {
    let arr1: Array<string> = Array.of(String("hello"), String("world"));
    let arr2: Array<string> = arr1.slice(0, arr1.length);
    checkArray(arr2, arr1);
    arktest.assertNE(arr1, arr2, "slice() must return a copy of the array");
    let arr3: Array<string> = arr1.slice(0, arr1.length + 1);
    checkArray(arr3, arr1);
    arktest.assertNE(arr1, arr3, "slice() must return a copy of the array");

    let arr4: Array<string> = arr1.slice(1, arr1.length);
    checkArray(arr4, Array.of(String("world")));
    arktest.assertLE(2, arr1.length)
    let arr5: Array<string> = arr1.slice(0, 1);
    checkArray(arr5, Array.of(String("hello")));

    let arr6: Array<string> = Array.of(String("hello"), String("world"), String("the"), String("new"));
    let arr7: Array<string> = arr6.slice(-1, arr6.length);
    checkArray(arr7, Array.of(String("new")));
    let arr8: Array<string> = arr6.slice(-2, arr6.length);
    checkArray(arr8, Array.of(String("the"), String("new")));

    let arrminus1: Array<string> = arr6.slice(-1, -2);
    arktest.assertEQ(arrminus1.length, 0);
    let arrminus2: Array<string> = arr6.slice(2, 1);
    arktest.assertEQ(arrminus2.length, 0);
}

function testFixedArrayOfIntStartIndexEndIndex() {
    let arr1: FixedArray<int> = [-42, 10];
    let arr2: FixedArray<int> = slice(arr1, 0, arr1.length);
    checkArray(arr2, arr1);
    arktest.assertNE(arr1, arr2, "slice() must return a copy of the array");
    let arr3: FixedArray<int> = slice(arr1, 0, arr1.length + 1);
    checkArray(arr3, arr1);
    arktest.assertNE(arr1, arr3, "slice() must return a copy of the array");

    let arr4: FixedArray<int> = slice(arr1, 1, arr1.length);
    checkArray(arr4, [10]);
    arktest.assertLE(2, arr1.length)
    let arr5: FixedArray<int> = slice(arr1, 0, 1);
    checkArray(arr5, [-42]);

    let arr6: FixedArray<int> = [42, -1, 0, -4]
    let arr7: FixedArray<int> = slice(arr6, -1, arr6.length);
    checkArray(arr7, [-4]);
    let arr8: FixedArray<int> = slice(arr6, -2, arr6.length);
    checkArray(arr8, [0, -4]);

    let arrminus1: FixedArray<int> = slice(arr6, -1, -2);
    arktest.assertEQ(arrminus1.length, 0);
    let arrminus2: FixedArray<int> = slice(arr6, 2, 1);
    arktest.assertEQ(arrminus2.length, 0);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('Array');
    testSuite.addTest('slice for arrays', testArrayOfString);
    testSuite.addTest('slice for arrays with GC', testArrayOfStringWithGC);
    testSuite.addTest('slice for large arrays', testLargeArrayOfString);
    testSuite.addTest('slice for a single page of 4 bytes', testArrayOfStringSinglePageOf4Bytes);
    testSuite.addTest('slice for a single page of 8 bytes', testArrayOfStringSinglePageOf8Bytes);
    testSuite.addTest('slice for arrays with start', testArrayOfStringStartIndex);
    testSuite.addTest('slice for arrays with start and end', testArrayOfStringStartIndexEndIndex);
    let arrayRes: int = testSuite.run();

    testSuite = new arktest.ArkTestsuite('FixedArray<int>');
    testSuite.addTest('slice', testFixedArrayOfInt);
    testSuite.addTest('slice with start', testFixedArrayOfIntStartIndex);
    testSuite.addTest('slice with start and end', testFixedArrayOfIntStartIndexEndIndex);
    return arrayRes + testSuite.run();
}
