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

import { launch } from "std/concurrency"

let counter = 0;

type L = StdDebug.Logger

function stackEater(): void {
    stackEater();
}

function coroStackOverflow(): Object | null {
    stackEater();
    return null;
}

function coroError(): Object {
    throw new Error();
}

function launchStackOverflow(): boolean {
    let overflow: boolean = false;

    try {
        launch<Object | null, () => Object | null>(coroStackOverflow).Await();
        L.log('No errors thrown by coroStackOverflow() but should be!')
        overflow = false;
    } catch (e) {
        if (!(e instanceof StackOverflowError)) {
            L.log('Expected StackOverflowError but another error has been thrown!');
            overflow = false;
        } else {
            overflow = true;
        }
    }

    return overflow;
}

function launchError(): boolean {
    let hasError: boolean = false;

    try {
        launch<Object, () => Object>(coroError).Await();
        L.log('No errors thrown by coroError() but should be!')
        hasError = false;
    } catch (e) {
        if (!(e instanceof Error)) {
            L.log('Expected Error but another error has been thrown!');
            hasError = false;
        } else {
            hasError = true;
        }
    }

    return hasError;
}

function testLaunchStackOverflow() {
    let res: boolean = launchStackOverflow();
    arktest.assertEQ(res, true);
}

function testLaunchError() {
    let res: boolean = launchError();
    arktest.assertEQ(res, true);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.launch_error');
    testSuite.addTest('testLaunchStackOverflow', testLaunchStackOverflow);
    testSuite.addTest('testLaunchError', testLaunchError);
    let res = testSuite.run();
    return res;
}