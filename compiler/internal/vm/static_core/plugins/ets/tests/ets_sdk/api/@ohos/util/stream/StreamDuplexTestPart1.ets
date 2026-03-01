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

    suite.addTest("Duplex finish event should not be triggered", testDuplexFinishEventNotTriggered);
    suite.addAsyncTest("Duplex writableFinished state validation", testDuplexWritableFinishedState);
    suite.addTest("Duplex basic read and write functionality", testDuplexBasicReadWrite);
    suite.addAsyncTest("Duplex read with valid size", testDuplexReadWithValidSize);
    suite.addAsyncTest("Duplex error handling during write", testDuplexErrorHandlingOnWrite);
    suite.addTest("Duplex read and write with synchronous data", testDuplexReadWriteWithSyncData);

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

function mustCallObjectArray(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (...args: Object[]): void => {
        callCount++;
        fn?.unsafeCall(args);
    }

    mustChecks.push(() => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
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

class MyDuplex1 extends stream.Duplex {
    constructor() {
        super();
    }

    doRead(size: int) {
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplexEnd001
function testDuplexFinishEventNotTriggered() {
    let finishCalled = false;
    let duplex = new MyDuplex1();
    duplex.on('finish', () => {
        finishCalled = true;
    })
    duplex.resume();
    duplex.push(null);
    arktest.assertFalse(finishCalled);
}

class MyDuplex2 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function) {
        arktest.assertFalse(this.writableFinished, "writableFinished should be false");
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplexWritableFinished002
async function testDuplexWritableFinishedState() {
    let duplex = new MyDuplex2();
    let func: Function =  mustCallNoArgs((): void => {
        arktest.assertTrue(duplex.writableFinished, "writableFinished should be true");
    })
    duplex.on('finish', func);
    duplex.end('testing finished state', 'utf8', mustCall((err: Error | undefined | null): void => {
        arktest.assertTrue(duplex.writableFinished, "writableFinished should be true");
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyDuplex3 extends stream.Duplex {
    written: string | Uint8Array = "";

    constructor() {
        super();
    }

    doRead(size: int) {
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        this.written = chunk;
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplex001
function testDuplexBasicReadWrite() {
    let read: Object;
    let duplex = new MyDuplex3();
    arktest.assertTrue(duplex instanceof stream.Duplex, "duplex instanceof Duplex should be true");
    duplex.on('data', (...param: Object[]): void => {
        if (param.length > 0 && param[0] instanceof Object) {
            read = param[0];
        }
    });
    duplex.push('read');
    duplex.end('written');
    setTimeout(() => {
        arktest.assertEQ(read.toString(), 'read');
        arktest.assertEQ(duplex.written, 'written');
    }, 0);
}

let callCount_MyDuplex4 = 0;
class MyDuplex4 extends stream.Duplex {
    constructor() {
        super();
    }

    doRead(size: int) {
        callCount_MyDuplex4++;
        arktest.assertEQ(size, 16384);
        this.push('duplex test');
        this.push(null);
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplex002
async function testDuplexReadWithValidSize() {
    mustChecks.push(() => {
        if (callCount_MyDuplex4 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex4} times.`);
        }
    });

    let duplex = new MyDuplex4();
    duplex.write('duplex test', 'utf8', mustCallObjectArray((...param: Object[]): void => {
        if (param.length > 0 && param[0] instanceof Error) {
            const err = param[0] as Error;
            arktest.assertEQ(err, null);
        }
    }));
    duplex.on('data', mustCallObjectArray((...param: Object[]): void => {
        if (param.length > 0 && param[0] instanceof string) {
            const chunk = param[0] as string;
            arktest.assertEQ(chunk.toString(), 'duplex test');
        }
    }));
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyDuplex5 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callback.unsafeCall(new Error('Write error'));
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplex003
async function testDuplexErrorHandlingOnWrite() {
    let duplex = new MyDuplex5();
    let func: Function = (err: Error) => {
        arktest.assertEQ(err.message, "Write error", "err.message is valid");
        arktest.assertFalse(duplex.writable, "writable should be false");
    }
    duplex.on('error', mustCall(func));
    duplex.write('test');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class MyDuplex6 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        setTimeout(() => {
            callback.unsafeCall();
        }, 20);
    }

    doRead(size: int) {
        this.push('Sync data');
        this.push(null);
    }
}
// Test cases ported from ArkTS 1.0:testStreamDuplex004
function testDuplexReadWriteWithSyncData() {
    let duplex = new MyDuplex6();
    duplex.on('data', (...chunk: Object[]): void => {
        if (chunk.length > 0 && chunk[0] instanceof string) {
            arktest.assertEQ(chunk[0].toString(), "Sync data");
        }
    });
    duplex.write('Test data');
}
