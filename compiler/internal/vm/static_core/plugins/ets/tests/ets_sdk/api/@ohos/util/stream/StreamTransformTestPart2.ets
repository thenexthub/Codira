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

    suite.addTest("Data event of Transform.", testTransformDataEvent);
    suite.addAsyncTest("Cork and uncork of Transform.", testTransformCorkUncork);
    suite.addAsyncTest("Error event of Transform.", testTransformErrorEvent);
    suite.addTest("Remove listener of Transform.", testTransformRemoveListener);
    suite.addTest("Multiple listeners of Transform.", testTransformMultipleListeners);
    suite.addTest("Flush operation of Transform.", testTransformFlushOperation);

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

function mustCallFn(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn = (chunk: string, encoding: string, callback: Function) => {
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

class TestTransform1 extends stream.Transform {
    constructor() {
        super();
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        let transformedChunk = chunk.toUpperCase();
        this.push(transformedChunk);
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransformEvent001
function testTransformDataEvent() {
    let transformedData = '';
    let transform = new TestTransform1();
    transform.on('data', (chunk: buffer.Buffer): void => {
        transformedData += chunk.toString();
    });
    transform.on('end', (): void => {
        arktest.assertEQ(transformedData, 'TEST DATA');
    });
    transform.write('Test data');
    transform.end();
}

let callCount_TestTransform2 = 0;
class TestTransform2 extends stream.Transform {
    constructor() {
        super();
    }

    doWritev(chunks: string[] | Uint8Array[], callback: Function) {
        callback.unsafeCall();
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        callCount_TestTransform2++;
        arktest.assertEQ(chunk, 'data1');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransformEvent002
async function testTransformCorkUncork() {
    mustChecks.push(() => {
        if (callCount_TestTransform2 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_TestTransform2} times.`);
        }
    });

    let transformStream = new TestTransform2();
    transformStream.cork();
    transformStream.write('data1', 'utf8');
    transformStream.uncork();
    transformStream.end();
    let func: Function = mustCall();
    transformStream.on('finish', (): void => {
        func(null);
    });
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_TestTransform3 = 0;
class TestTransform3 extends stream.Transform {
    constructor() {
        super();
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        callCount_TestTransform3++;
        arktest.assertEQ(chunk, 'test');
        callback.unsafeCall(new Error('Write error'));
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransformEvent003
async function testTransformErrorEvent() {
    mustChecks.push(() => {
        if (callCount_TestTransform3 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_TestTransform3} times.`);
        }
    });

    let transformStream = new TestTransform3();
    transformStream.on('error', mustCall());
    transformStream.write('test');
    setTimeout(() => {
        arktest.expectNoThrow(() => { await mustCallCheckAll(); });
    }, 10);
}

// Test cases ported from ArkTS 1.0:testStreamTransformEvent004
function testTransformRemoveListener() {
    let transformStream = new stream.Transform();
    let testListenerCalled = false;
    let testListener: Function = (): void => {
        testListenerCalled = true;
    };
    transformStream.setEncoding('utf8');
    transformStream.on('readable', testListener);
    transformStream.off('readable');
    transformStream.push('test');
    arktest.assertFalse(testListenerCalled);
}

// Test cases ported from ArkTS 1.0:testStreamTransformEvent005
function testTransformMultipleListeners() {
    let transformStream = new stream.Transform();
    let testListener1Called = false;
    let testListener2Called = false;
    let testListener1: Function = (): void => {
        testListener1Called = true;
    };
    let testListener2: Function = (): void => {
        testListener2Called = true;
    };
    transformStream.setEncoding('utf8');
    transformStream.on('readable', testListener1);
    transformStream.on('readable', testListener2);
    transformStream.off('readable');
    transformStream.push('test');
    setTimeout(() => {
        arktest.assertFalse(testListener1Called);
        arktest.assertFalse(testListener2Called);
    }, 0);
}

class TestTransform4 extends stream.Transform {
    expected = 'test';

    constructor() {
        super();
    }

    doTransform(chunk: string, encoding: string, callback: Function): void {
        callback.unsafeCall();
    }

    doFlush(callback: Function): void {
        callback.unsafeCall(this.expected);
    }
}
// Test cases ported from ArkTS 1.0:testStreamTransformFlushData001
function testTransformFlushOperation() {
    let transform = new TestTransform4();
    transform.end('my test');
    transform.on('data', (data: string): void => {
        arktest.assertEQ(data.toString(), transform.expected);
    });
}
