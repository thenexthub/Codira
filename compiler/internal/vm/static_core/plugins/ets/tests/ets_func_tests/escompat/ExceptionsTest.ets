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

const success = 0;
const fail = 1;

function main(): int {
    let failures = 0;

    failures += test(testThrowError(), "Throw an Error and catch it");
    failures += test(testErrorWithMessage(), "Error object with cause");
    failures += test(testThrowRuntimeErrorWithoutArgs(), "Throw an RuntimeError object");
    failures += test(testThrowRuntimeErrorWithoutReason(), "Throw an RuntimeError object with message");
    return failures;
}

function testThrowError(): int {
    try {
        throw new Error("Test throw");
    } catch (ex) {
        if (ex instanceof Error) {
            return success;
        }
        return fail;
    }
}

function testErrorWithMessage(): int {
    try {
        let message = "Test Error with message";
        throw new Error(message);
    } catch (ex) {
        if (ex instanceof Error) {
            return success;
        }
        return fail;
    }
}

function testThrowRuntimeErrorWithoutArgs(): int {
    try {
        throw new RuntimeError();
    }
    catch (ex) {
        if (ex instanceof RuntimeError) {
            return success;
        }
        return fail;
    }
}

function testThrowRuntimeErrorWithoutReason(): int {
    try {
        throw new RuntimeError("Test RuntimeError");
    }
    catch (ex) {
        if (ex instanceof RuntimeError) {
            return success;
        }
        return fail;
    }
}


function test(result: int, message: String): int {
    if (result == 0) {
      return success;
    }
    console.println("FAILED: " + message);
    return fail;
}
