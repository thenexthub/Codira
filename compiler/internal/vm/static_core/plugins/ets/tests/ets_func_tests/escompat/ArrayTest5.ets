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
    failures += check((): int => { return length() }, "length");
    failures += check((): int => { return shiftUnshift() }, "shiftUnshift");
    failures += check((): int => { return pushPop() }, "pushPop");
    failures += check((): int => { return at() }, "at");
    return check(failures, "All tests run");
}

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

function length(): int {
    return check(() => new Array<number>, (a: Array<number>) => {
        if (a.length != 0) {
            return fail;
        }
        let threw = false;
        try {
            a.length = 12;
        } catch (e) {
            threw = true;
        }
        if (!threw) {
            return fail;
        }
        if (a.length != 0) {
            return fail;
        }

        a.push(1, 2, 3, 4, 5);
        if (a.length != 5) {
            return fail;
        }
        a.length = 3
        if (a.length != 3) {
            return fail
        }
        if (a[0] != 1 || a[1] != 2 || a[2] != 3) {
            return fail;
        }
        a.length = 0
        if (a.length != 0) {
            return fail
        }
        return success;
    })
}

function shiftUnshift(): int {
    return check(() => new Array<number>, (a: Array<number>) => {
        if (a.shift() !== undefined) {
            return fail;
        }
        a.unshift(3, 2, 1)
        if (a.length != 3) {
            return fail;
        }
        a.unshift(6, 5, 4)
        if (a.length != 6) {
            return fail;
        }
        for (let i = 5; i >= 0; i--) {
            if (a.shift()! != i + 1) {
                return fail
            }
            if (a.length != i) {
                return fail;
            }
        }
        return success;
    })
}

function pushPop(): int {
    return check(() => new Array<number>, (a: Array<number>) => {
        if (a.pop() !== undefined) {
            return fail;
        }
        a.push(1, 2, 3)
        if (a.length != 3) {
            return fail;
        }
        a.push(4, 5, 6)
        if (a.length != 6) {
            return fail;
        }
        for (let i = 5; i >= 0; i--) {
            if (a.pop()! != i + 1) {
                return fail
            }
            if (a.length != i) {
                return fail;
            }
        }
        return success;
    })
}

function at(): int {
    return check(() => new Array<number>, (a: Array<number>) => {
        try {
            arr.at(50)
            return fail;
        } catch (e) {
            arktest.assertEQ(e.message, "Invalid index");
        }
        a.pushECMA(1, 2, 3, 4, 5)
        if (a.length != 5) {
            return fail;
        }
        for (let i = 5; i >= 0; i--) {
            if (a.at(i)! != i) {
                return fail
            }
        }
        try {
            arr.at(arr.length);
            return fail;
        } catch (e) {
            arktest.assertEQ(e.message, "Invalid index");
        }
        try {
            arr.at(-arr.length - 1);
            return fail;
        } catch (e) {
            arktest.assertEQ(e.message, "Invalid index");
        }
        if (arr.at(-arr.length)! != arr[0] || arr.at(-arr.length + 1)! != arr[1]) {
            return fail;
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
