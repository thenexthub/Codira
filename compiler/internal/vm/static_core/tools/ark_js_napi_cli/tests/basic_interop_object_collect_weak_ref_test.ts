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

// This test check basic cases of interop objects
// being collected by GC

import { interop } from './gc_test_common';

function getSTSWeakRef(): WeakRef<Object> {
    return new WeakRef(interop.GetSTSObjectWithWeakRef());
}

function getJSPromiseWeakRef(): WeakRef<Promise<Object>> {
    let a = new Promise<Object>((resolve) => {resolve(new Object());});
    interop.SetJSPromiseWithWeakRef(a);
    return new WeakRef(a);
}

// From ETS -> JS, ETS collect 1st, JS collect 2nd
function isSts2JsObjectCollectedTest(): number {
    let bTestResult = 0;
    let wr = getSTSWeakRef();
    interop.RunInteropGC();
    globalThis.test.RunJsGC();
    interop.RunPandaGC();

    if (!interop.isSTSObjectCollected()) {
        print('isSts2JsObjectCollectedTest failed. ETS object is collected.');
        bTestResult = 1;
    }
    if (wr.deref() !== undefined) {
        print('isSts2JsObjectCollectedTest failed. JS object is collected.');
        bTestResult = 1;
    }
    return bTestResult;
}

// From JS -> ETS, ETS collect 1st, JS collect 2nd
function isJs2StsPromiseCollectedTest(): number {
    let bTestResult = 0;
    let wr = getJSPromiseWeakRef();
    interop.RunInteropGC();
    globalThis.test.RunJsGC();
    interop.RunPandaGC();

    if (!(interop.isSTSPromiseCollected())) {
        print('isJs2StsPromiseCollectedTest failed. ETS promise is not collected.');
        bTestResult = 1;
    }
    if (wr.deref() !== undefined) {
        print('isJs2StsPromiseCollectedTest failed. JS promise is not collected.');
        bTestResult = 1;
    }
    return bTestResult;
}

function main(): void {
    let isTestFailed = 0;
    isTestFailed += isSts2JsObjectCollectedTest();
    isTestFailed += isJs2StsPromiseCollectedTest();
    if (isTestFailed) {
        throw Error('Basic interop object collect test is failed!');
    }
}
main();
