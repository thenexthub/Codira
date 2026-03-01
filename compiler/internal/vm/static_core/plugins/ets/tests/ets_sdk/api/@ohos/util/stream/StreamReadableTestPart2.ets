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

    suite.addTest("Resume behavior of readable stream", testReadableStreamResumeBehavior);
    suite.addAsyncTest("Set encoding and readable behavior", testStreamReadableSetEncodingAndReadableBehavior);
    suite.addTest("Data event handling in readable stream", testReadableStreamDataEventBehavior);
    suite.addTest("Set encoding parameter error in readable stream", testReadableStreamSetEncodingParameterError);
    suite.addTest("Emit readable event after data push", testReadableStreamEmitReadableBehavior);
    suite.addTest("Emit readable event for short stream", testReadableStreamEmitReadableShortStreamBehavior);
    suite.addTest("Readable ended state verification", testReadableStreamReadableEndedBehavior);
    suite.addAsyncTest("Edge case for readable ended state", testReadableStreamReadableEndedEdgeCase);
    suite.addTest("High watermark async behavior in readable stream", testReadableStreamHighWaterMarkAsyncBehavior);

    return suite.run();
}

function mustNotCall() {
    let fn: Function = (...params: Object[]) => { }
    return mustCall(fn, 0);
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

function mustCallString(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (str: string): void => {
        callCount++;
        fn?.unsafeCall(str);
    }

    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

class MyRead1 extends stream.Readable {
    doReadCalled = true;

    constructor() {
        super();
    }

    doRead(size: int) {
        this.push(null);
        this.doReadCalled = true;
    }
}
// Test cases ported from ArkTS 1.0:testStreamReadableConstructorSetMethods001
function testReadableStreamResumeBehavior() {
    let readable = new MyRead1();
    readable.resume();
    arktest.assertTrue(readable.doReadCalled);
}

class MyRead2 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamReadableData001
async function testStreamReadableSetEncodingAndReadableBehavior() {
    let readable = new MyRead2();
    readable.setEncoding('utf8');
    readable.on('readable', (): void => {
    });
    readable.off('readable');
    setTimeout(() => {
        readable.on('data', mustCallString());
        readable.push('hello');
        arktest.assertTrue(readable.readable, "readable should be true");
    }, 0);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyRead3 extends stream.Readable {
    dataSource = new Array<string>("x");

    constructor() {
        super();
    }

    doRead(size: int) {
        let data = this.dataSource.shift();
        this.push(data as string);
    }
}
// Test cases ported from ArkTS 1.0:testStreamReadableData002
function testReadableStreamDataEventBehavior() {
    let readableStream = new MyRead3();
    readableStream.on('data', (chunk: string): void => {
        arktest.assertEQ(chunk.toString(), 'x');
    });
}

// Test cases ported from ArkTS 1.0:testStreamReadableDefaultEncoding001
function testReadableStreamSetEncodingParameterError() {
    const test: Function = () => {
        let readableStream = new stream.Readable();
        readableStream.setEncoding('my valid encoding');
    }

    let exceptionCheck = (e: Error): string | boolean => {
        return ((e as BusinessError).message == "Parameter error. The type of my valid encoding must be string.");
    }
    arktest.expectThrow(() => {test()}, exceptionCheck);
}

// Test cases ported from ArkTS 1.0:testStreamReadableEmitReadableShortStream001
function testReadableStreamEmitReadableBehavior() {
    let readableStream = new TestReadable();
    readableStream.on('readable', (): void => {
        while (true) {
            let chunk = readableStream.read();
            if (!chunk) {
                break;
            }
            arktest.assertEQ((chunk as Object).toString(), 'content');
        }
    });
    readableStream.push('content');
    readableStream.push(null);
}

// Test cases ported from ArkTS 1.0:testStreamReadableEmitReadableShortStream002
function testReadableStreamEmitReadableShortStreamBehavior() {
    let readableStream = new TestReadable();
    let count = 0;
    readableStream.on('readable', (): void => {
        let chunk: Any = null;
        while ((chunk = readableStream.read()) !== null) {
            count++;
            arktest.assertEQ((chunk as Object).toString(), 'content');
        }
    });
    readableStream.on('end', (): void => {
        arktest.assertEQ(count, 1);
    });
    readableStream.push('content');
    readableStream.push(null);
}

class MyRead4 extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
        arktest.assertFalse(this.readableEnded, "readableEnded should be false");
        this.push('asd');
        arktest.assertFalse(this.readableEnded, "readableEnded should be false");
        this.push(null);
        arktest.assertFalse(this.readableEnded, "readableEnded should be false");
    }
}
// Test cases ported from ArkTS 1.0:testStreamReadableEnded002
function testReadableStreamReadableEndedBehavior() {
    let readableStream = new MyRead4();
    readableStream.on('end', (): void => {
        arktest.assertTrue(readableStream.readableEnded, "readableEnded should be true");
    });
    readableStream.on('data', (): void => {
        arktest.assertFalse(readableStream.readableEnded, "readableEnded should be false");
    });
}

// Test cases ported from ArkTS 1.0:testStreamReadableEnded003
async function testReadableStreamReadableEndedEdgeCase() {
    let readable = new stream.Readable();
    readable.on('readable', (): void => {
        readable.read();
    });
    arktest.assertTrue(readable.readable, "readable should be true");
    readable.on('error', mustNotCall());
    readable.on('end', mustCall());
    readable.push('a');
    readable.push(null);
    readable.push(null);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyRead5 extends stream.Readable {
    count = 5;

    constructor() {
        super();
    }

    doRead(size: int) {
        setTimeout(() => {
            if (this.count--) {
                this.push('a');
            } else {
                this.push(null);
            }
        }, 0);
    }
}
// Test cases ported from ArkTS 1.0:testStreamReadableHwm0Async001
function testReadableStreamHighWaterMarkAsyncBehavior() {
    let dataTimes = 0;

    let readable = new MyRead5();
    readable.on('end', (): void => {
        arktest.assertEQ(dataTimes, 5);
    });
    readable.on('data', (): void => {
        dataTimes++;
    });
}

class TestReadable extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
