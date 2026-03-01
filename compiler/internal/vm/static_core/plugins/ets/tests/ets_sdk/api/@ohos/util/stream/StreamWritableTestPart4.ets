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

    suite.addAsyncTest("Pipe error handling", testStreamWritablePipeErrorHandling);
    suite.addAsyncTest("Write after finish error", testStreamWritableWriteAfterFinishError);
    suite.addTest("Write after finish error", testStreamWritableConstructorSetMethodsError);
    suite.addAsyncTest("End event error", testWritableEndEventError);
    suite.addTest("Writable ended state", testWritableEndedState);
    suite.addAsyncTest("Writable finished state", testWritableFinishedState);
    suite.addTest("Writable finish event", testWritableFinishEvent);
    suite.addTest("Writable prefinish event", testWritablePrefinishEvent);
    suite.addTest("Writable finish sync state", testWritableFinishSyncState);

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

function mustNotCall() {
    let fn: Function = () => { }
    return mustCall(fn, 0);
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

class MyRead1 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
class MyWritable1 extends stream.Writable {
    constructor() {
        super();
    }

    doWritev(chunks: string[] | Uint8Array[], callback: Function): void {
        callback.unsafeCall(new Error());
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableWritevFinish005
async function testStreamWritablePipeErrorHandling() {
    let readableStream = new MyRead1();
    readableStream.push('ok');
    readableStream.push(null);
    let errorCalled = false;
    let writableStream = new MyWritable1();
    writableStream.on('finish', mustNotCall());
    writableStream.on('error', mustCall((): void => {
        errorCalled = true;
    }));
    readableStream.pipe(writableStream);
    setTimeout(() => {
        arktest.assertTrue(errorCalled);
    }, 50);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable2 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        setTimeout(() => {
            callback.unsafeCall();
        });
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableWritevFinish006
async function testStreamWritableWriteAfterFinishError() {
    let writableStream = new MyWritable2();
    writableStream.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'Write after end.');
    }));
    writableStream.on('finish', (): void => {
        writableStream.write('test');
    });
    writableStream.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamWritableConstructorSetMethods001
function testStreamWritableConstructorSetMethodsError() {
    let writableStream = new stream.Writable();
    const test = (): void => {
        writableStream.end('test');
    }
    let exceptionCheck = (e: Error): boolean => {
        return ((e as BusinessError).message == "The doWrite() method is not implemented.")
    }
    arktest.expectThrow(() => { test(); }, exceptionCheck);
}

class MyWritable3 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        setTimeout(() => {
            callback.unsafeCall(new Error('new error'));
        }, 0);
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableEndCbError001
async function testWritableEndEventError() {
    const _error = new Error('new error');
    let writable = new MyWritable3();
    writable.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'new error');
    }));
    writable.write('test');
    writable.end(undefined, undefined, mustCall((error: Error): void => {
        arktest.assertEQ(error.message, _error.message);
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable4 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        arktest.assertEQ(this.writableEnded, false);
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableEndedState001
function testWritableEndedState() {
    let endCalled = false;
    let writableStream = new MyWritable4();
    arktest.assertEQ(writableStream.writable, true);
    arktest.assertEQ(writableStream.writableEnded, false);
    writableStream.end('testing ended', 'utf8', (): void => {
        arktest.assertEQ(writableStream.writable, false);
        arktest.assertEQ(writableStream.writableEnded, true);
        endCalled = true;
    });
    arktest.assertEQ(writableStream.writable, false);
    arktest.assertEQ(writableStream.writableEnded, true);
    writableStream.on('finish', (): void => {
        arktest.assertTrue(endCalled);
    });
}

class MyWritable5 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        arktest.assertEQ(this.writableFinished, false);
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableFinished002
async function testWritableFinishedState() {
    let writable = new MyWritable5();
    writable.on('finish', mustCall((): void => {
        arktest.assertEQ(writable.writableFinished, true);
    }));
    writable.end('testing finished state', undefined, mustCall((): void => {
        arktest.assertEQ(writable.writableFinished, true);
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable6 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableFinished003
function testWritableFinishEvent() {
    let writableStream = new MyWritable6();
    let finishCalled = false;
    writableStream.end();
    writableStream.on('finish', (): void => {
        finishCalled = true;
    });
    writableStream.on('finish', (): void => {
        arktest.assertTrue(finishCalled);
    });
}

class MyWritable7 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableFinished004
function testWritablePrefinishEvent() {
    let writableStream = new MyWritable7();
    let sync = true;
    writableStream.on('prefinish', (): void => {
        arktest.assertTrue(sync);
    });
    writableStream.end();
    sync = false;
}

class MyWritable8 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableFinished005
function testWritableFinishSyncState() {
    let writableStream = new MyWritable8();
    let sync = true;
    writableStream.on('finish', (): void => {
        sync = false;
    });
    writableStream.on('prefinish', (): void => {
        arktest.assertTrue(sync);
    });
    writableStream.end();
    sync = false;
}
