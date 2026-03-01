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

let registry: FinalizationRegistry<int> | null = null;
let registry2: FinalizationRegistry<int> | null = null;


function doFullGC(): void {
    try {
        GC.waitForFinishGC(GC.startGC(GC.Cause.FULL, GC.IN_PLACE_MODE));
    } catch (e) {
        arktest.assertTrue(false, "Unexpected error during GC");
    }
}

function fillFinReg(registry: FinalizationRegistry<int> | null) {
    let object1: Object | null =  new Object();
    let object1_callbackArg = 1;
    registry!.register(object1!, object1_callbackArg);
    object1 = null;
}

function createFinReg(callback: (value: int) => void) : FinalizationRegistry<int>  {
    return new FinalizationRegistry<int>(callback);
}

function InitFinRegAndFill(callback: (value: int) => void, isMainCoro: boolean = true) {
    if (isMainCoro) {
        registry = createFinReg(callback);
        fillFinReg(registry);
    } else {
        registry2 = createFinReg(callback);
        fillFinReg(registry2);
    }
}

function runGCInDiffThreads() {
    doFullGC();
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    launch<void, () => void>(doFullGC).Await();
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
}

function testFinRegOnMainAndDiffCoro() {
    let result_finreg_main : int = 0;
    let result_finreg_not_main : int = 0;
    let cb = (value: int) => {
        result_finreg_not_main = value;
    }
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_NON_MAIN);
    launch<void, (cb: (v: int) => void, f: boolean) => void>(InitFinRegAndFill, cb, false).Await();
    CoroutineExtras.setSchedulingPolicy(CoroutineExtras.POLICY_ANY);
    InitFinRegAndFill((value: int) => {
        result_finreg_main = value;
    }, true);
    runGCInDiffThreads();
    while(!result_finreg_not_main) {}
    arktest.assertEQ(result_finreg_not_main, 1);
    // We need to switch from main coroutine to coroutine created by FinalizationRegistry because 
    // if main coro is executed first, the test will return false
    while(!result_finreg_main) {
        Coroutine.Schedule();
    }
    arktest.assertEQ(result_finreg_main, 1);
}

function main(): void {
    testFinRegOnMainAndDiffCoro();
}
