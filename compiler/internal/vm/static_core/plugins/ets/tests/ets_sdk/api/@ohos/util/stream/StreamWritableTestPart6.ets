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

    suite.addTest("Writable cork uncork", testWritableCorkUncork);
    suite.addTest("Writable cork uncork multiple times", testWritableCorkUncorkMultipleTimes);
    suite.addTest("Writable cork uncork partial flush", testWritableCorkUncorkPartialFlush);
    suite.addTest("Writable remove listeners", testWritableRemoveListeners);
    suite.addTest("Writable remove all listeners", testWritableRemoveAllListeners);
    suite.addTest("Writable default encoding", testWritableDefaultEncoding);
    suite.addTest("Writable set default encoding", testWritableSetDefaultEncoding);
    suite.addTest("Writable set default encoding undefined", testWritableSetDefaultEncodingUndefined);
    suite.addTest("Writable set default encoding empty string", testWritableSetDefaultEncodingEmptyString);
    suite.addTest("Writable default encoding behavior", testWritableDefaultEncodingBehavior);
    suite.addTest("Writable drain event high watermark", testWritableDrainEventHighWatermark);
    suite.addAsyncTest("Writable drain event async high watermark", testWritableDrainEventAsyncHighWatermark);

    return suite.run();
}

function mustCallTwoArgs(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (chunks: string[] | Uint8Array[], callback: Function) => {
        callCount++;
        fn?.unsafeCall(chunks, callback);
    }
    mustChecks.push(() => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
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

class MyWritable1 extends stream.Writable {
    dataWritten: string = '';

    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.dataWritten += chunk;
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableCork001
function testWritableCorkUncork() {
    let writableStream = new MyWritable1();
    writableStream.cork();
    writableStream.write('a');
    writableStream.write('b');
    writableStream.uncork();
    arktest.assertEQ(writableStream.dataWritten, 'ab');
}

class MyWritable2 extends stream.Writable {
    dataWritten = '';

    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        this.dataWritten += chunk;
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableCork002
function testWritableCorkUncorkMultipleTimes() {
    let writableStream = new MyWritable2();
    writableStream.cork();
    writableStream.write('a');
    writableStream.cork();
    writableStream.write('b');
    writableStream.cork();
    writableStream.write('c');
    writableStream.uncork();
    writableStream.uncork();
    writableStream.uncork();
    arktest.assertEQ(writableStream.dataWritten, 'abc');
}

class MyWritable3 extends stream.Writable {
    dataWritten = '';

    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.dataWritten += chunk;
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableCork003
function testWritableCorkUncorkPartialFlush() {
    let writableStream = new MyWritable3();
    writableStream.cork();
    writableStream.write('a');
    writableStream.cork();
    writableStream.write('b');
    writableStream.cork();
    writableStream.write('c');
    writableStream.uncork();
    arktest.assertEQ(writableStream.dataWritten, '');
}

class MyWritable4 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableOff001
function testWritableRemoveListeners() {
    let writableStream = new MyWritable4();
    let testListenerCalled = false;
    let testListener: Function = (): void => {
        testListenerCalled = true;
    };
    writableStream.on('finish', testListener);
    writableStream.off('finish', testListener);
    writableStream.write('test');
    writableStream.end();
    setTimeout(() => {
        arktest.assertFalse(testListenerCalled);
    }, 0);
}

class MyWritable5 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableOff002
function testWritableRemoveAllListeners() {
    let writableStream = new MyWritable5();
    let testListener1Called = false;
    let testListener2Called = false;
    let testListener1: Function = (): void => {
        testListener1Called = true;
    };
    let testListener2: Function = (): void => {
        testListener2Called = true;
    };
    writableStream.on('finish', testListener1);
    writableStream.on('finish', testListener2);
    writableStream.off('finish');
    writableStream.write('test');
    writableStream.end();
    setTimeout(() => {
        arktest.assertFalse(testListener1Called);
        arktest.assertFalse(testListener2Called);
    }, 0);
}

// Test cases ported from ArkTS 1.0:testStreamWritableChangeDefaultEncoding001
function testWritableDefaultEncoding() {
    let testWritable = new TestWritable((chunk, type, enc) => {
        arktest.assertEQ(enc, 'utf-8');
    });
    testWritable.write('test');
    testWritable.end();
}

// Test cases ported from ArkTS 1.0:testStreamWritableChangeDefaultEncoding002
function testWritableSetDefaultEncoding() {
    let testWritable = new TestWritable((chunk, type, enc) => {
        arktest.assertEQ(enc, 'utf-8');
        arktest.assertEQ(type, 'string');
    });
    testWritable.setDefaultEncoding('ascii');
    testWritable.write('car');
    testWritable.end();
}

function testWritableSetDefaultEncodingUndefined() {
    const x = new stream.Writable;
    arktest.assertFalse(x.setDefaultEncoding());
}

function testWritableSetDefaultEncodingEmptyString() {
    const x = new stream.Writable;
    arktest.assertFalse(x.setDefaultEncoding(""));
}

// Test cases ported from ArkTS 1.0:testStreamWritableChangeDefaultEncoding003
function testWritableDefaultEncodingBehavior() {
    const test: Function = () => {
        let testWritable = new TestWritable(
            (type, enc) => {
            });
        testWritable.setDefaultEncoding("abc");
        testWritable.write('bar');
        testWritable.end();
    }
    let exceptionCheck = (e: Error): boolean => {
        return ((e as BusinessError).message == "Parameter error. The type of abc must be string.")
    }
    arktest.expectThrow(test, exceptionCheck);
}

class MyWritable6 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        setTimeout(() => {
            callback.unsafeCall()
        }, 0);
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableDrain001
function testWritableDrainEventHighWatermark() {
    let writableStream = new MyWritable6();
    let drainCalled = false;
    writableStream.on('drain', (): void => {
        drainCalled = true;
    });
    arktest.assertFalse(drainCalled);
    writableStream.write('test');
    writableStream.end();
}

let callCount_MyWritable7 = 0;
class MyWritable7 extends stream.Writable {
    doWriteVCalled = false;

    constructor() {
        super();
    }

    doWritev(chunks: string[] | Uint8Array[], callback: Function): void {
        callCount_MyWritable7++;
        this.doWriteVCalled = true;
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableWritev001
async function testWritableDrainEventAsyncHighWatermark() {
    mustChecks.push(() => {
        if (callCount_MyWritable7 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_MyWritable7} times.`);
        }
    });

    let writable = new MyWritable7();
    writable.write('test', 'utf8', mustCallNoArgs((): void => {
    }))
    arktest.assertTrue(writable.doWriteVCalled);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class TestWritable extends stream.Writable {
    private fn: (chunk: string | Uint8Array, type: string, enc: string) => void;

    constructor(fn: (chunk: string | Uint8Array, type: string, enc: string) => void) {
        super();
        this.fn = fn;
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        this.fn(chunk, typeof chunk, encoding);
        callback.unsafeCall();
    }
}
