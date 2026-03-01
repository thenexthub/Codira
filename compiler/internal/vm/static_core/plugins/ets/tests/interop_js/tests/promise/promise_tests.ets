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

class Test {
    private static UNSET: int = 0;
    private static SUCCESS: int = 1;
    private static FAIL: int = 2;

    constructor(iterations: int = 0) {
        this.result = Test.UNSET;
        this.iterations = iterations;
    }

    success(): void {
        if (this.result == Test.UNSET) {
            this.result = Test.SUCCESS;
        }
    }

    fail(): void {
        this.result = Test.FAIL;
        console.println("Test failed");
        throw new Error();
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

    ready(): boolean {
        if (this.iterations == 0) {
            return true;
        }
        this.iterations--;
        return false;
    }

    static FATAL: int = 0;
    static ERROR: int = 1;
    static INFO: int = 2;
    // private log_level: int = 5;
    private log_level: int = 1;

    setLogLevel(lvl: int): void {
        this.log_level = lvl;
    }

    msg(s: String, lvl: int): void {
        if (lvl <= this.log_level) {
            console.println(s);
        }
    }

    private result: int;
    private iterations: int;
}
let globalTest: Test | null = null;

function check(): int {
    globalTest!.check();
    return 0;
}

function is_unset(): boolean {
    return globalTest!.is_unset();
}

function ready(): boolean {
    return globalTest!.ready();
}

function testPendingPromise(): int {
    globalTest = new Test();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        globalTest!.success()
    });
    globalTest!.check();
    p.then<void>((): void => {
        globalTest!.fail();
    });
    p.catch<void>((error: Any): void => {
        globalTest!.fail();
    });
    // Check callback are not called inside 'then' and 'catch'
    globalTest!.check();
    return 0;
}

function testResolvedPromise(): int {
    let obj = new Object();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve(obj);
    });
    globalTest = new Test();
    p.then<Object | null>((value: Object): Object | null => {
        if (value == obj) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
        return null;
    });
    p.catch<Object | null>((err: Any): Object | null => {
        globalTest!.fail();
        return null;
    });
    return 0;
}

function testRejectedPromise(): int {
    globalTest = new Test();
    let error = new Error();
    try {
        let p = new Promise<Object>((resolve: (value: Object) => void): void => {
            throw error;
        });
        p.then<Object | null>((value: Object): Object | null => {
            globalTest!.fail();
            return null;
        });
        p.catch<Object | null>((err: Error): Object => {
            if (err == error) {
                globalTest!.success();
            } else {
                globalTest!.fail();
            }
        });
    } catch (e) {
        globalTest!.fail();
    }
    return 0;
}

class ThenBeforeResolveFixture {
    public fn: (value: Object) => void = (value: Object) => {};
    public state = 0;
}

function testThenBeforeResolve(): int {
    globalTest = new Test();
    let fixture = new ThenBeforeResolveFixture();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        fixture.fn = resolve;
    });
    let obj = new Object();
    p.then<Object | null>((value: Object): Object | null => {
        if (value == obj) {
            if (fixture.state == 0) {
                fixture.state = 1;
            } else {
                console.println("Wrong 'then' call order");
                globalTest!.fail();
            }
        } else {
            console.println("Then is called with wrong object");
            globalTest!.fail();
        }
        return null;
    });
    p.then<Object | null>((value: Object): Object | null => {
        if (value == obj) {
            if (fixture.state == 1) {
                globalTest!.success();
            } else {
                console.println("Wrong 'then' call order");
                globalTest!.fail();
            }
        } else {
            console.println("Then is called with wrong object");
            globalTest!.fail();
        }
        return null;
    });
    fixture.fn(obj);
    return 0;
}

function testPromiseEmptyThen(): int {
    globalTest = new Test();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve(new Object());
    });
    let nextP = p.then<Object>((): Object => {
        globalTest!.success();
    });
    return 0;
}

class PromiseChainFixture {
    public state = 0;
}

function testPromiseChain(): int {
    globalTest = new Test();
    let fixture = new PromiseChainFixture();
    let p = new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve(new Object());
    });
    // NOTE(audovichenko): Remove p1 and p2 variables #17359
    let p1: Promise<void> = p.then<void>((): void => {
        if (fixture.state == 0) {
            fixture.state = 1;
        } else {
            globalTest!.fail();
        }
    });
    let p2: Promise<void> = p1.then<void>((): void => {
        if (fixture.state == 1) {
            fixture.state = 2;
        } else {
            globalTest!.fail();
        }
    });
    p2.then<void>((): void => {
        if (fixture.state == 2) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
    return 0;
}

function testPromiseCatchFinallyChain(): int {
    globalTest = new Test();
    let fixture = new PromiseChainFixture();
    let p: Promise<Object> = Promise.reject<Object>(new Error());
    // NOTE(audovichenko): Remove p1 and p2 variables #17359
    let p1: Promise<void> = p.then<void>((v: Object): void => {
        globalTest!.fail();
    });
    let p2: Promise<void> = p1.catch<void>((value: Error): void => {
        if (fixture.state == 0) {
            fixture.state = 1;
        } else {
            globalTest!.fail();
        }
    });
    p2.finally((): void => {
        if (fixture.state == 1) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
    return 0;
}

function testResolvePromiseThenFinally(): int {
    globalTest = new Test();
    let fixture = new PromiseChainFixture();
    let p = Promise.resolve<Object>(new Object());
    // NOTE(audovichenko): Remove p1 and p2 variables #17359
    let p1: Promise<void> = p.then<void, void>((value: Object): void => {
        if (fixture.state == 0) {
            fixture.state = 1;
        } else {
            globalTest!.fail();
        }
    }, (error: Any): void => {
        globalTest!.fail();
    });
    p1.finally((): void => {
        if (fixture.state == 1) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
    return 0;
}

function testRejectPromiseThenFinally(): int {
    globalTest = new Test();
    let fixture = new PromiseChainFixture();
    let p = Promise.reject<Object>(new Error());
    // NOTE(audovichenko): Remove p1 and p2 variables #17359
    let p1: Promise<void> = p.then<void, void>((value: Object): void => {
        globalTest!.fail();
    }, (error: Error): void => {
        if (fixture.state == 0) {
            fixture.state = 1;
        } else {
            globalTest!.fail();
        }
    });
    p1.finally((): void => {
        if (fixture.state == 1) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
    return 0;
}

function testReturnPromise(): Promise<Object> {
    return new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve("Panda");
    });
}

async function testReturnPromiseFromAsync(): Promise<Object> {
    return new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve("Panda");
    });
}

let resolvePromiseFn: ((value: Object) => void) | null = null;

function testReturnPendingPromise(): Promise<Object> {
    return new Promise<Object>((resolve: (value: Object) => void): void => {
        resolvePromiseFn = resolve;
    });
}

function resolvePendingPromise(): boolean {
    if (resolvePromiseFn == null) {
        return false;
    }
    (resolvePromiseFn as (value: Object) => void)("Panda");
    return true;
}

let unresolved1: Object | null = null;
let unresolved2: Object | null = null;
let unresolved3: Object | null = null;

async function asyncFuncAwait(): Promise<Object | null> {
    let promise: Promise<Object> = new Promise<Object>((resolve: (obj: Object) => void): void => {
        resolve("resolved");
    });
    unresolved1 = await promise;
    return null;
}

let asyncLambdaAwait: () => Promise<Object | null> = async (): Promise<Object | null> => {
    let promise: Promise<Object> = new Promise<Object>((resolve: (obj: Object) => void): void => {
        resolve("resolved");
    });
    unresolved2 = await promise;
    return null;
}

async function RejPromise1(): Promise<string> {
    let s: string = await new Promise<string>(
        (resolve: (obj: string|PromiseLike<string>) => void,
         reject: (obj: Error) => void): void => {
             reject(new Error("rejected"));
    });
    return s;
}

async function RejPromise2(): Promise<string> {
    return await RejPromise1();
}

async function asyncFuncAwaitRejected(): Promise<Object | null> {
    let promise: Promise<string> = RejPromise2();

    let promise2: Promise<string> =
    promise.then((x: string|Promise<string>): string => {
        return "smth";
    })

    try {
        unresolved3 = await promise;
        globalTest!.fail();
    } catch (e) {
        return null;
    }
    globalTest!.fail();
    return null;
}

function testAwaitPromise(): int {
    globalTest = new Test();
    let p1: Promise<Object | null> = asyncFuncAwait();
    let p2: Promise<Object | null> = asyncLambdaAwait();
    let p3: Promise<Object | null> = asyncFuncAwaitRejected();
    // execute async func (post await part) before check
    waitForCompletion(() => Promise.all([p1, p2, p3]));
    if (unresolved1 != "resolved" || unresolved2 != "resolved") {
        globalTest!.fail();
    }
    globalTest!.success();
    return 0;
}

function makeResolvedPromise(v: String): Promise<Object> {
    return new Promise<Object>((resolve: (value: Object) => void): void => {
        resolve(v);
    });
}

async function testAwaitJsPromiseSimple(p: Promise<Object>): Promise<void> {
    let res: String = (await p) as String;
}

async function testAwaitJsPromise(pendingP: Promise<Object>): Promise<Object> {
    globalTest = new Test();

    globalTest!.msg("testAwaitJsPromise: start", Test.INFO);
    let res: String = (await pendingP) as String;

    globalTest!.msg("testAwaitJsPromise: await result is: " + res, Test.INFO);

    if (res != "JS_PROMISE_RESULT") {
        globalTest!.msg("testAwaitJsPromise: await result is wrong!", Test.FATAL);
        globalTest!.fail();
    } else {
        globalTest!.msg("testAwaitJsPromise: await result is CORRECT!", Test.INFO);
        globalTest!.success();
    }

    globalTest!.msg("testAwaitJsPromise: done", Test.INFO);
    /**
     * NOTE(konstanting, #IAD5MH): add a test where we would return here a PENDING promise.
     * Currently this leads to a crash, because:
     *     - testAwaitJsPromise$asyncImpl resolves its linked promise (P1) with makeResolvedPromise's result (P2)
     *     - since we returned P1 to JS as PENDING (due to await presence), it has listeners
     *     - thus, P1 resolution triggers PandaEtsVM::FirePromiseStateChanged, which acquires promise_listeners_lock_
     *     - this leads to wrapping P1's resolution value (P2) into a JS promise
     *     - and since P2 is still PENDING, we must add a listener to it in order to notify JS of its resolution later on
     *     - this triggers PandaEtsVM::AddPromiseListener, which acquires already acquired promise_listeners_lock_ within
     *       the same thread
     *     - BOOM! Assertion fires.
     */
    return makeResolvedPromise("Panda");
}

async function testAsyncBoolean(): Promise<boolean> {
    return true;
}

async function testAsyncByte(): Promise<byte> {
    let x: byte = 127;
    return x;
}

async function testAsyncChar(): Promise<char> {
    return c'x';
}

async function testAsyncShort(): Promise<short> {
    let x: short = 32767;
    return x;
}

async function testAsyncInt(): Promise<int> {
    return 5;
}

async function testAsyncLong(): Promise<long> {
    return Long.MAX_VALUE;
}

async function testAsyncFloat(): Promise<float> {
    let x: float = 3.14f;
    return x;
}

async function testAsyncDouble(): Promise<double> {
    return 3.2;
}

async function testAsyncString(): Promise<String> {
    return "Panda";
}

async function testAsyncVoid(): Promise<void> { return undefined }
async function testAsyncVoidEmpty(): Promise<void> {}
async function testAsyncVoidNothing() {}

function testPromiseAllEmpty(): void {
    globalTest = new Test();
    let promises: FixedArray<Promise<Object>> = [];
    Promise.all<Object>(promises).then<void>((values: Array<Object>): void => {
        if (values == null || values.length != 0) {
            globalTest!.fail();
        } else {
            globalTest!.success();
        }
    });
}

function testPromiseAllResolved(): void {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.resolve<Object>("abc");
    let p2: Promise<Object> = Promise.resolve<Object>("def");
    let p3: Promise<Object> = Promise.resolve<Object>("xyz");

    Promise.all<Object>([
        p1, p2, p3
    ]).then<void>((values: Array<Object>): void => {
        if (values.length != 3) {
            globalTest!.fail();
        }
        if (values[0] != "abc") {
            globalTest!.fail();
        }
        if (values[1] != "def") {
            globalTest!.fail();
        }
        if (values[2] != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
    });
}

function testPromiseAllRejected(): void {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.resolve<Object>("abc");
    let p2: Promise<Object> = Promise.reject<Object>(new Error("def"));
    let p3: Promise<Object> = Promise.resolve<Object>("xyz");
    Promise.all([
        p1, p2, p3
    ]).then<void, void>((v: Object): void => {
        globalTest!.fail();
    }, (err: Error): void => {
        if (err.message != "def") {
            globalTest!.fail();
        } else {
            globalTest!.success();
        }
    });
}

function testPromiseAllRawValues(): void {
    globalTest = new Test();
    let p1: Object = "abc";
    let p2: Object = "def";
    let p3: Object = "xyz";

    Promise.all<Object>([
        p1, p2, p3
    ]).then<void>((values: Array<Object>): void => {
        if (values.length != 3) {
            globalTest!.fail();
        }
        if (values[0] != "abc") {
            globalTest!.fail();
        }
        if (values[1] != "def") {
            globalTest!.fail();
        }
        if (values[2] != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
    });
}

function testPromiseAllIterable(): void {
    globalTest = new Test();
    let array = new Array<Object|PromiseLike<Object>>();
    array.push(Promise.resolve<Object>("abc"));
    array.push("def");
    array.push(Promise.resolve<Object>("xyz"));

    Promise.all<Object>(array).then<void>((values: Array<Object>): void => {
        if (values.length != 3) {
            globalTest!.fail();
        }
        if (values[0] != "abc") {
            globalTest!.fail();
        }
        if (values[1] != "def") {
            globalTest!.fail();
        }
        if (values[2] != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
    });
}

function testPromiseAllSettledEmpty(): void {
    globalTest = new Test();
    let promises: FixedArray<Promise<Object>> = [];
    Promise.allSettled<Object>(promises).then<void, void>((values: PromiseSettledResult<Object>[]): void => {
        if (values.length != 0) {
            globalTest!.fail();
        }
        globalTest!.success();
    }, (err: Error): void => {
        globalTest!.fail();
    });
}

function testPromiseAllSettled(): void {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.resolve<Object>("abc");
    let p2: Promise<Object> = Promise.reject<Object>(new Error("def"));
    let p3: Promise<Object> = Promise.resolve<Object>("xyz");
    Promise.allSettled<Object>([
        p1, p2, p3
    ]).then<void, void>((result: PromiseSettledResult<Object>[]): void => {
        if (result.length != 3) {
            globalTest!.fail();
        }
        if (!(result[0] instanceof PromiseFulfilledResult)) {
            globalTest!.fail();
        }
        let res0 = result[0] as PromiseFulfilledResult<Object>;
        if (res0.status != PromiseStatus.fulfilled || res0.value != "abc") {
            globalTest!.fail();
        }
        if (!(result[1] instanceof PromiseRejectedResult)) {
            globalTest!.fail();
        }
        let res1 = result[1] as PromiseRejectedResult;
        if (res1.status != PromiseStatus.rejected || res1.reason.message != "def") {
            globalTest!.fail();
        }
        if (!(result[2] instanceof PromiseFulfilledResult)) {
            globalTest!.fail();
        }
        let res2 = result[2] as PromiseFulfilledResult<Object>;
        if (res2.status != PromiseStatus.fulfilled || res2.value != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
    }, (err: Error): void => {
        globalTest!.fail();
    });
}

function testPromiseAnyEmpty(): void {
    globalTest = new Test();
    let promises: FixedArray<Promise<Object>> = [];
    Promise.any(promises).then<void, void>((v: Object): void => {
        globalTest!.fail();
    }, (err: Error): void => {
        if (err != null && err instanceof AggregateError) {
            globalTest!.success();
        } else {
            globalTest!.fail();
        }
    });
}

function testPromiseAnyResolved(): void {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.reject<Object>(new Error("abc"));
    let p2: Promise<Object> = Promise.reject<Object>(new Error("def"));
    let p3: Promise<Object> = Promise.resolve<Object>("xyz");
    Promise.any([
        p1, p2, p3
    ]).then<void>((value: Object): void => {
        if (value != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
    });
}

function testPromiseAnyRejected(): void {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.reject<Object>(new Error("abc"));
    let p2: Promise<Object> = Promise.reject<Object>(new Error("def"));
    let p3: Promise<Object> = Promise.reject<Object>(new Error("xyz"));
    Promise.any([
        p1, p2, p3
    ]).then<void, void>((v: Object): void => {
        globalTest!.fail();
    }, (err: Error): void => {
        if (err == null || !(err instanceof AggregateError)) {
            globalTest!.fail();
        } else {
            globalTest!.success();
        }
    });
}

function testPromiseAnyRawValues(): void {
    globalTest = new Test();
    Promise.any([
        "abc", "def", "xyz"
    ]).then<void>((value: Object): void => {
        if (value != "abc") {
            globalTest!.fail();
        }
        globalTest!.success();
    });
}

function testPromiseAnyIterable(): void {
    globalTest = new Test();
    let array = new Array<Object|PromiseLike<Object>>();
    array.push(Promise.reject<Object>(new Error("def")));
    array.push(Promise.resolve<Object>("xyz"));
    Promise.any<Object>(array).then<void>((value: Object): void => {
        if (value != "xyz") {
            globalTest!.fail();
        }
        globalTest!.success();
    });
}

function testPromiseRaceResolved(): void {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.resolve<Object>("abc");
    let p2: Promise<Object> = Promise.reject<Object>(new Error("def"));
    Promise.race([
        p1, p2
    ]).then<void, void>((value: Object): void => {
        if (value != "abc") {
            globalTest!.fail();
        }
        globalTest!.success();
    }, (error: Error): void => {
        globalTest!.fail();
    });
}

function testPromiseRaceRejected(): void {
    globalTest = new Test();
    let p1: Promise<Object> = Promise.reject<Object>(new Error("abc"));
    let p2: Promise<Object> = Promise.resolve<Object>("def");
    Promise.race([
        p1, p2
    ]).then<void, void>((value: Object): void => {
        globalTest!.fail();
    }, (error: Error): void => {
        if (error.message != "abc") {
            globalTest!.fail();
        }
        globalTest!.success();
    });
}

function testPromiseThenOnRejection(): void {
    globalTest = new Test();
    Promise.reject<Error>(new Error("test")).then<string, string>(
    (value: Object): string => {
        globalTest!.fail();
        return "fail";
    }, (error: Error): string => {
        return "correct flow";
    }).then<string, void>((value: string): string => {
        globalTest!.success();
        return value as string
    }, (error: Error): void => {
        globalTest!.fail();
    });
}

let promiseFixture: Promise<void>|null = null;

function getPromise(): Promise<void> {
    promiseFixture = Promise.resolve();
    return promiseFixture!;
}

function setAndCheckPromise(p: Promise<void>): void {
    globalTest = new Test();
    if (promiseFixture == null) {
        console.log("Promise is not set");
        globalTest!.fail();
    }
    if (promiseFixture !== p) {
        console.log("Expected the returned promise and the argument are the same but they differs");
        globalTest!.fail();
    }
}

function getTheSamePromise(p: Promise<void>): Promise<void> {
    return p;
}
