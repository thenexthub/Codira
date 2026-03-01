/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

class Arrays {
  static copy<T>(src: Array<T>): Array<T> {
    const copy = new Array<T>(src.length);
    for (let i = 0;i < src.length;i++) copy[i] = src[i];
    return copy;
  }

  static copy(src: FixedArray<Number>): Array<Number> {
    const copy = new Array<Number>(src.length);
    for (let i = 0;i < src.length;i++) copy[i] = src[i];
    return copy;
  }

  static equals<T>(left: ReadonlyArray<T>, right: ReadonlyArray<T>): boolean {
    if (left.length != right.length) return false;

    for (let i = 0;i < left.length;i++) {
      if (left[i] != right[i]) return false;
    }

    return true;
  }

  static equals<T>(left: Array<T>, right: Array<T>): boolean {
    const leftReadOnly = left as ReadonlyArray<T>;
    const rightReadOnly = right as ReadonlyArray<T>;
    return Arrays.equals(leftReadOnly, rightReadOnly);
  }

    // NOTE(smirnovandrei) may be left: ArrayLike can help to avoid this code duplication (the same code as equals(ReadonlyArray))
  static equals<T>(left: FixedArray<Number>, right: Array<T>): boolean {
    if (left.length != right.length) return false;

    for (let i = 0;i < left.length;i++) {
      if (left[i] != right[i]) return false;
    }

    return true;
  }
}

const TEST_ARRAY: FixedArray<Number> = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];

function main(): int {
    const suite = new arktest.ArkTestsuite('ReadonlyArray tests');
    suite.addTest('findWithFuncArg3ReadonlyArray', findWithFuncArg3ReadonlyArray);
    suite.addTest('findIndexFuncArg3ReadonlyArray', findIndexFuncArg3ReadonlyArray);
    suite.addTest('mapFuncArg3ReadonlyArray', mapFuncArg3ReadonlyArray);
    suite.addTest('everyFuncArg3ReadonlyArray', everyFuncArg3ReadonlyArray);
    suite.addTest('filterFuncArg3ReadonlyArray', filterFuncArg3ReadonlyArray);
    suite.addTest('reduceFuncArg4ReadonlyArray', reduceFuncArg4ReadonlyArray);
    suite.addTest('reduceFuncArg4ReadonlyArrayWithInitialValue', reduceFuncArg4ReadonlyArrayWithInitialValue);
    suite.addTest('reduceRightFuncArg4ReadonlyArray', reduceRightFuncArg4ReadonlyArray);
    suite.addTest('reduceRightFuncArg4ReadonlyArrayWithInitialValue', reduceRightFuncArg4ReadonlyArrayWithInitialValue);
    suite.addTest('forEachFuncArg3ReadonlyArray', forEachFuncArg3ReadonlyArray);
    suite.addTest('testIteratorWithEmptyReadonlyArray', testIteratorWithEmptyReadonlyArray);
    suite.addTest('testIteratorWithNonEmptyReadonlyArray', testIteratorWithNonEmptyReadonlyArray);
    suite.addTest('testLastIndexOf', testLastIndexOf);
    return suite.run();
}

function filterPositiveElement(elem: Number, index: int, arr: ReadonlyArray<Number>): boolean {
    return arr[index] == elem && elem == 12 && index > 0;
}

function filterNegativeElement(elem: Number, index: int, arr: ReadonlyArray<Number>): boolean {
     return arr[index] == elem && elem < 0 && index > 0;
}

function filterUndefinedElement(elem: Number, index: int, arr: ReadonlyArray<Number>): boolean {
    return arr[index] == elem && elem == undefined && index < 0;
}

function filterNullElement(elem: Number, index: int, arr: ReadonlyArray<Number>) {
    return arr[index] == elem && elem == null && index > 0;
}

function findWithFuncArg3ReadonlyArray() {
    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    arktest.assertEQ(arr.find(filterPositiveElement), 12);
    arktest.assertEQ(arr.find(filterNegativeElement), -1);
    arktest.assertEQ(arr.find(filterUndefinedElement), undefined);
    arktest.assertEQ(arr.find(filterNullElement), undefined);
}

function findIndexFuncArg3ReadonlyArray() {
    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    arktest.assertEQ(arr.findIndex(filterPositiveElement), 9);
    arktest.assertEQ(arr.findIndex(filterNegativeElement), 1);
    arktest.assertEQ(arr.findIndex(filterUndefinedElement), -1);
    arktest.assertEQ(arr.findIndex(filterNullElement), -1);
}

function mapFuncArg3ReadonlyArray() {
    let mappingFunc = (v: Number, index: int, arr: ReadonlyArray<Number>): Number =>
        (index < 4) && (index < arr.length) ? Number.valueOf(arr[index] * arr[index]) : Number.valueOf(0);

    let arrEmpty: ReadonlyArray<Number> = new Array<Number>;
    let res0 = arrEmpty.map<Number>(mappingFunc);
    arktest.assertEQ(res0.length, arrEmpty.length);
    arktest.assertEQ(res0.length, 0);

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    let res = arr.map<Number>(mappingFunc);
    arktest.assertEQ(res.length, arr.length);
    for (let i: int = 0; i < res.length; ++i) {
        const expected = i < 4 ? Number.valueOf(arr[i] * arr[i]) : Number.valueOf(0);
        arktest.assertEQ(res[i], expected);
    }
}

function everyFuncArg3ReadonlyArray() {
    const emptyArray: ReadonlyArray<Number> = new Array<Number>;
    arktest.assertTrue(emptyArray.every(filterPositiveElement));

    const testArray: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    const everyResult = testArray.every((n: Number, i: int, ns: ReadonlyArray<Number>) => n >= -5 && n <= 237);
    arktest.assertTrue(everyResult);
}

function filterFuncArg3ReadonlyArray() {
    let fnTrue = (v: Number, index: int, arr: ReadonlyArray<Number>): boolean => {
        return arr[index] == v && v > -1000;
    }
    let arrEmpty = new Array<Number>;
    arktest.assertEQ(arrEmpty.filter(fnTrue).length, 0);

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    let res = arr.filter(fnTrue);

    arktest.assertTrue(Arrays.equals(res, arr));

    let fnFalse = (v: Number, index: int, arr: ReadonlyArray<Number>): boolean => {
        return arr.at(index) == v && v == undefined;
    }
    arktest.assertEQ(arr.filter(fnFalse).length, 0);
}

function reduceFunc(a: Number, b: Number, index: int, arr: ReadonlyArray<Number>): Number {
    if (index < 0 || index >= arr.length) {
        return Number.valueOf(-1);
    }
    return a + b + arr[index];
}

function reduceFuncArg4ReadonlyArray() {
    let arrEmpty: ReadonlyArray<Number> = new Array<Number>;
    arktest.expectThrow(() => { arrEmpty.reduce(reduceFunc); }, (e) => { return e instanceof TypeError; });

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    let res = arr.reduce(reduceFunc);
    arktest.assertEQ(res, 996);
}

// NOTE(smirnovandrei) add 'test' prefix to all test functions
function reduceFuncArg4ReadonlyArrayWithInitialValue() {
    let arrEmpty = new Array<Number>;
    let initialValue: number = 0;
    let res = arrEmpty.reduce(reduceFunc, initialValue);
    arktest.assertEQ(res, initialValue);

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    initialValue = -arr[0];
    res = arr.reduce(reduceFunc, initialValue);
    arktest.assertEQ(res, 996);
}

function reduceRightFuncArg4ReadonlyArray() {
    let arrEmpty: ReadonlyArray<Number> = new Array<Number>;
    arktest.expectThrow(() => { arrEmpty.reduceRight(reduceFunc); }, (e) => { return e instanceof TypeError; });

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    let res = arr.reduceRight(reduceFunc);
    arktest.assertEQ(res, 994);
}

function reduceRightFuncArg4ReadonlyArrayWithInitialValue() {
    let arrEmpty = new Array<Number>;
    let initialValue: number = 0;
    let res = arrEmpty.reduce(reduceFunc, initialValue);
    arktest.assertEQ(res, initialValue);

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    initialValue = -arr[0];
    res = arr.reduce(reduceFunc, initialValue);
    arktest.assertEQ(res, 996);
}

function forEachFuncArg3ReadonlyArray() {
    let s = new String;
    let fn = (a: Number, index: int, arr: ReadonlyArray<Number>): void => {
        if (index < 0 && index >= arr.length) {
            s = '';
        } else {
            let tmp = arr[index] + a;
            s += tmp.toString() + ',';
        }
    }

    const arr: ReadonlyArray<Number> = Arrays.copy(TEST_ARRAY);
    arr.forEach(fn);
    arktest.assertEQ(s, '20,-2,474,-10,296,130,6,68,0,24,');
}

function testIteratorWithEmptyReadonlyArray() {
    const emptyArray: ReadonlyArray<Number> = new Array<Number>;
    const iterator = emptyArray.$_iterator();
    const v = iterator.next();
    arktest.assertTrue(v.done);
}

function testIteratorWithNonEmptyReadonlyArray() {
    const arr: ReadonlyArray<Number> = new Array<Number>(1, 2, 3);
    const iterator = arr.$_iterator();

    let v = iterator.next();
    arktest.assertFalse(v.done);
    arktest.assertEQ(v.value, 1);

    v = iterator.next();
    arktest.assertFalse(v.done);
    arktest.assertEQ(v.value, 2);

    v = iterator.next();
    arktest.assertFalse(v.done);
    arktest.assertEQ(v.value, 3);

    v = iterator.next();
    arktest.assertTrue(v.done);
}

function testLastIndexOf() {
    let arr: readonly number[] = [1, 2, 3, 2, 4];
    let res = arr.lastIndexOf(2);
    arktest.assertEQ(res, 3);

    res = arr.lastIndexOf(2, arr.length);
    arktest.assertEQ(res, 3);

    res = arr.lastIndexOf(2, 2);
    arktest.assertEQ(res, 1);

    res = arr.lastIndexOf(2, undefined);
    arktest.assertEQ(res, -1);
}
