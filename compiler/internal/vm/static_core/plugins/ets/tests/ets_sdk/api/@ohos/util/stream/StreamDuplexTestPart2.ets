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

    suite.addTest("Duplex writableObjectMode property validation", testDuplexWritableObjectModeProperty);
    suite.addTest("Duplex writableHighWatermark property validation", testDuplexWritableHighWatermarkProperty);
    suite.addTest("Duplex readableHighWatermark property validation", testDuplexReadableHighWatermarkProperty);
    suite.addAsyncTest("Duplex doWritev concatenates chunks correctly", testDuplexDoWritevConcatenatesChunks);
    suite.addAsyncTest("Duplex doWritev error handling", testDuplexDoWritevErrorHandling);
    suite.addAsyncTest("Duplex doWritev callback multiple calls handling", testDuplexDoWritevCallbackMultipleCalls);
    suite.addTest("Duplex write undefined", testDuplexWriteUndefined);
    suite.addTest("Duplex write empty string", testDuplexWriteEmptyString);

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

// Test cases ported from ArkTS 1.0:testStreamDuplexProperties001
function testDuplexWritableObjectModeProperty() {
    let duplex = new stream.Duplex();
    arktest.assertFalse(duplex.writableObjectMode, "writableObjectMode should be false");
}

// Test cases ported from ArkTS 1.0:testStreamDuplexProperties002
function testDuplexWritableHighWatermarkProperty() {
    let duplex = new stream.Duplex();
    arktest.assertEQ(duplex.writableHighWatermark, 16 * 1024);
}

// Test cases ported from ArkTS 1.0:testStreamDuplexProperties003
function testDuplexReadableHighWatermarkProperty() {
    let duplex = new stream.Duplex();
    arktest.assertEQ(duplex.readableHighWatermark, 16 * 1024);
}

class CustomDuplex1 extends stream.Duplex {
    data: string;

    constructor() {
        super();
        this.data = '';
    }

    doWritev(chunks: string[] | Uint8Array[], callback: Function) {
        for (const chunk of chunks) {
            this.data += chunk
        }
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplexDoWritev001
async function testDuplexDoWritevConcatenatesChunks() {
    let duplex = new CustomDuplex1();
    let func: Function = mustCallNoArgs((): void => {
        arktest.assertEQ(duplex.data, 'Hello World');
    })
    duplex.write('Hello ');
    duplex.write('World', 'utf8', func);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class CustomDuplex2 extends stream.Duplex {
    constructor() {
        super();
    }

    doWritev(chunks: string[] | Uint8Array[], callback: Function) {
        callback.unsafeCall(new Error('dowritev error'));
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplexDoWritev002
async function testDuplexDoWritevErrorHandling() {
    let duplex = new CustomDuplex2();
    duplex.on('error', mustCall((err: Error): void => {
        arktest.assertEQ(err.message, 'dowritev error');
    }));
    duplex.write('Hello ');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_CustomDuplex3 = 0;
class CustomDuplex3 extends stream.Duplex {
    constructor() {
        super();
    }

    doWritev(chunks: string[] | Uint8Array[], callback: Function) {
        callCount_CustomDuplex3++;
        callback.unsafeCall();
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplexDoWritev003
async function testDuplexDoWritevCallbackMultipleCalls() {
    mustChecks.push(() => {
        if (callCount_CustomDuplex3 !== 1) {
            throw new Error(`Function was expected to be called 1 times, `
                            + `but was called ${callCount_CustomDuplex3} times.`);
        }
    });

    let duplex = new CustomDuplex3();
    duplex.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.name, 'BusinessError');
        arktest.assertEQ(error.message, 'Callback called multiple times');
    }));
    duplex.write('test', 'utf8', mustCallNoArgs());
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

function testDuplexWriteUndefined() {
    let err: BusinessError = new BusinessError();
    err.message = 'Parameter error. The type of undefined must be string or UintArray';
    let duplex = new stream.Duplex();
    arktest.expectError(() => { duplex.write(); }, err);
}

function testDuplexWriteEmptyString() {
    let err: BusinessError = new BusinessError();
    err.message = 'Cannot read property byteLength of undefined';
    let duplex = new stream.Duplex();
    arktest.expectError(() => { duplex.write(""); }, err);
}
