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

import {CoroutineExtras, AtomicFlag, launch} from "std/debug/concurrency"

class Test {
    checkIfPromiseInJs(): boolean {
        return this.promiseInJsFlag.get();
    }

    signalPromiseInJs(): void {
        this.promiseInJsFlag.set(true);
    }

    checkIfPromiseIsResolved(): boolean {
        return this.promiseIsResolvedFlag.get();
    }

    signalPromiseIsResolved(): void {
        this.promiseIsResolvedFlag.set(true);
    }

    // Flag is used to signal ETS world that promise now was returned in JS world
    private promiseInJsFlag = new AtomicFlag(false);
    // Flag is used to signal JS world that promise was resloved in ETS world
    private promiseIsResolvedFlag = new AtomicFlag(false);
}

let globalTest: Test | null = null;

function checkIfPromiseIsResolved(): boolean {
    return globalTest!.checkIfPromiseIsResolved();
}

function signalPromiseInJs(): void {
    globalTest!.signalPromiseInJs();
}

function resolverFunction(value: int): int {
    while(!(globalTest!.checkIfPromiseInJs())) {}
    globalResolver!(value);
    return 0;
}

let globalResolver: ((value: int) => void) | null = null;

function testPromiseResolve(value: int): Promise<int> {
    globalTest = new Test;
    let p = new Promise<int> ((resolve: (value: int) => void): void => {
        globalResolver = resolve;
    });
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    launch<int, (i: int) => int>(resolverFunction, value);
    return p;
}