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
    failures += check((): int => { return reduceRightFuncArg4ArrayNumber() }, "reduceRightFuncArg4ArrayNumber");
    failures += check((): int => { return reduceRightFuncArg3() }, "reduceRightFuncArg3");
    failures += check((): int => { return reduceRightFuncArg2() }, "reduceRightFuncArg2");
    failures += check((): int => { return reduceRightFuncArg4ArrayNumberWithInitialValue() }, "reduceRightFuncArg4ArrayNumberWithInitialValue");
    failures += check((): int => { return reduceRightFuncArg3WithInitialValue() }, "reduceRightFuncArg3WithInitialValue");
    failures += check((): int => { return reduceRightFuncArg2WithInitialValue() }, "reduceRightFuncArg2WithInitialValue");
    failures += check((): int => { return forEachFuncArg3ArrayNumber() }, "forEachFuncArg3ArrayNumber");
    failures += check((): int => { return forEachFuncArg2() }, "forEachFuncArg2");
    failures += check((): int => { return forEachFuncArg1() }, "forEachFuncArg1");
    failures += check((): int => { return sortFunc() }, "sortFunc");
    failures += check((): int => { return toSortedFunc() }, "toSortedFunc");
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

function reduceRightFuncArg4ArrayNumber(): int {
    let fn = (a: number, b: number, index: number, arr: Array<Number>): number => {
        if (index < 0 || index >= arr.length) {
            return -1;
        }
        return a + b + arr.at(index)!;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let hasError: boolean = false;
        try {
            let res = arr.reduceRight(fn);
            return fail;
        } catch(e) {
            hasError = true;
        }
        if (hasError == false) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.reduceRight(fn);
        if (res != 994) { return fail; }
        if (res ==  -1) { return fail; }
        return success;
    })
}

function reduceRightFuncArg3(): int {
    let fn = (a: number, b: number, index: number): number => {
        if (index < 0) {
            return -1;
        }
        return a + b;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let hasError: boolean = false;
        try {
            let res = arr.reduceRight(fn);
            return fail;
        } catch(e) {
            hasError = true;
        }
        if (hasError == false) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.reduceRight(fn);
        if (res != 503) { return fail; }
        if (res ==  -1) { return fail; }
        return success;
    })
}

function reduceRightFuncArg2(): int {
    let fn = (a: number, b: number): number => {
        return a + b;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let hasError: boolean = false;
        try {
            let res = arr.reduceRight(fn);
            return fail;
        } catch(e) {
            hasError = true;
        }
        if (hasError == false) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.reduceRight(fn);
        if (res != 503) { return fail; }
        return success;
    })
}

function reduceRightFuncArg4ArrayNumberWithInitialValue(): int {
    let fn = (a: number, b: number, index: number, arr: Array<Number>): number => {
        if (index < 0 || index >= arr.length) {
            return -1;
        }
        return a + b + arr.at(index)!;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let initialValue = 0.0;
        let res = arr.reduce(fn, initialValue);
        if (res != initialValue) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        initialValue = -arr.at(0)!;
        res = arr.reduce(fn, initialValue);
        if (res != 996) { return fail; }
        if (res ==  -1) { return fail; }
        return success;
    })
}

function reduceRightFuncArg3WithInitialValue(): int {
    let fn = (a: number, b: number, index: number): number => {
        if (index < 0) {
            return -1;
        }
        return a + b;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let initialValue = 0.0;
        let res = arr.reduce(fn, initialValue);
        if (res != initialValue) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        initialValue = -arr.at(0)!;
        res = arr.reduce(fn, initialValue);
        if (res != 493) { return fail; }
        if (res ==  -1) { return fail; }
        return success;
    })
}

function reduceRightFuncArg2WithInitialValue(): int {
    let fn = (a: number, b: number): number => {
        return a + b;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let initialValue = 0.0;
        let res = arr.reduce(fn, initialValue);
        if (res != initialValue) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        initialValue = -arr.at(0)!;
        res = arr.reduce(fn, initialValue);
        if (res != 493) { return fail; }
        return success;
    })
}

function forEachFuncArg3ArrayNumber(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let s = new String;
        let fn = (a: number, index: number, arr: Array<Number>): void => {
            if (index < 0 && index >= arr.length) {
                s = "";
            } else {
                let tmp = arr.at(index)! + a;
                s += (tmp as Number).toString() + ",";
            }
        }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        arr.forEach(fn);
        if (s != "20,-2,474,-10,296,130,6,68,0,24,") { return fail; }
        return success;
    })
}

function forEachFuncArg2(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let s = new String;
        let fn = (a: number, index: number): void => {
            if (index < 0) {
                s = "";
            } else {
                s += (a as Number).toString() + ",";
            }
        }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        arr.forEach(fn);
        if (s != "10,-1,237,-5,148,65,3,34,0,12,") { return fail; }
        return success;
    })
}

function forEachFuncArg1(): int {
    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let s = new String;
        let fn = (a: number): void => {
            s += (a as Number).toString() + ",";
        }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        arr.forEach(fn);
        if (s != "10,-1,237,-5,148,65,3,34,0,12,") { return fail; }
        return success;
    })
}

function sortFunc(): int {
    let fn = (a: number, b: number): number => {
        let res: number = a - b;
        return res;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let res0 = arr.sort(fn);
        if (res0.length > 0) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.sort(fn);
        if (res.length != arr.length)   { return fail; }
        for (let i: int = 0; i < res.length; ++i) {
            if (res.at(i) == undefined) { return fail; }
        }
        if (res.at(0)! != -5)  { return fail; }
        if (res.at(1)! != -1)  { return fail; }
        if (res.at(2)! != -0)  { return fail; }
        if (res.at(3)! != 3)   { return fail; }
        if (res.at(4)! != 10)  { return fail; }
        if (res.at(5)! != 12)  { return fail; }
        if (res.at(6)! != 34)  { return fail; }
        if (res.at(7)! != 65)  { return fail; }
        if (res.at(8)! != 148) { return fail; }
        if (res.at(9)! != 237) { return fail; }
        return success;
    })
}

function toSortedFunc(): int {
    let fn = (a: number, b: number): number => { return a - b; }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let res0 = arr.sort(fn);
        if (res0.length > 0) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.toSorted(fn);
        if (res.length != arr.length)   { return fail; }
        for (let i: int = 0; i < res.length; ++i) {
            if (res.at(i) == undefined) { return fail; }
        }
        if (res.at(0)! != -5)  { return fail; }
        if (res.at(1)! != -1)  { return fail; }
        if (res.at(2)! != -0)  { return fail; }
        if (res.at(3)! != 3)   { return fail; }
        if (res.at(4)! != 10)  { return fail; }
        if (res.at(5)! != 12)  { return fail; }
        if (res.at(6)! != 34)  { return fail; }
        if (res.at(7)! != 65)  { return fail; }
        if (res.at(8)! != 148) { return fail; }
        if (res.at(9)! != 237) { return fail; }
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
