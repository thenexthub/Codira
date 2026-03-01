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
    failures += check((): int => { return testIteratorWithEmptyArray() }, "testIteratorWithEmptyArray");
    failures += check((): int => { return testIteratorWithModifyArrayDuringIteration() }, "testIteratorWithModifyArrayDuringIteration");
    failures += check((): int => { return testIteratorWithDeleteElementDuringIteration() }, "testIteratorWithDeleteElementDuringIteration");
    return check(failures, "All tests run");
}

const dd: Number[] = [1, 0, 3, 4, 0, 6];

function testIteratorWithEmptyArray(): int {
    const arr = new Array<Number>;
    const iterator = arr.$_iterator();
    const v = iterator.next();
    if (!v.done) {
        return fail;
    }
    return success;
}

function testIteratorWithModifyArrayDuringIteration(): int {
    const arr = new Array<Number>;
    let iterator = arr.$_iterator();
    let i: int = 0;

    for (let i: int = 0; i < dd.length; ++i) {
        arr.push(dd[i]);
    }
    while (true) {
        const v = iterator.next();
        if (v.done) {
            if (i != arr.length) {
                return fail;
            }
            break;
        }
        if (v.value != arr[i]) {
            return fail;
        }
        if (i == 1) {
            arr.push(100);
        }
        i++;
    }
    return success;
}

function testIteratorWithDeleteElementDuringIteration(): int {
    const arr = new Array<Number>;
    let iterator = arr.$_iterator();
    let i: int = 0;

    for (let i: int = 0; i < dd.length; ++i) {
        arr.push(dd[i]);
    }
    
    while (true) {
        const v = iterator.next();
        if (v.done) {
            if (i != arr.length) {
                return fail;
            }
            break;
        }
        if (v.value != arr[i]) {
            return fail;
        }
        if (i == 1) {
            arr.splice(1, 1);
        }
        i++;
    }
    return success;
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
