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

const mustChecks: Array<Function> = new Array<Function>();

function main(): int {
    const suite = new arktest.ArkTestsuite("Stream API tests");

    suite.addAsyncTest("Multiple callbacks", testWritableMultipleCallbacks);
    suite.addAsyncTest("Callback repeatedly", testWritableCallbackRepeatedly);
    suite.addAsyncTest("Callback duplicate", testWritableCallbackDuplicate);
    suite.addAsyncTest("Write data error", testWritableWriteError);
    suite.addTest("Unknown encoding error", testWritableUnknownEncoding);

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

function mustCallUint8Array(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (chunk: string | Uint8Array, encoding: string, callback: Function) => {
        callCount++;
        fn?.unsafeCall(chunk, encoding, callback);
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

let callCount_MyWritable1 = 0;
class MyWritable1 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callCount_MyWritable1++;
        callback.unsafeCall();
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableCbTwice001
async function testWritableMultipleCallbacks() {
    mustChecks.push(() => {
        if (callCount_MyWritable1 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_MyWritable1} times.`);
        }
    });

    let writableStream = new MyWritable1();
    writableStream.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'Callback called multiple times.');
        arktest.assertEQ(error.name, 'BusinessError');
    }));
    writableStream.write('test');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable2 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall();
        setTimeout(() => {
            callback.unsafeCall();
        }, 0);
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableCbTwice002
async function testWritableCallbackRepeatedly() {
    let writableStream = new MyWritable2();
    writableStream.write('test');
    writableStream.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'Callback called multiple times.');
        arktest.assertEQ(error.name, 'BusinessError');
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable3 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        setTimeout(() => {
            callback.unsafeCall();
        }, 0);
        setTimeout(() => {
            callback.unsafeCall();
        }, 0);
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableCbTwice003
async function testWritableCallbackDuplicate() {
    let writableStream = new MyWritable3();
    writableStream.write('test');
    writableStream.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'Callback called multiple times');
        arktest.assertEQ(error.name, 'BusinessError');
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable4 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableWriteError001
async function testWritableWriteError() {
    let writableStream = new MyWritable4();
    let ticked = false;
    writableStream.end();
    writableStream.write('test', 'utf8', mustCall((error: Error): void => {
        arktest.assertTrue(ticked);
        arktest.assertEQ(error.message, 'Write after end.');
    }));
    ticked = true;
    writableStream.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'Write after end.');
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable5 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall();
    }
}
function testMyWritableDoWrite() {
    let writableStream = new MyWritable5();
    writableStream.write('my', 'test');
}

// Test cases ported from ArkTS 1.0:testStreamWritableWriteError003
function testWritableUnknownEncoding() {
    let exceptionCheck = (e: Error): boolean => {
        return ((e as BusinessError).message == "Parameter error. The type of test must be string.")
    }
    arktest.expectThrow(testMyWritableDoWrite, exceptionCheck);
}
