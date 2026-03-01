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

    suite.addAsyncTest("Writev finish event", testWritableWritevFinish);
    suite.addAsyncTest("Writev error", testWritableWritevError);
    suite.addAsyncTest("Writev error scenario one", testWritableWritevErrorScenarioOne);
    suite.addAsyncTest("Writev error scenario two", testWritableWritevErrorScenarioTwo);

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

class MyWritable1 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall(new Error('write test error'));
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableWritevFinish001
async function testWritableWritevFinish() {
    let writableStream = new MyWritable1();
    writableStream.on('finish', mustNotCall());
    writableStream.on('prefinish', mustNotCall());
    writableStream.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'write test error');
    }));
    writableStream.end('my test');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable2 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        setTimeout(() => {
            callback.unsafeCall(new Error('write error'));
        });
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableWritevFinish002
async function testWritableWritevError() {
    let writable = new MyWritable2();
    writable.on('finish', mustNotCall());
    writable.on('prefinish', mustNotCall());
    writable.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'write error');
    }));
    writable.end('my test');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable3 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        callback.unsafeCall(new Error('write error'));
    }

    doWritev(chunks: string[] | Uint8Array[], callback: Function): void {
        callback.unsafeCall(new Error('writev error'));
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableWritevFinish003
async function testWritableWritevErrorScenarioOne() {
    let writable = new MyWritable3();
    writable.on('finish', mustNotCall());
    writable.on('prefinish', mustNotCall());
    writable.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'writev error');
    }));
    writable.cork();
    writable.write('test');
    setTimeout(() => {
        writable.end('test');
    }, 10);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyWritable4 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        setTimeout(() => {
            callback.unsafeCall(new Error('write error'));
        });
    }

    doWritev(chunks: string[] | Uint8Array[], callback: Function): void {
        setTimeout(() => {
            callback.unsafeCall(new Error('writev error'));
        });
    }
}
// Test cases ported from ArkTS 1.0:testStreamWritableWritevFinish004
async function testWritableWritevErrorScenarioTwo() {
    let writable = new MyWritable4();
    writable.on('finish', mustNotCall());
    writable.on('prefinish', mustNotCall());
    writable.on('error', mustCall((error: Error): void => {
        arktest.assertEQ(error.message, 'writev error');
    }));
    writable.cork();
    writable.write('test');
    setTimeout(() => {
        writable.end('test');
    }, 0)
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
