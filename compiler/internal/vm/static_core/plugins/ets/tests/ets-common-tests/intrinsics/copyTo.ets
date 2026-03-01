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

function testArrayOfObjects() {
    let f1: FixedArray<int> = [1];
    let f2: FixedArray<int> = [2];
    let f3: FixedArray<int> = [3];
    let f4: FixedArray<int> = [4];
    let f5: FixedArray<int> = [5];
    let f6: FixedArray<int> = [6];
    let f7: FixedArray<int> = [7];

    let farr1: FixedArray<FixedArray<int>> = [f1, f2, f3];
    let farr2: FixedArray<FixedArray<int>> = [f4, f4, f4];

    copyTo(farr1, farr2, 0, 1, farr1.length);
    arktest.assertEQ(farr2[0], f2);
    arktest.assertEQ(farr2[0][0], 2);
    arktest.assertEQ(farr2[1], f3);
    arktest.assertEQ(farr2[2], f4); // has not been changed

    farr1[1][0] = 5;
    arktest.assertEQ(farr2[0][0], 5); // shallow copy: only the reference has been copied, not the object itself

    let f8: FixedArray<int> = [2];
    farr2 = [f1, f8, f3];
    farr1 = [f5, f6, f7];
    copyTo(farr2, farr1, 1, 0, farr2.length - 1);
    arktest.assertEQ(farr1[0], f5);
    arktest.assertEQ(farr1[0][0], 5);
    arktest.assertEQ(farr1[1], f1);
    arktest.assertEQ(farr1[2], f8);
}

function testArrayOfObjectsDoNothing() {
    let f1: FixedArray<int> = [1];
    let f2: FixedArray<int> = [2];
    let f3: FixedArray<int> = [3];
    let f4: FixedArray<int> = [4];

    let fempty: FixedArray<FixedArray<int>> = [];
    let farr1: FixedArray<FixedArray<int>> = [f1, f2, f3];
    let farr2: FixedArray<FixedArray<int>> = [f4, f4, f4];

    copyTo(farr1, farr2, 0, 1, 1); // different arrays, length is 0
    arktest.assertEQ(farr2[0], f4);
    arktest.assertEQ(farr2[1], f4);
    arktest.assertEQ(farr2[2], f4);

    copyTo(fempty, farr2, 0, 0, fempty.length); // different arrays, length is 0
    arktest.assertEQ(farr2[0], f4);
    arktest.assertEQ(farr2[1], f4);
    arktest.assertEQ(farr2[2], f4);

    copyTo(farr1, farr1, 0, 0, 1); // same arrays, srcPos == dstPos
    arktest.assertEQ(farr1[0], f1);
    arktest.assertEQ(farr1[1], f2);
    arktest.assertEQ(farr1[2], f3);

    copyTo(farr1, farr1, 1, 1, farr1.length); // same arrays, srcPos == dstPos
    arktest.assertEQ(farr1[0], f1);
    arktest.assertEQ(farr1[1], f2);
    arktest.assertEQ(farr1[2], f3);

    copyTo(farr1, farr2, 0, 1, 1); // different arrays, length is 0
    arktest.assertEQ(farr2[0], f4);
    arktest.assertEQ(farr2[1], f4);
    arktest.assertEQ(farr2[2], f4);

    copyTo(fempty, farr2, 0, 0, fempty.length); // different arrays, length is 0
    arktest.assertEQ(farr2[0], f4);
    arktest.assertEQ(farr2[1], f4);
    arktest.assertEQ(farr2[2], f4);

    copyTo(farr1, farr1, 0, 0, 1); // same arrays, srcPos == dstPos
    arktest.assertEQ(farr1[0], f1);
    arktest.assertEQ(farr1[1], f2);
    arktest.assertEQ(farr1[2], f3);

    copyTo(farr1, farr1, 1, 1, farr1.length); // same arrays, srcPos == dstPos
    arktest.assertEQ(farr1[0], f1);
    arktest.assertEQ(farr1[1], f2);
    arktest.assertEQ(farr1[2], f3);
}

function testArrayOfObjectsOverlap() {
    let f4: FixedArray<int> = [4];
    let f5: FixedArray<int> = [5];
    let f6: FixedArray<int> = [6];
    let f7: FixedArray<int> = [7];

    let farr2: FixedArray<FixedArray<int>> = [f4, f4, f4];
    let farr3: FixedArray<FixedArray<int>> = [f5, f6, f7];

    copyTo(farr3, farr3, 0, 1, farr3.length); // forward copy (src >= dst)
    arktest.assertEQ(farr3[0], f6);
    arktest.assertEQ(farr3[1], f7);
    arktest.assertEQ(farr3[2], f7);
    farr3[2][0] = -2;
    arktest.assertEQ(farr3[1][0], -2);
    arktest.assertEQ(farr3[2][0], -2);

    copyTo(farr2, farr2, 0, 1, farr2.length); // forward copy (src >= dst)
    arktest.assertEQ(farr2[0], f4);
    arktest.assertEQ(farr2[1], f4);
    arktest.assertEQ(farr2[2], f4);
    farr2[2][0] = -2;
    arktest.assertEQ(farr2[0][0], -2);
    arktest.assertEQ(farr2[1][0], -2);
    arktest.assertEQ(farr2[2][0], -2);

    let f8: FixedArray<int> = [7]
    farr3 = [f5, f6, f8];
    arktest.assertLE(2, farr3.length);
    copyTo(farr3, farr3, 1, 0, 2); // backward copy (src < dst)
    arktest.assertEQ(farr3[0], f5);
    arktest.assertEQ(farr3[1], f5);
    arktest.assertEQ(farr3[2], f6);
    arktest.assertEQ(farr3[0][0], 5);
    arktest.assertEQ(farr3[1][0], 5);
    arktest.assertEQ(farr3[2][0], 6);
}

let strings: FixedArray<string> = createNewStringArray(310000);

function initStringArray(arr: FixedArray<string>) {
    for (let i: int = 0; i < arr.length; i++) {
        arr[i] = strings[i];
    }
}

function resetStringArray(arr: FixedArray<string>) {
    let empty: string = "";
    for (let i: int = 0; i < arr.length; i++) {
        arr[i] = empty;
    }
}

function createSourceStringArray(size: int): FixedArray<string> {
    let arr: FixedArray<string> = new FixedArray<string>(size);
    initStringArray(arr);
    return arr;
}


function createNewStringArray(size: int): FixedArray<string> {
    let src: FixedArray<string> = new FixedArray<string>(100)
    for (let i = 0; i < src.length; i++) {
        src[i] = "s" + (i + 1);
    }
    let arr: FixedArray<string> = new FixedArray<string>(size)
    for (let i: int = 0; i < arr.length; i++) {
        arr[i] = src[(i + 1) % src.length];
    }
    return arr;
}

function checkStringArray(src: FixedArray<string>, dst: FixedArray<string>, offset: int, start?: int, end?: int) {
    const resolvedStart: int = start ?? 0;
    const resolvedEnd: int = end ?? src.length;
    arktest.assertLT(offset, resolvedEnd);
    arktest.assertLE(resolvedStart, resolvedEnd - offset);
    for (let i: int = resolvedStart; i < resolvedEnd - offset; i++) {
        arktest.assertEQ(src[i + offset], dst[i]);
    }
}

function testArrayOfObjectsBigLength() {
    let farr1: FixedArray<string> = createSourceStringArray(10240);
    let farr2: FixedArray<string> = new FixedArray<string>(farr1.length)
    copyTo(farr1, farr2, 0, 1, farr1.length);
    // checkStringArray dominates the test's running time and because it is a very slow operation (checking an array of
    // 300'000 strings takes about 80 seconds on x86-64 for the debug build), we need to shrink the checked range.
    // Instead of checking every element in the copied array, it is enough to check 1000 elements at the start,
    // at the end, and in the middle of the array.
    checkStringArray(farr1, farr2, 1, 0, 1000);
    checkStringArray(farr1, farr2, 1, farr1.length - 1000, farr1.length);
    checkStringArray(farr1, farr2, 1, farr1.length / 2 - 500, farr1.length / 2 + 500);
    initStringArray(farr2);
    resetStringArray(farr1);
    copyTo(farr2, farr1, 0, 1, farr2.length);
    checkStringArray(farr2, farr1, 1, 0, 1000);
    checkStringArray(farr2, farr1, 1, farr2.length - 1000, farr2.length);
    checkStringArray(farr2, farr1, 1, farr2.length / 2 - 500, farr2.length / 2 + 500);

    farr1 = createSourceStringArray(200000);
    farr2 = new FixedArray<string>(farr1.length)
    copyTo(farr1, farr2, 0, 1, farr1.length);
    checkStringArray(farr1, farr2, 1, 0, 1000);
    checkStringArray(farr1, farr2, 1, farr1.length - 1000, farr1.length);
    checkStringArray(farr1, farr2, 1, farr1.length / 2 - 500, farr1.length / 2 + 500);
    initStringArray(farr2);
    resetStringArray(farr1);
    copyTo(farr2, farr1, 0, 1, farr2.length);
    checkStringArray(farr2, farr1, 1, 0, 1000);
    checkStringArray(farr2, farr1, 1, farr2.length - 1000, farr2.length);
    checkStringArray(farr2, farr1, 1, farr2.length / 2 - 500, farr2.length / 2 + 500);

    farr1 = createSourceStringArray(300033);
    farr2 = new FixedArray<string>(farr1.length)
    copyTo(farr1, farr2, 0, 1, farr1.length);
    checkStringArray(farr1, farr2, 1, 0, 1000);
    checkStringArray(farr1, farr2, 1, farr1.length - 1000, farr1.length);
    checkStringArray(farr1, farr2, 1, farr1.length / 2 - 500, farr1.length / 2 + 500);
    initStringArray(farr2);
    resetStringArray(farr1);
    copyTo(farr2, farr1, 0, 1, farr2.length);
    checkStringArray(farr2, farr1, 1, 0, 1000);
    checkStringArray(farr2, farr1, 1, farr2.length - 1000, farr2.length);
    checkStringArray(farr2, farr1, 1, farr2.length / 2 - 500, farr2.length / 2 + 500);
}

function checkRevStringArray(src: FixedArray<string>, dst: FixedArray<string>, offset: int, checkStart: int,
                             checkEnd: int) {
    arktest.assertLE(checkStart, src.length);
    arktest.assertLE(checkEnd, src.length);
    arktest.assertLE(offset, src.length - checkStart);
    arktest.assertLE(src.length - checkStart, src.length - checkEnd);
    for (let i: int = src.length - checkStart; i < src.length - checkEnd; i++) {
        arktest.assertEQ(src[i - offset], dst[i]);
    }
}

function testArrayOfObjectsBigLengthOverlap() {
    let farr: FixedArray<String> = createSourceStringArray(10240);
    copyTo(farr, farr, 0, farr.length / 2, farr.length);
    checkRevStringArray(farr, farr, farr.length / 2, farr.length / 2, farr.length / 2 - 100);
    checkRevStringArray(farr, farr, farr.length / 2, farr.length / 4 + 50, farr.length / 4 - 50);
    checkRevStringArray(farr, farr, farr.length / 2, 100, 0);

    farr = createSourceStringArray(300002);
    const middle: string = farr[farr.length / 2];
    copyTo(farr, farr, 0, farr.length / 2, farr.length);
    // checkRevStringArray dominates the test's running time and because it is a very slow operation (checking an
    // array of 150'000 strings takes about 45 seconds on x86-64 for the debug build), we need to shrink the
    // checked range. Instead of checking every element in the copied array, it is enough to check 1000 elements
    // at the start, at the end, and in the middle of the array.
    checkRevStringArray(farr, farr, farr.length / 2, farr.length / 2, farr.length / 2 - 1000);
    checkRevStringArray(farr, farr, farr.length / 2, farr.length / 4 + 500, farr.length / 4 - 500);
    checkRevStringArray(farr, farr, farr.length / 2, 1000, 0);
    arktest.assertEQ(farr[0], middle);
}

function testArrayOfObjectsExceptions() {
    let f1: FixedArray<int> = [1]
    let farr1: FixedArray<FixedArray<int>> = [f1, f1];
    let farr2: FixedArray<FixedArray<int>> = [f1, f1];

    copyTo(farr1, farr2, 0, 1, 1); // srcStart == srcEnd - ok.

    arktest.expectThrow(() => {
        copyTo(farr1, farr2, 0, 1, 0); // srcStart > srcEnd
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError && e.message == 'copyTo: src bounds verification failed';
    });
    arktest.expectThrow(() => {
        copyTo(farr1, farr2, 0, farr1.length, 1); // srcStart == farr1.length
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError && e.message == 'copyTo: src bounds verification failed';
    });
    arktest.expectThrow(() => {
        copyTo(farr1, farr2, 0, 0, farr1.length + 1); // srcEnd > farr1.length
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError && e.message == 'copyTo: src bounds verification failed';
    });
    arktest.expectThrow(() => {
        copyTo(farr1, farr2, 0, -1, farr1.length); // srcStart < 0
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError && e.message == 'copyTo: src bounds verification failed';
    });
    arktest.expectThrow(() => {
        copyTo(farr1, farr2, -1, 0, farr1.length); // dstStart < 0
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError && e.message == 'copyTo: dst bounds verification failed';
    });
    arktest.expectThrow(() => {
        copyTo(farr1, farr2, farr2.length + 1, 0, farr1.length); // dstStart > farr2.length
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError && e.message == 'copyTo: dst bounds verification failed';
    });
    let farr3: FixedArray<FixedArray<int>> = [f1];
    arktest.expectThrow(() => {
        copyTo(farr1, farr3, 0, 0, farr1.length); // (srcEnd - srcStart) > (dstLen - dstStart)
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError && e.message == 'copyTo: Destination array doesn\'t have enough space';
    });
    let farr4: FixedArray<FixedArray<int>> = [f1, f1, f1];
    arktest.expectThrow(() => {
        copyTo(farr1, farr4, farr4.length - 1, 0, farr1.length); // (srcEnd - srcStart) > (dstLen - dstStart)
    }, (e: Error) => {
        return e instanceof ArrayIndexOutOfBoundsError && e.message == 'copyTo: Destination array doesn\'t have enough space';
    });
}

function fillArrays(srcarr: FixedArray<string>, dstarr: FixedArray<string>, objarr: FixedArray<string>,
                    pos1: int, pos2: int, num: int, overlap: boolean) {
    for (let i: int = 0; i < dstarr.length; i++) {
        srcarr[i] = "";
        dstarr[i] = "";
        objarr[i] = dstarr[i];
    }

    for (let i: int = 0; i < num; i++) {
        if (overlap) {
            objarr[pos2 + i] = dstarr[pos1 + i];
        } else {
            objarr[pos2 + i] = srcarr[pos1 + i];
        }
    }
}

function copyArrays(srcarr: FixedArray<string>, dstarr: FixedArray<string>,
                    pos1: int, pos2: int, num: int, overlap: boolean) {
    if (overlap) {
        copyTo(dstarr, dstarr, pos2, pos1, pos1 + num);
    } else {
        copyTo(srcarr, dstarr, pos2, pos1, pos1 + num);
    }
}

function checkArrays(objarr: FixedArray<string>, dstarr: FixedArray<string>) {
    arktest.assertEQ(objarr.length, objarr.length, 'Unexpected array length')
    for (let i: int = 0; i < objarr.length; i++) {
        arktest.assertEQ(objarr[i], dstarr[i], 'Unexpected element with index ' + i + ': expected \'' + objarr[i]
            + '\' but was \'' + dstarr[i] + '\'');
    }
}

function testArrayOfObjectsFull(overlap: boolean) {
    let srcarr: FixedArray<string> = new FixedArray<string>(10)
    let objarr: FixedArray<string> = new FixedArray<string>(10)
    let dstarr: FixedArray<string> = new FixedArray<string>(10)

    for (let pos1: int = 0; pos1 < srcarr.length; pos1++) {
        for (let pos2: int = 0; pos2 < srcarr.length; pos2++) {
            for (let num: int = 0; num <= srcarr.length; num++) {
                if (pos1 + num > srcarr.length || pos2 + num > srcarr.length)
                    continue;
                fillArrays(srcarr, dstarr, objarr, pos1, pos2, num, overlap);
                copyArrays(srcarr, dstarr, pos1, pos2, num, overlap);
                checkArrays(objarr, dstarr);
            }
        }
    }
}

function testArrayOfObjectsFullNoOverlap() {
    testArrayOfObjectsFull(false);
}

function testArrayOfObjectsFullOverlap() {
    testArrayOfObjectsFull(true);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('copyTo');
    testSuite.addTest('copyTo for arrays of objects', testArrayOfObjects);
    testSuite.addTest('copyTo for arrays of objects do nothing', testArrayOfObjectsDoNothing);
    testSuite.addTest('copyTo for arrays of objects overlap', testArrayOfObjectsOverlap);
    testSuite.addTest('copyTo for arrays of objects with exceptions', testArrayOfObjectsExceptions);
    testSuite.addTest('copyTo for arrays of objects big length', testArrayOfObjectsBigLength);
    testSuite.addTest('copyTo for arrays of objects big length overlap', testArrayOfObjectsBigLengthOverlap);
    testSuite.addTest('copyTo for arrays of objects full test', testArrayOfObjectsFullNoOverlap);
    testSuite.addTest('copyTo for arrays of objects full test overlap', testArrayOfObjectsFullOverlap);
    return testSuite.run();
}
