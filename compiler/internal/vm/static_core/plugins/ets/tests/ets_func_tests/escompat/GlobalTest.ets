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

function foo(a: object): int {
    return success;
}

function bar(a: Object): int {
    return success;
}

function main(): int {
    let failures: int = 0;
    failures += check((): int => { return testFooPrimitiveObject()}, "test foo with type object");
    failures += check((): int => { return testFooObject()}, "test foo with type Object");
    failures += check((): int => { return testBarPrimitiveObject()}, "test bar with type object");
    failures += check((): int => { return testBarObject()}, "test bar with type Object");

    return check(failures, "All tests run");
}

function testFooPrimitiveObject(): int {
    return foo(new object()) == success ? success : fail;
}

function testFooObject(): int {
    return foo(new Object()) == success ? success : fail;
}

function testBarPrimitiveObject(): int {
    return bar(new object()) == success ? success : fail;
}

function testBarObject(): int {
    return bar(new Object()) == success ? success : fail;
}

function check(result: int, message: String): int {
    if(result == success) {
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
    if(result == success) {
        return success;
    }
    console.println("\nFAILED: " + message);
    return fail;
}
