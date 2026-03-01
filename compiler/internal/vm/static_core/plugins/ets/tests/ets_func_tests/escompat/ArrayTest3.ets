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
    failures += check((): int => { return everyFuncArg3ArrayNumber() }, "everyFuncArg3ArrayNumber");
    failures += check((): int => { return everyFuncArg2() }, "everyFuncArg2");
    failures += check((): int => { return everyFuncArg1() }, "everyFuncArg1");
    failures += check((): int => { return someFuncArg2() }, "someFuncArg2");
    failures += check((): int => { return someFuncArg1() }, "someFuncArg1");
    failures += check((): int => { return filterFuncArg3ArrayNumber() }, "filterFuncArg3ArrayNumber");
    failures += check((): int => { return filterFuncArg2() }, "filterFuncArg2");
    failures += check((): int => { return filterFuncArg1() }, "filterFuncArg1");
    failures += check((): int => { return reduceFuncArg4ArrayNumber() }, "reduceFuncArg4ArrayNumber");
    failures += check((): int => { return reduceFuncArg3() }, "reduceFuncArg3");
    failures += check((): int => { return reduceFuncArg2() }, "reduceFuncArg2");
    failures += check((): int => { return reduceFuncArg4ArrayNumberWithInitialValue() }, "reduceFuncArg4ArrayNumberWithInitialValue");
    failures += check((): int => { return reduceFuncArg3WithInitialValue() }, "reduceFuncArg3WithInitialValue");
    failures += check((): int => { return reduceFuncArg2WithInitialValue() }, "reduceFuncArg2WithInitialValue");
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

function everyFuncArg3ArrayNumber(): int {
    let fnTrue = (elem: number, index: number, arr: Array<Number>): boolean => {
        return arr.at(index)! == elem && elem < 238;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        if (arr.every(fnTrue) == false) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        if (arr.every(fnTrue) == false) { return fail; }

        let fnFalse = (elem: number, index: number, arr: Array<Number>): boolean => {
            return arr.at(index)! == elem && elem < -238;
        }
        if (arr.every(fnFalse) == true) { return fail; }
        return success;
    })
}

function everyFuncArg2(): int {
    let fnTrue = (elem: number, index: number): boolean => {
        return index >= 0 && elem < 238;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        if (arr.every(fnTrue) == false) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        if (arr.every(fnTrue) == false) { return fail; }

        let fnFalse = (elem: number, index: number): boolean => {
            return index >= 0 && elem < -238;
        }
        if (arr.every(fnFalse) == true) { return fail; }
        return success;
    })
}

function everyFuncArg1(): int {
    let fnTrue = (elem: number): boolean => {
        return elem < 238;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        if (arr.every(fnTrue) == false) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        if (arr.every(fnTrue) == false) { return fail; }

        let fnFalse = (elem: number): boolean => {
            return elem < -238;
        }
        if (arr.every(fnFalse) == true) { return fail; }
        return success;
    })
}

function someFuncArg2(): int {
    let fnTrue = (v: number, index: number): boolean => {
        return index >= 0 && v < -4;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        if (arr.some(fnTrue) == true) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        if (arr.some(fnTrue) == false) { return fail; }

        let fnFalse = (v: number, index: number): boolean => {
            return index >= 0 && v < -238;
        }
        if (arr.some(fnFalse) == true) { return fail; }
        return success;
    })
}

function someFuncArg1(): int {
    let fnTrue = (v: number): boolean => {
        return v < -4;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        if (arr.some(fnTrue) == true) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        if (arr.some(fnTrue) == false) { return fail; }

        let fnFalse = (v: number): boolean => {
            return v < -238;
        }
        if (arr.some(fnFalse) == true) { return fail; }
        return success;
    })
}

function filterFuncArg3ArrayNumber(): int {
    let fnTrue = (v: number, index: number, arr: Array<Number>): boolean => {
        return (arr.at(index)! == v) && v > -1000;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        if (arr.filter(fnTrue).length != 0) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.filter(fnTrue);
        if (res.length != arr.length) { return fail; }
        for (let i: int = 0; i < arr.length; ++i) {
            if (res.at(i) != arr.at(i)) { return fail; }
        }

        let fnFalse = (v: Number, index: number, arr: Array<Number>): boolean => {
            return arr.at(index)! == v && v == undefined;
        }
        if (arr.filter(fnFalse).length != 0) { return fail; }
        return success;
    })
}

function filterFuncArg2(): int {
    let fnTrue = (v: number, index: number): boolean => {
        return index >= 0 && v > -1000;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        if (arr.filter(fnTrue).length != 0) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.filter(fnTrue);
        if (res.length != arr.length) { return fail; }
        for (let i: int = 0; i < arr.length; ++i) {
            if (res.at(i) != arr.at(i)) { return fail; }
        }

        let fnFalse = (v: Number, index: number): boolean => {
            return index >= 0 && v == undefined;
        }
        if (arr.filter(fnFalse).length != 0) { return fail; }
        return success;
    })
}

function filterFuncArg1(): int {
    let fnTrue = (v: number): boolean => {
        return v > -1000;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        if (arr.filter(fnTrue).length != 0) { return fail; }
        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.filter(fnTrue);
        if (res.length != arr.length) { return fail; }
        for (let i: int = 0; i < arr.length; ++i) {
            if (res.at(i) != arr.at(i)) { return fail; }
        }

        let fnFalse = (v: Number): boolean => {
            return v == undefined;
        }
        if (arr.filter(fnFalse).length != 0) { return fail; }
        return success;
    })
}

function reduceFuncArg4ArrayNumber(): int {
    let fn = (a: number, b: number, index: number, arr: Array<Number>): number => {
        if (index < 0 || index >= arr.length) {
            return -1;
        }
        return new Number(a + b + (arr.at(index) as Number));
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let hasError: boolean = false;
        try {
            let res = arr.reduce(fn);
            return fail;
        } catch(e) {
            hasError = true;
        }
        if (hasError == false) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.reduce(fn);
        if (res != 996) { return fail; }
        if (res ==  -1) { return fail; }
        return success;
    })
}

function reduceFuncArg3(): int {
    let fn = (a: number, b: number, index: number): number => {
        if (index < 0) {
            return -1;
        }
        return a + b;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let hasError: boolean = false;
        try {
            let res = arr.reduce(fn);
            return fail;
        } catch(e) {
            hasError = true;
        }
        if (hasError == false) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.reduce(fn);
        if (res != 503) { return fail; }
        if (res ==  -1) { return fail; }
        return success;
    })
}

function reduceFuncArg2(): int {
    let fn = (a: number, b: number): number => {
        return a + b;
    }

    return check(() => new Array<Number>, (arr: Array<Number>) => {
        let hasError: boolean = false;
        try {
            let res = arr.reduce(fn);
            return fail;
        } catch(e) {
            hasError = true;
        }
        if (hasError == false) { return fail; }

        for (let i: int = 0; i < d.length; ++i) {
            arr.push(d[i]);
        }
        let res = arr.reduce(fn);
        if (res != 503) { return fail; }
        if (res ==  -1) { return fail; }
        return success;
    })
}

function reduceFuncArg4ArrayNumberWithInitialValue(): int {
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

function reduceFuncArg3WithInitialValue(): int {
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
        res = arr.reduce(fn, initialValue);
        if (res != 503) { return fail; }
        if (res ==  -1) { return fail; }
        return success;
    })
}

function reduceFuncArg2WithInitialValue(): int {
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
        res = arr.reduce(fn, initialValue);
        if (res != 503) { return fail; }
        if (res ==  -1) { return fail; }
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
        return 1;
    }
    if (result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}
