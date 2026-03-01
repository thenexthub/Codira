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

const success : int = 0;
const fail    : int = 1;

function main(): int {
    let failures: int = 0;
    //NOTE (ikorobkov): below tests have no implementation/support in Array
    failures += check((): int => { return createWithArrayOfNumber()}, "Create Array with given first item and Number[]");
    failures += check((): int => { return findWithFuncArg3ReadonlyArray()}, "findWithFuncArg3ReadonlyArray");
    failures += check((): int => { return findIndexFuncArg3ReadonlyArray()}, "findIndexFuncArg3ReadonlyArray");
    failures += check((): int => { return mapFuncArg3ReadonlyArray()}, "mapFuncArg3ReadonlyArray");
    failures += check((): int => { return everyFuncArg3ReadonlyArray()}, "everyFuncArg3ReadonlyArray");
    failures += check((): int => { return fromArg1()}, "fromArg1");
    failures += check((): int => { return fromArg2()}, "fromArg2");
    failures += check((): int => { return fromArrayLikeAndIterable()}, "fromArrayLikeAndIterable");
    failures += check((): int => { return fromArrayLikeAndIterableAndFunc()}, "fromArrayLikeAndIterableAndFunc");
    failures += check((): int => { return filterFuncArg3ReadonlyArray()}, "filterFuncArg3ReadonlyArray");
    failures += check((): int => { return reduceFuncArg4ReadonlyArray()}, "reduceFuncArg4ReadonlyArray");
    failures += check((): int => { return reduceFuncArg4ReadonlyArrayWithInitialValue()}, "reduceFuncArg4ReadonlyArrayWithInitialValue");
    failures += check((): int => { return reduceRightFuncArg4ReadonlyArray()}, "reduceRightFuncArg4ReadonlyArray");
    failures += check((): int => { return reduceRightFuncArg4ReadonlyArrayWithInitialValue()}, "reduceRightFuncArg4ReadonlyArrayWithInitialValue");
    failures += check((): int => { return forEachFuncArg3ReadonlyArray()}, "forEachFuncArg3ReadonlyArray");
    failures += check((): int => { return flatMapFuncArg3()}, "flatMapFuncArg3");
    failures += check((): int => { return flatMapFuncArg2()}, "flatMapFuncArg2");
    failures += check((): int => { return flatMapFuncArg1()}, "flatMapFuncArg1");
    failures += check((): int => { return flat()}, "flat");
    failures += check((): int => { return flatDepth()}, "flatDepth");
    failures += check((): int => { return toLocaleString()}, "toLocaleString");
    failures += check((): int => { return toLocaleStringObject()}, "toLocaleStringObject");
    failures += check((): int => { return toLocaleStringObjectOption()}, "toLocaleStringObjectOption");
    return check(failures, "All tests run");
}


const d: Number[] = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];
const dd: Number[] = [1, 0, 3, 4, 0, 6];

function createWithArrayOfNumber(): int {
    return success;
    //NOTE (ikorobkov): uncomment when Rest parameters will be supported by Array
    // let first = new Number(0);
    // let arr = new Array<Number>(first, ...d);
    // if (arr[0].valueOf() != first.valueOf()) { return fail; }
    // for(let i: int = 0; i < d.length; ++i) {
    //     if(arr[i+1]! != d[i]) {
    //        return fail;
    //     }
    // }
    // if (arr.length != d.length + 1) { return fail; }
    // return success;
}

function findWithFuncArg3ReadonlyArray(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // let fnPositive = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj[index] == elem && elem.valueOf() == 12 && index > 0; };
    // let fnNegative = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj[index] == elem && elem.valueOf() < 0 && index > 0; };
    // let fnUndefined = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj.at(index) == undefined && index < 0; };
    // let fnNull = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj.at(index) == null && index > 0; };
    // if (arr.find(fnPositive) != 12) { return fail; }
    // if (arr.find(fnNegative) != -1) { return fail; }
    // if (arr.find(fnUndefined) != undefined) { return fail; }
    // if (arr.find(fnNull) != undefined) { return fail; }
    // return success;
}

function findIndexFuncArg3ReadonlyArray(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    //let arr = new Array<Number>;
    //for(let i: int = 0; i < d.length; ++i) {
    //    arr.push(d[i]);
    //}
    // let fnPositive = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj[index] == elem && elem.valueOf() == 12 && index > 0; };
    // let fnNegative = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj[index] == elem && elem.valueOf() < 0 && index > 0; };
    // let fnUndefined = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj.at(index) == undefined && index < 0; };
    // let fnNull = (elem: Number, index: number, obj: ReadonlyArray<Number>): boolean => { return obj.at(index) == null && index > 0; };
    // if (arr.findIndex(fnPositive) != 9) { return fail; }
    // if (arr.findIndex(fnNegative) != 1) { return fail; }
    // if (arr.findIndex(fnUndefined) != -1) { return fail; }
    // if (arr.findIndex(fnNull) != -1) { return fail; }
    // return success;
}

function mapFuncArg3ReadonlyArray(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let fn = (v: Number, index: number, arr: ReadonlyArray<Number>): Number => {
    //     if ((index < 4) && (index < arr.length)) {
    //         return new Number(arr[index].valueOf() * arr[index].valueOf());
    //     } else {
    //         return new Number(0);
    //     }
    // }
    // let arrEmpty = new Array<Number>;
    // let res0 = arrEmpty.map<Number>(fn);
    // if (res0.length != arrEmpty.length) { return fail; }
    // if (res0.length != 0) { return fail; }
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // let res = arr.map<Number>(fn);
    // if (res.length != arr.length) { return fail; }
    // let tmp: Number;
    // for (let i: int = 0; i < res.length; ++i) {
    //     if (i < 4) {
    //         tmp = new Number(arr[i].valueOf() * arr[i].valueOf());
    //     } else {
    //         tmp = new Number(0);
    //     }
    //     if (res[i].valueOf() != tmp.valueOf()) {
    //         return fail;
    //     }
    // }
    // return success;
}

function everyFuncArg3ReadonlyArray(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let fn = (v: Number, index: number, arr: ReadonlyArray<Number>): Number => {
    //     if ((index < 4) && (index < arr.length)) {
    //         return new Number(arr[index].valueOf() * arr[index].valueOf());
    //     } else {
    //         return new Number(0);
    //     }
    // }
    // let arrEmpty = new Array<Number>;
    // let res0 = arrEmpty.map<Number>(fn);
    // if (res0.length != arrEmpty.length) { return fail; }
    // if (res0.length != 0) { return fail; }
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // let res = arr.map<Number>(fn);
    // if (res.length != arr.length) { return fail; }
    // let tmp: Number;
    // for (let i: int = 0; i < res.length; ++i) {
    //     if (i < 4) {
    //         tmp = new Number(arr[i].valueOf() * arr[i].valueOf());
    //     } else {
    //         tmp = new Number(0);
    //     }
    //     if (res[i).valueOf() != tmp.valueOf()) {
    //         return fail;
    //     }
    // }
    // return success;
}

// NOTE (ikorobkov): uncomment when Array will have implementation of from<T,U>(arr: ArrayLike<T>|Iterable<T>, fn: (v: T, k: number) => U)
//
// class ArrayLikeImplTest<T> implements ArrayLike<T> {
//     private buffer: Any[]
//     internal actualLength: int
//
//     public at(index: number): T | undefined {
//         let len = this.actualLength;
//         let k: int;
//         if (index >= 0) {
//             k = index as int;
//         } else {
//             k = len + index as int;
//         }
//         if (k < 0 || k >= len) {
//             return undefined;
//         }
//         return this.$_get(k as number);
//     }
//
//     public $_get(index: number): T {
//         if ((index as int  < 0) || (index as int >= this.actualLength)) {
//             throw new RangeError("Out of bounds");
//         }
//         return this.buffer[index as int] as T;
//     }
//
//     public override lengthMakeProp(): number {
//         return this.actualLength as number;
//     }
//
//     public push(val: T): number {
//         this.ensureUnusedCapacity(1);
//         this.buffer[this.actualLength] = val as Any;
//         this.actualLength += 1;
//         return this.actualLength;
//     }
//
//     private ensureUnusedCapacity(cap: int): void {
//         if (this.actualLength + cap > this.buffer.length) {
//             const copy = new Any[this.buffer.length * 2 + cap];
//             for (let i = 0; i < this.actualLength; i++) {
//                 copy[i] = this.buffer[i];
//             }
//             this.buffer = copy;
//         }
//     }
//
//     public constructor() {
//         this.buffer = new Any[4]
//         this.actualLength = 0
//     }
// }

// NOTE (ikorobkov): uncomment when issue #15523 will be done
// class IterableImplTest<T> implements IterableIterator<T> {
//     private parent: Array<T>
//     private idx: int = 0
//
//     constructor() {
//         this.parent = new Array<T>;
//     }
//
//     constructor(parent: Array<T>) {
//         this.parent = parent;
//     }
//
//     override next(): IteratorResult<T> {
//         if (this.idx >= this.parent.length) {
//             return new IteratorResult<T>();
//         }
//         return new IteratorResult<T>(this.parent[this.idx++]!);
//     }
//
//     override iterator(): Iterator<T> {
//         return this;
//     }
// }

function fromArg1(): int {
    return success;
    //NOTE (ikorobkov): uncomment when Array will have implementation of from
    // let arr2 = new Array<Number>;
    // for(let i: int = 0; i < dd.length; ++i) {
    //     arr2.push(dd[i]);
    // }
    // let arr1 = Array.from<Number>(arr2);
    // if (arr1.length != arr2.length) {
    //     return fail;
    // }
    // for (let i: int = 0; i < arr1.length; ++i) {
    //     if (arr1[i] != arr2[i]) {
    //         return fail;
    //     }
    // }
    // return success;
}

function fromArg2(): int {
    return success;
    //NOTE (ikorobkov): uncomment when Array will have implementation of from
    // let arr2 = new Array<Number>;
    // for(let i: int = 0; i < dd.length; ++i) {
    //     arr2.push(dd[i]);
    // }
    // let fn = (v: Number, k: number): Number => { return v = new Number(v + k); };
    // let arr1 = Array.from<Number, Number>(arr2, fn);
    // if (arr1.length != arr2.length) {
    //     return fail;
    // }
    // for (let i: int = 0; i < arr1.length; ++i) {
    //     if (arr1[i] != arr2[i] + i) {
    //         return fail;
    //     }
    // }
    // return success;
}

function fromArrayLikeAndIterable(): int {
    return success;
    //NOTE (ikorobkov): uncomment when Array will have implementation of from from<T>(arr: ArrayLike<T>|Iterable<T>)
    //NOTE (ikorobkov): below is check of ArrayLike<T>
    // let arrEmpty = new Array<Number>;
    // let res0 = arrEmpty.from<Number>(new ArrayLikeImplTest<Number>);
    // if (res0.length > 0) { return fail; }
    // let arrLike = new ArrayLikeImplTest<Number>;
    // for (let i: int = 0; i < d.length; ++i) {
    //     arrLike.push(d[i]);
    // }
    // let res = arrEmpty.from<Number>(arrLike);
    // if ((res.length as int != arrLike.lengthMakeProp() as int) || (arrEmpty.length != 0)) {
    //     return fail;
    // }
    // for (let i = 0; i < res.length; ++i) {
    //     if (res[i].valueOf() != arrLike.at(i as number)!.valueOf()) {
    //         return fail;
    //     }
    // }
    // //NOTE (ikorobkov): below is check of Iterable<T>
    // let res00 = arrEmpty.from<Number>(new IterableImplTest<Number>);
    // if (res00.length > 0) { return fail; }
    // let tmp = new Array<Number>;
    // for (let i: int = 0; i < d.length; ++i) {
    //     tmp.push(d[i]);
    // }
    // res = arrEmpty.from<Number>(new IterableImplTest<Number>(tmp));
    // if ((res.length as int != tmp.length as int) || (arrEmpty.length != 0)) {
    //     return fail;
    // }
    // for (let i = 0; i < res.length; ++i) {
    //     if (res[i].valueOf() != tmp[i].valueOf()) {
    //         return fail;
    //     }
    // }
    // return success;
}

function fromArrayLikeAndIterableAndFunc(): int {
    return success;
    //NOTE (ikorobkov): uncomment when Array will have implementation of from<T,U>(arr: ArrayLike<T>|Iterable<T>, fn: (v: T, k: number) => U)
    //NOTE (ikorobkov):  and have call 'iteratorForEach<T>' at 801Line in Array.ets
    //NOTE (ikorobkov): Below is check ArrayLike<T>
    // let fn = (val: Number, index: number): Number => {
    //     if (index < 0) {
    //         return new Number(-1);
    //     } else {
    //         return new Number(val.valueOf() * 2);
    //     }
    // }
    // let arrEmpty = new Array<Number>;
    // let res0 = arrEmpty.from<Number, Number>(new ArrayLikeImplTest<Number>, fn);
    // if (res0.length > 0) { return fail; }
    // let arrLike = new ArrayLikeImplTest<Number>;
    // for (let i: int = 0; i < d.length; ++i) {
    //     arrLike.push(d[i]);
    // }
    // let res = arrEmpty.from<Number, Number>(arrLike, fn);
    // if ((res.length as int != arrLike.lengthMakeProp() as int) || (arrEmpty.length != 0)) {
    //     return fail;
    // }
    // for (let i = 0; i < res.length; ++i) {
    //     if (res[i].valueOf() != arrLike.at(i as number)!.valueOf() * 2) {
    //         return fail;
    //     }
    // }
    // //NOTE (ikorobkov): below is check Iterable<T>
    // let res00 = arrEmpty.from<Number, Number>(new IterableImplTest<Number>, fn);
    // if (res00.length > 0) { return fail; }
    // let tmp = new Array<Number>;
    // for (let i: int = 0; i < d.length; ++i) {
    //     tmp.push(d[i]);
    // }
    // res = arrEmpty.from<Number, Number>(new IterableImplTest<Number>(tmp), fn);
    // if ((res.length as int != tmp.length as int) || (arrEmpty.length != 0)) {
    //     return fail;
    // }
    // for (let i = 0; i < res.length; ++i) {
    //     if (res[i].valueOf() != tmp[i].valueOf() * 2) {
    //         return fail;
    //     }
    // }
    // return success;
}

function filterFuncArg3ReadonlyArray(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let fnTrue = (v: Number, index: number, arr: ReadonlyArray<Number>): boolean => {
    //     return arr[index].valueOf() == v.valueOf() && v.valueOf() > -1000;
    // }
    // let arrEmpty = new Array<Number>;
    // if (arrEmpty.filter(fnTrue).length != 0) { return fail; }
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // let res = arr.filter(fnTrue);
    // if (res.length != arr.length) { return fail; }
    // for(let i: int = 0; i < arr.length; ++i) {
    //     if (res[i] != arr[i]) { return fail; }
    // }
    // let fnFalse = (v: Number, index: number, arr: ReadonlyArray<Number>): boolean => {
    //     return arr.at(index) == undefined;
    // }
    // if (arr.filter(fnFalse).length != 0) { return fail; }
    // return success;
}

function reduceFuncArg4ReadonlyArray(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let fn = (a: Number, b: Number, index: number, arr: ReadonlyArray<Number>): Number => {
    //     if (index < 0 || index >= arr.length) {
    //         return new Number(-1);
    //     }
    //     return new Number(a.valueOf() + b.valueOf() + arr[index].valueOf());
    // }
    // let arrEmpty = new Array<Number>;
    // let hasError: boolean = false;
    // try {
    //     let res = arrEmpty.reduce(fn);
    //     return fail;
    // } catch(e) {
    //     hasError = true;
    // }
    // if (hasError == false) { return fail; }
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // let res = arr.reduce(fn);
    // if (res.valueOf() != 996) { return fail; }
    // if (res.valueOf() ==  -1) { return fail; }
    // return success;
}

function reduceFuncArg4ReadonlyArrayWithInitialValue(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let fn = (a: Number, b: Number, index: number, arr: ReadonlyArray<Number>): Number => {
    //     if (index < 0 || index >= arr.length) {
    //         return new Number(-1);
    //     }
    //     return new Number(a.valueOf() + b.valueOf() + arr[index].valueOf());
    // }
    // let arrEmpty = new Array<Number>;
    // let initialValue = new Number(0);
    // let res = arrEmpty.reduce(fn, initialValue);
    // if (res.valueOf() != initialValue.valueOf()) { return fail; }
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // initialValue = -arr[0];
    // res = arr.reduce(fn, initialValue);
    // if (res.valueOf() != 996) { return fail; }
    // if (res.valueOf() ==  -1) { return fail; }
    // return success;
}

function reduceRightFuncArg4ReadonlyArray(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let fn = (a: Number, b: Number, index: number, arr: ReadonlyArray<Number>): Number => {
    //     if (index < 0 || index >= arr.length) {
    //         return new Number(-1);
    //     }
    //     return new Number(a.valueOf() + b.valueOf() + arr[index].valueOf());
    // }
    // let arrEmpty = new Array<Number>;
    // let hasError: boolean = false;
    // try {
    //     let res = arrEmpty.reduceRight(fn);
    //     return fail;
    // } catch(e) {
    //     hasError = true;
    // }
    // if (hasError == false) { return fail; }
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // let res = arr.reduceRight(fn);
    // if (res.valueOf() != 994) { return fail; }
    // if (res.valueOf() ==  -1) { return fail; }
    // return success;
}


function reduceRightFuncArg4ReadonlyArrayWithInitialValue(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let fn = (a: Number, b: Number, index: number, arr: ReadonlyArray<Number>): Number => {
    //     if (index < 0 || index >= arr.length) {
    //         return new Number(-1);
    //     }
    //     return new Number(a.valueOf() + b.valueOf() + arr[index].valueOf());
    // }
    // let arrEmpty = new Array<Number>;
    // let initialValue = new Number(0);
    // let res = arrEmpty.reduce(fn, initialValue);
    // if (res.valueOf() != initialValue.valueOf()) { return fail; }
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // initialValue = -arr[0];
    // res = arr.reduce(fn, initialValue);
    // if (res.valueOf() != 996) { return fail; }
    // if (res.valueOf() ==  -1) { return fail; }
    // return success;
}

function forEachFuncArg3ReadonlyArray(): int {
    return success;
    //NOTE (ikorobkov): uncomment when ReadonlyArray will be supported
    // let s = new String;
    // let fn = (a: Number, index: number, arr: ReadonlyArray<Number>): void => {
    //     if (index < 0 && index >= arr.length) {
    //         s = "";
    //     } else {
    //         let tmp = new Number(arr[index].valueOf() + a.valueOf());
    //         s += tmp.toString() + ",";
    //     }
    // }
    // let arr = new Array<Number>;
    // for(let i: int = 0; i < d.length; ++i) {
    //     arr.push(d[i]);
    // }
    // arr.forEach(fn);
    // if (s != "20,-2,474,-10,296,130,6,68,0,24,") { return fail; }
    // return success;
}

function flatMapFuncArg3(): int {
    let s = "Not implemented. No implementation of flat method in Array";
    return success;
}

function flatMapFuncArg2(): int {
    let s = "Not implemented. No implementation of flat method in Array";
    return success;
}

function flatMapFuncArg1(): int {
    let s = "Not implemented. No implementation of flat method in Array";
    return success;
}

// NOTE (ikorobkov): below commented types could be used for future test for testing of Array.flat method
// type UT0 = Array<(Number)>;
// type UT1 = Array<(Number|UT0)>;
// type UT2 = Array<(Number|UT1)>;
// type UT3 = Array<(Number|UT2)>;

function flat(): int {
    let s = "Not implemented. No implementation of flat method in Array";
    return success;
    //NOTE (ikorobkov): draft ideas for future test of method Array.flat are presented below:
    // const small: UT0 = new UT0();
    // small.push(333);
    // small.push(444);
    // const big: UT1 = new UT1();
    // big.push(111);
    // big.push(small);
    // const small:   UT0 = [333, 444];
    // const big:     UT1 = [111, small];
    // const veryBig: UT2 = [0, big];
    // const veryVeryBig: UT3 = [-1, veryBig];
    // let arr = new Array<UT3>(veryVeryBig);
    // let arr2 = arr1.flat();
    // console.log(arr2);
    // console.log(arr2 instanceof Array);
    // console.log("[0]: " + (arr2[0] instanceof Array) + " " + typeof arr2[0]);
    // console.log("[1]: " + (arr2[1] instanceof Array) + " " + typeof arr2[0]);
    // console.log("[2]: " + (arr2[2] instanceof Array) + " " + typeof arr2[0]);
    // console.log("[3]: " + (arr2[3] instanceof Array) + " " + typeof arr2[0]);
    // return success;
}

function flatDepth(): int {
    let s = "Not implemented. No implementation of flat in Array";
    return success
}

function toLocaleString(): int {
    let s = "Not implemented. No implementation of toLocaleString in Array";
    return success;
}

function toLocaleStringObject(): int {
    let s = "Not implemented. No implementation of toLocaleString in Array";
    return success;
}

function toLocaleStringObjectOption(): int {
    let s = "Not implemented. No implementation of toLocaleString in Array";
    return success;
}

function check(result: int, message: String): int {
    if(result == 0) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    try {
        result = fn();
    } catch(e) {
        console.println("FAILED: " + message + " => Error catched: " + e);
        return 1;
    }

    if(result == success) {
      return 0;
    }
    console.println("\nFAILED: " + message);
    return 1;
}
