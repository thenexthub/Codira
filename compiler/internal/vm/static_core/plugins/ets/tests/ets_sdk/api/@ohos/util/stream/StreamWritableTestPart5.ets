/*
 * Copyright (C) 2025-2026 Huawei Device Co., Ltd.
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

    suite.addTest("Writable properties", testWritableProperties);
    suite.addTest("Writable object mode", testWritableObjectMode);
    suite.addTest("Writable high watermark", testWritableHighWatermark);
    suite.addTest("Writable finish state", testWritableFinishState);
    suite.addTest("Writable length during write", testWritableLengthDuringWrite);
    suite.addTest("Writable initial length", testWritableInitialLength);
    suite.addTest("Custom writable length", testCustomWritableLength);
    suite.addTest("Custom writable length after multiple writes", testCustomWritableLengthAfterMultipleWrites);
    suite.addAsyncTest("Writable length after finish", testWritableLengthAfterFinish);
    suite.addTest("Writable write large data", testWritableWriteLargeData);
    suite.addAsyncTest("Writable drain after large data", testWritableDrainAfterLargeData);
    suite.addAsyncTest("Writable drain event", testWritableDrainEvent);
    suite.addTest("Writable state after end", testWritableStateAfterEnd);
    suite.addTest("Writable cork uncork state", testWritableCorkUncorkState);
    suite.addTest("Writable error event", testWritableErrorEvent);
    suite.addTest("Writable error handling", testWritableErrorHandling);
    suite.addAsyncTest("Writable close event", testWritableCloseEvent);

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

// Test cases ported from ArkTS 1.0:testStreamWritableProperties001
function testWritableProperties() {
    let writableStream = new stream.Writable();
    arktest.assertEQ(writableStream.writableCorked, 0);
    writableStream.uncork();
    arktest.assertEQ(writableStream.writableCorked, 0);
    writableStream.cork();
    arktest.assertEQ(writableStream.writableCorked, 1);
    writableStream.cork();
    arktest.assertEQ(writableStream.writableCorked, 2);
    writableStream.uncork();
    arktest.assertEQ(writableStream.writableCorked, 1);
    writableStream.uncork();
    arktest.assertEQ(writableStream.writableCorked, 0);
    writableStream.uncork();
    arktest.assertEQ(writableStream.writableCorked, 0);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties002
function testWritableObjectMode() {
    let writableStream = new stream.Writable();
    arktest.assertEQ(writableStream.writableObjectMode, false);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties003
function testWritableHighWatermark() {
    let writableStream = new stream.Writable();
    arktest.assertEQ(writableStream.writableHighWatermark, 16 * 1024);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties004
function testWritableFinishState() {
    let writableStream = new stream.Writable();
    writableStream.on('finish', (): void => {
        arktest.assertEQ(writableStream.writable, false);
    });
    writableStream.end();
}

class MyWritable1 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        setTimeout(() => {
            callback.unsafeCall();
        });
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableProperties005
function testWritableLengthDuringWrite() {
    let writableStream = new MyWritable1();
    writableStream.write('hello');
    arktest.assertEQ(writableStream.writableLength, 5);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties006
function testWritableInitialLength() {
    let writableStream = new stream.Writable();
    arktest.assertEQ(writableStream.writableLength, 0);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties007
function testCustomWritableLength() {
    let customWritable = new CustomWritable();
    customWritable.write('hello');
    arktest.assertEQ(customWritable.writableLength, 5);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties008
function testCustomWritableLengthAfterMultipleWrites() {
    let customWritable = new CustomWritable();
    customWritable.write('hello');
    customWritable.write('world');
    arktest.assertEQ(customWritable.writableLength, 10);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties009
async function testWritableLengthAfterFinish() {
    let customWritable = new CustomWritable();
    customWritable.write('hello');
    arktest.assertEQ(customWritable.writableLength, 5);
    customWritable.end();
    await customWritable.waitForFinish();
    arktest.assertEQ(customWritable.writableLength, 0);
}

class MyWritable2 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        setTimeout(() => {
            callback.unsafeCall();
        }, 10);
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableProperties010
function testWritableWriteLargeData() {
    const largeData = 'ab'.repeat(Math.floor(16384 / 2).toInt());
    let writableStream = new MyWritable2();
    let writeResult = writableStream.write(largeData);
    arktest.assertFalse(writeResult);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties011
async function testWritableDrainAfterLargeData() {
    const largeData = 'ab'.repeat(Math.floor(16384 / 2).toInt());
    let writableStream = new CustomWritable();
    writableStream.write(largeData);
    await new Promise<void>((resolve) => {
        writableStream.on('drain', (): void => {
            resolve(undefined);
        });
    });
    writableStream.write(largeData, 'utf8', mustCallNoArgs((): void => {}));
    arktest.assertEQ(writableStream.writable, true);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties012
async function testWritableDrainEvent() {
    const largeData = 'ab'.repeat(Math.floor(16384 / 2).toInt());
    let writableStream = new CustomWritable();
    writableStream.write(largeData);
    await writableStream.waitForDrain();
    arktest.assertEQ(writableStream.writableEnded, false);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties013
function testWritableStateAfterEnd() {
    const largeData = 'ab'.repeat(Math.floor(16384 / 2).toInt());
    let writableStream = new CustomWritable();
    writableStream.write(largeData);
    arktest.assertEQ(writableStream.writable, true);
    writableStream.end();
    arktest.assertEQ(writableStream.writable, false);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties014
function testWritableCorkUncorkState() {
    let writableStream = new CustomWritable();
    arktest.assertEQ(writableStream.writableCorked, 0);
    writableStream.cork();
    arktest.assertEQ(writableStream.writableCorked, 1);
    writableStream.write('Hello ');
    writableStream.write('World!');
    writableStream.uncork();
    arktest.assertEQ(writableStream.writableCorked, 0);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties015
function testWritableErrorEvent() {
    let errorEvent = new ErrorEventWritable();
    let chunks = 'C';
    errorEvent.on('error', (error: Error): void => {
        arktest.assertEQ(error.message, 'error');
    });
    errorEvent.write(chunks);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties016
function testWritableErrorHandling() {
    let errorEvent = new ErrorEventWritable();
    let chunks = 'C';
    errorEvent.on('error', (error: Error): void => {
        arktest.assertEQ(error.message, 'error');
    });
    errorEvent.write(chunks);
}

// Test cases ported from ArkTS 1.0:testStreamWritableProperties017
async function testWritableCloseEvent() {
    let writableStream = new stream.Writable();
    writableStream.on('close', mustCall((): void => {
        arktest.assertEQ(writableStream.writableFinished, true);
    }));
    writableStream.end();
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class CustomWritable extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        setTimeout(() => {
            callback.unsafeCall();
        }, 0);
    }

    async waitForFinish(): Promise<void> {
        return new Promise<void>((resolve) => {
            this.on('finish', (): void => {
                resolve(undefined);
            });
        });
    }

    async waitForDrain(): Promise<void> {
        return new Promise<void>((resolve) => {
            this.on('drain', (): void => {
                resolve(undefined);
            });
        });
    }
}

class ErrorEventWritable extends stream.Writable {
    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        if (chunk as string === 'C') {
            callback.unsafeCall(new Error('error'));
        } else {
            callback.unsafeCall();
        }
    }
}
