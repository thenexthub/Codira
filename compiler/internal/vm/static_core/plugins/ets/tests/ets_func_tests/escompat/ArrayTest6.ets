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
    failures += check((): int => { return findIndexFuncArg1() }, "findIndexFuncArg1");
    failures += check((): int => { return findIndexFuncArg2() }, "findIndexFuncArg2");
    failures += check((): int => { return findIndexFuncArg3ArrayNumber() }, "findIndexFuncArg3ArrayNumber");
    failures += check((): int => { return findLastIndexFuncArg1() }, "findLastIndexFuncArg1");
    failures += check((): int => { return findLastFuncArg1() }, "findLastFuncArg1");
    failures += check((): int => { return mapFuncArg1() }, "mapFuncArg1");
    failures += check((): int => { return mapFuncArg2() }, "mapFuncArg2");
    failures += check((): int => { return mapFuncArg3ArrayNumber() }, "mapFuncArg3ArrayNumber");
    return check(failures, "All tests run");
}


const d: Number[] = [10, -1, 237, -5, 148, 65, 3, 34, 0, 12];
const dd: Number[] = [1, 0, 3, 4, 0, 6];

function newProxy<T>(a: Array<T>): Array<T> {
    class Handler<T> extends proxy.DefaultArrayProxyHandler<T> {}

    return proxy.Proxy.create<T>(a, new Handler<T>())
}

function check<T>(newArrayFn: () => Array<T>, testFn: (a: Array<T>) => int): int {
    let failures = 0

    failures += testFn(newArrayFn())
    failures += testFn(newProxy(newArrayFn()))

    return failures
}

function findIndexFuncArg1(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        const fnPositive = (elem: number): boolean => { return elem == 12; };
        const fnNegative = (elem: number): boolean => { return elem < 0; };
        const fnUndefined = (elem: Number): boolean => { return elem == undefined; };
        const fnNull = (elem: Number): boolean => { return elem == null; };
        if (arr.findIndex(fnPositive) != 9) { return fail; }
        if (arr.findIndex(fnNegative) != 1) { return fail; }
        if (arr.findIndex(fnUndefined) != -1) { return fail; }
        if (arr.findIndex(fnNull) != -1) { return fail; }
        return success;
    })
}

function findIndexFuncArg2(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < d.length; ++i) {
           arr.push(d[i]);
        }
        const fnPositive = (elem: number, index: number): boolean => {
            return elem == 12 && index > 0;
        };
        const fnNegative = (elem: number, index: number): boolean => {
            return elem < 0 && index > 0;
        };
        const fnUndefined = (elem: Number, index: number): boolean => {
            return elem == undefined && index < 0;
        };
        const fnNull = (elem: Number, index: number): boolean => {
            return elem == null && index > 0;
        };
        if (arr.findIndex(fnPositive) != 9) { return fail; }
        if (arr.findIndex(fnNegative) != 1) { return fail; }
        if (arr.findIndex(fnUndefined) != -1) { return fail; }
        if (arr.findIndex(fnNull) != -1) { return fail; }
        return success;
    })
}

function findIndexFuncArg3ArrayNumber(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        const fnPositive = (elem: number, index: number, obj: Array<Number>): boolean => {
            return obj.at(index)! == elem && elem == 12 && index > 0;
        };
        const fnNegative = (elem: number, index: number, obj: Array<Number>): boolean => {
            return obj.at(index)! == elem && elem < 0 && index > 0;
        };
        const fnUndefined = (elem: Number, index: number, obj: Array<Number>): boolean => {
            return obj.at(index)! == elem && elem == undefined && index < 0;
        };
        const fnNull = (elem: Number, index: number, obj: Array<Number>): boolean => {
            return obj.at(index)! == elem && elem == null && index > 0;
        };
        if (arr.findIndex(fnPositive) != 9) { return fail; }
        if (arr.findIndex(fnNegative) != 1) { return fail; }
        if (arr.findIndex(fnUndefined) != -1) { return fail; }
        if (arr.findIndex(fnNull) != -1) { return fail; }
        return success;
    })
}

function findLastIndexFuncArg1(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        const fnPositive = (elem: number): boolean => { return elem == 12; };
        const fnNegative = (elem: number): boolean => { return elem < 0; };
        const fnUndefined = (elem: Number): boolean => { return elem == undefined; };
        const fnNull = (elem: Number): boolean => { return elem == null; };
        if (arr.findLastIndex(fnPositive) != 9) { return fail; }
        if (arr.findLastIndex(fnNegative) != 3) { return fail; }
        if (arr.findLastIndex(fnUndefined) != -1) { return fail; }
        if (arr.findLastIndex(fnNull) != -1) { return fail; }
        return success;
    })
}

function findLastFuncArg1(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        const fnPositive = (elem: number): boolean => { return elem == 12; };
        const fnNegative = (elem: number): boolean => { return elem < 0; };
        const fnUndefined = (elem: Number): boolean => { return elem == undefined; };
        const fnNull = (elem: Number): boolean => { return elem == null; };
        if (arr.findLast(fnPositive) != 12) { return fail; }
        if (arr.findLast(fnNegative) != -5) { return fail; }
        if (arr.findLast(fnUndefined) != undefined) { return fail; }
        if (arr.findLast(fnNull) != undefined) { return fail; }
        return success;
    })
}

function mapFuncArg1(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let fn = (elem: number): number => {
            return elem * elem;
        };
        let res0 = arr.map<Number>(fn);
        if (res0.length != arr.length) { return fail; }
        if (res0.length != 0) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.map<Number>(fn);
        if (res.length != arr.length) { return fail; }
        for (let i: int = 0; i < res.length; ++i) {
            if (res.at(i) == undefined) { return fail; }
            if (res.at(i)! != arr.at(i)! * arr.at(i)!) {
                return fail;
            }
        }
        return success;
    })
}

function mapFuncArg2(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let fn = (elem: number, index: number): number => {
            if (index < 4) {
                return elem * elem;
            } else {
                return 0;
            }
        }
        let res0 = arr.map<Number>(fn);
        if (res0.length != arr.length) { return fail; }
        if (res0.length != 0) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.map<Number>(fn);
        if (res.length != arr.length) { return fail; }
        let tmp: Number;
        for (let i: int = 0; i < res.length; ++i) {
            if (res.at(i) == undefined) { return fail; }
            if (i < 4) {
                tmp = new Number(arr.at(i)! * arr.at(i)!);
            } else {
                tmp = 0;
            }
            if (res.at(i)! != tmp) {
                return fail;
            }
        }
        return success;
    })
}

function mapFuncArg3ArrayNumber(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let fn = (elem: number, index: number, arr: Array<Number>): number => {
            if ((index < 4) && (index < arr.length)) {
                return arr.at(index)! * arr.at(index)!;
            } else {
                return 0;
            }
        }
        let res0 = arr.map<Number>(fn);
        if (res0.length != arr.length) { return fail; }
        if (res0.length != 0) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.map<Number>(fn);
        if (res.length != arr.length) { return fail; }
        let tmp: Number;
        for (let i: int = 0; i < res.length; ++i) {
            if (res.at(i) == undefined) { return fail; }
            if (i < 4) {
                tmp = new Number(arr.at(i)! * arr.at(i)!);
            } else {
                tmp = 0;
            }
            if (res.at(i)! != tmp) {
                return fail;
            }
        }
        return success;
    })
}

function check(result: int, message: String): int {
    if (result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}

function check(fn: () => int, message: String): int {
    let result: int = 0;
    try {
        result = fn();
    } catch(e) {
        console.println("FAILED: " + message + " => Error catched: " + e);
        return fail;
    }
    if (result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}
