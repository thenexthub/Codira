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
    failures += check((): int => { return key() }, "key");
    failures += check((): int => { return values() }, "values");
    failures += check((): int => { return iterator() }, "iterator");
    failures += check((): int => { return findWithFuncArg1() }, "findWithFuncArg1");
    failures += check((): int => { return findWithFuncArg2() }, "findWithFuncArg2");
    failures += check((): int => { return findWithFuncArg3ArrayNumber() }, "findWithFuncArg3ArrayNumber");
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

function key(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let iterator = arr.keys();
        let i: int = 0;
        while (true) {
            const v = iterator.next();
            if (v.done) {
                if (i > 0){
                    return fail;
                };
                break;
            }
            i = i + 1;
        }
        for (i = 0; i < dd.length; ++i) {
            arr.push(dd[i]);
        }
        iterator = arr.keys();
        let idx: Number = 0;
        while (true) {
            const v = iterator.next();
            if (v.done) {
                if ((idx != arr.length) || (idx != dd.length)) {
                    return fail;
                }
                return success;
            }
            if ((v.value != idx) || (v.value == undefined)) {
                return fail;
            }
            idx = idx + 1;
        }
    })
}

function values(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let iterator = arr.values();
        let i: int = 0;
        while (true) {
            const v = iterator.next();
            if (v.done) {
                if ((i != 0) && (i != arr.length)) {
                    return fail;
                }
                break;
            }
            i++;
        }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i])
        }
        iterator = arr.values();
        i = 0;
        while (true) {
            const v = iterator.next();
            if (v.done) {
                if ((i > 0) && (i != arr.length)) {
                    return fail;
                }
                return success;
            }
            if (v.value != arr.at(i)) {
                return fail;
            }
            i++;
        }
    })
}

function iterator(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let iterator = arr.$_iterator();
        let i: int = 0;
        while (true) {
            const v = iterator.next();
            if (v.done) {
                if ((i > 0) && (i != arr.length)) {
                    return fail;
                }
                break;
            }
            i++;
        }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        iterator = arr.$_iterator();
        i = 0;
        while (true) {
            const v = iterator.next();
            if (v.done) {
                if ((i > 0) && (i != arr.length)) {
                    return fail;
                }
                return success;
            }
            if (v.value != arr.at(i)) {
                return fail;
            }
            i++;
        }
    })
}

function findWithFuncArg1(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < d.length as int; ++i) {
            arr.push(d[i]);
        }
        const fnPositive = (elem: number): boolean => { return elem == 12; };
        const fnNegative = (elem: number): boolean => { return elem < 0; };
        const fnUndefined = (elem: Number): boolean => { return elem == undefined; };
        const fnNull = (elem: Number): boolean => { return elem == null; };
        if (arr.find(fnPositive) != 12) { return fail; }
        if (arr.find(fnNegative) != -1) { return fail; }
        if (arr.find(fnUndefined) != undefined) { return fail; }
        if (arr.find(fnNull) != undefined) { return fail; }
        return success;
    })
}

function findWithFuncArg2(): int {
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
        if (arr.find(fnPositive) != 12) { return fail; }
        if (arr.find(fnNegative) != -1) { return fail; }
        if (arr.find(fnUndefined) != undefined) { return fail; }
        if (arr.find(fnNull) != undefined) { return fail; }
        return success;
    })
}

function findWithFuncArg3ArrayNumber(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        const fnPositive = (elem: number, index: number, obj: Array<Number>): boolean => {
            return obj[index] == elem && elem == 12 && index > 0;
        };
        const fnNegative = (elem: number, index: number, obj: Array<Number>): boolean => {
            return obj[index] == elem && elem < 0 && index > 0;
        };
        const fnUndefined = (elem: Number, index: number, obj: Array<Number>): boolean => {
            return obj[index] == elem && elem == undefined && index < 0;
        };
        const fnNull = (elem: Number, index: number, obj: Array<Number>): boolean => {
            return obj[index] == elem && elem == null && index > 0;
        };
        if (arr.find(fnPositive) != 12) { return fail; }
        if (arr.find(fnNegative) != -1) { return fail; }
        if (arr.find(fnUndefined) != undefined) { return fail; }
        if (arr.find(fnNull) != undefined) { return fail; }
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
