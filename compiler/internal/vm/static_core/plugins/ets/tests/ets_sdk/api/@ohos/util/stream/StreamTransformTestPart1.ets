/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License')
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { BusinessError } from "@ohos.base";
import { stream } from "@ohos.util.stream";
import buffer from '@ohos.buffer';

const mustChecks: Array<Function> = new Array<Function>();

function main(): int {
    const suite = new arktest.ArkTestsuite("Stream API tests");

    suite.addAsyncTest("Call Callback twice in Transform.", testTransformCallbackTwice);
    suite.addTest("Transform constructor.", testTransformConstructor);
    suite.addTest("Event triggering of Transform.", testTransformEventTriggering);
    suite.addTest("Event handling of Transform.", testTransformEventHandling);
    suite.addTest("Initialization of Transform.", testTransformInitialization);
    suite.addTest("Default encoding of Transform.", testTransformDefaultEncoding);
    suite.addAsyncTest("Read operation of Transform.", testTransformReadOperation);
    suite.addTest("Pause behavior of Transform.", testTransformPauseBehavior);
    suite.addTest("Resume behavior of Transform.", testTransformResumeBehavior);
    suite.addTest("Set encoding of Transform.", testTransformSetEncoding);
    suite.addTest("Pause state of Transform.", testTransformPauseState);
    suite.addAsyncTest("Read callback of Transform.", testTransformReadCallback);
    suite.addTest("Writable properties of Transform.", testTransformWritableProperties);
    suite.addTest("Readable properties of Transform.", testTransformReadableProperties);

    return suite.run();
}

async function mustCallCheckAll() {
    return new Promise<void>((resolve, reject) => {
        setTimeout(() => {
            try {
                mustChecks.forEach(check => {
                    check.unsafeCall();
                });
                mustChecks.length = 0;
                resolve(undefined);
            } catch (e) {
                mustChecks.length = 0;
                reject(e)
            }
        }, 2000);
    })
}

function mustCallArgSize(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (size: int) => {
        callCount++;
        fn?.unsafeCall(size);
    }
    mustChecks.push(() => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

function mustCall(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (err: Error | undefined | null): void => {
        callCount++;
        fn?.unsafeCall(err);
    }

    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

function mustCallChunk(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (chunk: Object): void => {
        callCount++;
        fn?.unsafeCall(chunk);
    }

    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

class MyTransform1 extends stream.Transform {
    constructor() {
        super();
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        callback.unsafeCall();
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransformCallbackTwice001
async function testTransformCallbackTwice() {
    let transform = new MyTransform1();
    transform.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.name, 'BusinessError');
        arktest.assertEQ(error.message, 'Callback called multiple times.');
    }));

    transform.write('test');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamTransformConstructor001
function testTransformConstructor() {
    let transform = new stream.Transform();
    const test: Function = () => {
        transform.end('test');
    }

    let exceptionCheck = (e: Error): string | boolean => {
        return ((e as BusinessError).message == "The doTransform() method is not implemented.");
    }
    arktest.expectThrow(() => { test(); }, exceptionCheck);
}

class MyTransform2 extends stream.Transform {
    constructor() {
        super();
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        let transformedChunk = chunk.toString().toUpperCase();
        this.push(transformedChunk);
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransform001
function testTransformEventTriggering() {
    let transform = new MyTransform2();
    let transformedData = '';
    transform.on('data', (chunk: buffer.Buffer): void => {
        transformedData += chunk.toString();
    });
    transform.on('end', (): void => {
        arktest.assertEQ(transformedData, 'TEST DATA');
    });
    transform.write('Test data');
    transform.end();
}

class Transform3 extends stream.Transform {
    constructor() {
        super();
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        this.push(chunk + ' ');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransform002
function testTransformEventHandling() {
    let transform = new Transform3();
    let endCalled = false;
    transform.on('end', (): void => {
        endCalled = true;
    });
    transform.write('A');
    transform.end();
    arktest.assertFalse(endCalled);
}

class MyTransform4 extends stream.Transform {
    doInitializeCalled: boolean = false;

    doInitialize(callback: Function) {
        super.doInitialize(callback);
        this.doInitializeCalled = true;
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        super.doWrite(chunk, encoding, callback);
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransform003
function testTransformInitialization() {
    let myTransform = new MyTransform4();
    myTransform.on('data', (): void => {
    });
    arktest.assertTrue(myTransform.doInitializeCalled);
}

// Test cases ported from ArkTS 1.0:testStreamTransform004
function testTransformDefaultEncoding() {
    let transformStream = new stream.Transform();
    let result = transformStream.setDefaultEncoding('utf8');
    arktest.assertTrue(result);
}
class MyTransform5 extends stream.Transform {
    dataSource = new Array<string>("x");

    doRead(size: int) {
        let data = this.dataSource.shift();
        if (data) {
            this.push(data as string);
        } else {
            this.push(null);
        }
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransform005
async function testTransformReadOperation() {
    let transformStream = new MyTransform5();
    let func: Function = mustCallChunk((chunk: Object): void => {
        arktest.assertEQ(chunk.toString(), 'x');
    })
    transformStream.on('data', (chunk: Object): void => {
        func.unsafeCall(chunk);
    });
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamTransform006
function testTransformPauseBehavior() {
    let transformStream = new stream.Transform();
    let dataCalled = false;
    transformStream.on('data', (): void => {
        dataCalled = true;
    });
    transformStream.pause();
    transformStream.push('test');
    arktest.assertEQ(transformStream.readable, true);
    arktest.assertEQ(transformStream.writable, true);
    arktest.assertTrue(transformStream.isPaused());
    setTimeout(() => {
        arktest.assertFalse(dataCalled);
    }, 10);
}

// Test cases ported from ArkTS 1.0:testStreamTransform007
function testTransformResumeBehavior() {
    let transformStream = new stream.Transform();
    let dataCalled = false;
    transformStream.on('data', (): void => {
        dataCalled = true;
    });
    transformStream.pause();
    transformStream.resume();
    transformStream.push('test');
    arktest.assertEQ(transformStream.readable, true);
    arktest.assertEQ(transformStream.writable, true);
    setTimeout(() => {
        arktest.assertTrue(dataCalled);
    }, 10);
}

// Test cases ported from ArkTS 1.0:testStreamTransform008
function testTransformSetEncoding() {
    let transformStream = new stream.Transform();
    transformStream.setEncoding('utf8');
    arktest.assertEQ(transformStream.readableEncoding, 'utf-8');
}

// Test cases ported from ArkTS 1.0:testStreamTransform009
function testTransformPauseState() {
    let transformStream = new stream.Transform();
    arktest.assertFalse(transformStream.isPaused())
    transformStream.pause();
    arktest.assertTrue(transformStream.isPaused());
}

let callCount_MyTransform6 = 0;
class MyTransform6 extends stream.Transform {
    doReadCalled = false;

    constructor() {
        super();
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        callback.unsafeCall();
    }

    doRead(size: int) {
        callCount_MyTransform6++;
        this.doReadCalled = true;
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransform012
async function testTransformReadCallback() {
    mustChecks.push(() => {
        if (callCount_MyTransform6 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_MyTransform6} times.`);
        }
    });

    let transform = new MyTransform6();
    transform.on('data', (): void => {
    });
    setTimeout(() => {
        arktest.assertTrue(transform.doReadCalled);
    }, 0);
    setTimeout(() => {
        arktest.expectNoThrow(() => { await mustCallCheckAll(); });
    }, 10);
}

// Test cases ported from ArkTS 1.0:testStreamTransformProperties001
function testTransformWritableProperties() {
    let transform = new stream.Transform();
    arktest.assertEQ(transform.writableHighWatermark, 16 * 1024);
}

// Test cases ported from ArkTS 1.0:testStreamTransformProperties002
function testTransformReadableProperties() {
    let transform = new stream.Transform();
    arktest.assertEQ(transform.readableHighWatermark, 16 * 1024);
}
