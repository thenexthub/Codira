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

    suite.addTest("Create a writable stream", testWritableCreation);
    suite.addTest("The state when closing the flow", testWritableCloseState);
    suite.addAsyncTest("The state of the writable attribute of a writable", testWritableAttributeState);
    suite.addAsyncTest("Write data to stream", testWritableWriteData);
    suite.addAsyncTest("DoWrite not called", testWritableDoWriteNotCalled);
    suite.addAsyncTest("Write data failed", testWritableWriteDataFailed);
    suite.addTest("Error on write", testWritableErrorOnWrite);
    suite.addTest("Write on Error Sequence without end", testWritableOnErrorSequence);
    suite.addTest("Write on Error Sequence with end", testWritableOnErrorSequenceWithEnd);

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

function mustCallNoArgs(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (): void => {
        callCount++;
        fn?.unsafeCall();
    }

    mustChecks.push((): void => {
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

// Test cases ported from ArkTS 1.0:testStreamWritable001
function testWritableCreation() {
    let writableStream = new stream.Writable();
    arktest.assertEQ(writableStream.writable, true);
    writableStream.end();
    arktest.assertEQ(writableStream.writable, false);
}

// Test cases ported from ArkTS 1.0:testStreamWritable002
function testWritableCloseState() {
    let writableStream = new stream.Writable();
    arktest.assertEQ(writableStream.writable, true);
    arktest.assertEQ(writableStream.writableEnded, false);
    writableStream.end();
    arktest.assertEQ(writableStream.writable, false,);
    arktest.assertEQ(writableStream.writableEnded, true);
}

class MyWritable1 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall(new Error());
        arktest.assertEQ(this.writable, false);
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritable003
async function testWritableAttributeState() {
    let writableStream = new MyWritable1();
    arktest.assertEQ(writableStream.writable, true);
    writableStream.on('error', mustCall());
    writableStream.write('test');
    arktest.assertEQ(writableStream.writable, false);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable2 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        arktest.assertEQ((chunk as string).toString(), 'write content');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritable004
async function testWritableWriteData() {
    let param: Object[] = [];
    let writableStream = new MyWritable2();
    writableStream.write('write content', 'utf8', (...param: Object[]): void => {
        if (param.length > 0 && param[0] instanceof Error) {
            const error = param[0] as Error;
            arktest.assertEQ(error, null);
        }
    });
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyWritable3 = 0;
class MyWritable3 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callCount_MyWritable3++;
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritable005
async function testWritableDoWriteNotCalled() {
    mustChecks.push(() => {
        if (callCount_MyWritable3 !== 0) {
            throw new Error(`Function was expected to be called 0 times, `
                            + `but was called ${callCount_MyWritable3} times.`);
        }
    });

    let writableStream = new MyWritable3();
    arktest.assertEQ(writableStream.writable, true);
    writableStream.end();
    arktest.assertEQ(writableStream.writable, false);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable4 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall(new Error());
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableCbError001
async function testWritableWriteDataFailed() {
    let callbackCalled = false;

    let writable = new MyWritable4();
    writable.on('error', mustCallNoArgs((): void => {
        arktest.assertTrue(callbackCalled);
    }));
    writable.write('hello', 'utf8', mustCallNoArgs((): void => {
        callbackCalled = true;
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable5 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall(new Error());
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableCbError002
function testWritableErrorOnWrite() {

    let writableStream = new MyWritable5();
    writableStream.on('error', (): void => {
    })
    let num = 0;
    while (writableStream.write('t')) {
        num++;
    }
    arktest.assertEQ(num, 0);
}

class ErrorEventWritable extends stream.Writable {
    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
      if (chunk.toString() === 'C') {
        callback.unsafeCall(new Error('error'));
      } else {
        callback.unsafeCall();
      }
    }
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties015
function testWritableOnErrorSequence()
{
    let writableStream = new ErrorEventWritable();
    let chunks = 'C';
    writableStream.write(chunks);
    writableStream.on('error', (error: Error) => {
        arktest.assertEQ(error.message, 'error');
    });
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties016
function testWritableOnErrorSequenceWithEnd()
{
    let writableStream = new ErrorEventWritable();
    let chunks = 'C';
    writableStream.write(chunks);
    writableStream.on('error', (error: Error) => {
        arktest.assertEQ(error.message, 'error');
    });
    writableStream.end();
}

