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

import {jsAsync} from 'promiseProxy'
import {jsAsyncFailing} from 'promiseProxy'

class Test {
    private static UNSET: int = 0;
    private static SUCCESS: int = 1;
    private static FAIL: int = 2;

    constructor() {
        this.result = Test.UNSET;
    }

    success(): void {
        if (this.is_unset()) {
            this.result = Test.SUCCESS;
        }
    }

    fail(): void {
        this.result = Test.FAIL;
        console.println("Test failed");
        // throw new Error();
    }

    check(): void {
        if (this.result == Test.SUCCESS) {
            return;
        }
        if (this.result == Test.UNSET) {
            console.println("Test result is not set");
        }
        throw new Error();
    }

    is_unset(): boolean {
        return (this.result == Test.UNSET);
    }

    private result: int;
}

let globalTest : Test | null = null;

function check() : int {
    globalTest!.check();
    return 0;
}

function is_unset(): boolean {
    return globalTest!.is_unset();
}

async function doTest(p : Promise<Object>) : Promise<void> {
    p.then((x:Object): void => {
        if (x as string == "success") {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
}

async function doTestWithAwait(p : Promise<Object>) : Promise<void> {
    p.then((x:Object): void => {
        if (x as string == "success") {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });

    await p;
}

async function doTestReject(p : Promise<Object>) : Promise<void> {
    p.then((x:Object): void => {
        globalTest!.fail();
    }).catch<void>((x: Error): void => {
        let jsx = x as Error;
        if (jsx.message == 'rejected') {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
}

async function doTestRejectWithAwait(p : Promise<Object>) : Promise<void> {
    let q = p.then((x:Object): void => {
        globalTest!.fail();
    }).catch((x: Error): void => {
        let jsx = x as Error;
        if (jsx.message == 'rejected') {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
    await q;
}

async function doTestRejectWithAwaitShouldFail(p : Promise<Object>) : Promise<void> {
    try {
        await p;
        globalTest!.fail();
    } catch (e) {
        let jse = e as Error;
        if (jse.message == 'rejected') {
            globalTest!.success();
            return Promise.resolve();
        }
    }
    globalTest!.fail();
    return Promise.resolve();
}

async function doTestThenRejectWithAwaitShouldFail(p : Promise<Object>) : Promise<void> {
    let q = p.then((x:Object): void => {
        globalTest!.fail();
    })

    try {
        await q;
        globalTest!.fail();
    } catch (e) {
        let jse = e as Error;
        if (jse.message == 'rejected') {
            globalTest!.success();
            return Promise.resolve();
        }
    }
    globalTest!.fail();
    return Promise.resolve();
}

function testProxyPromise() : void {
    globalTest = new Test();
    let p = jsAsync() as Promise<Object>;
    doTest(p);
}

function testAwaitPromiseProxy() : void {
    globalTest = new Test();
    let p = jsAsync() as Promise<Object>;
    doTestWithAwait(p);
}

function testProxyPromiseReject() : void {
    globalTest = new Test();
    let p = jsAsyncFailing() as Promise<Object>;
    doTestReject(p);
}

function testAwaitPromiseProxyReject() : void {
    globalTest = new Test();
    let p = jsAsyncFailing() as Promise<Object>;
    doTestRejectWithAwait(p);
}

function testAwaitPromiseProxyRejectShouldFail() : void {
    globalTest = new Test();
    let p = jsAsyncFailing() as Promise<Object>;
    doTestRejectWithAwaitShouldFail(p);
}

function testAwaitPromiseProxyThenRejectShouldFail() : void {
    globalTest = new Test();
    let p = jsAsyncFailing() as Promise<Object>;
    doTestThenRejectWithAwaitShouldFail(p);
}
