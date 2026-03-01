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

    suite.addAsyncTest("Pipe data transfer validation.", testPipeDataTransferValidation);
    suite.addAsyncTest("Pipe with readable and writable streams.", testPipeReadableAndWritableStreams);
    suite.addAsyncTest("Pipe large data transfer validation.", testPipeLargeDataTransferValidation);

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

let callCount_MyWrite1 = 0;
class MyRead1 extends TestReadable {
    constructor() {
        super();
    }

    doRead(size: int) {
        this.push('test data');
        this.push(null);
    }
}
class MyWrite1 extends CustomWritable {
    str = "{\"0\":116,\"1\":101,\"2\":115,\"3\":116,\"4\":32,\"5\":100,\"6\":97,\"7\":116,\"8\":97}";

    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        this._doWriteImpl(chunk as Uint8Array, encoding, callback)
    }

    private _doWriteImpl(chunk: Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite1++;
        arktest.assertEQ(JSON.stringify(chunk), this.str);
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamPipe003
async function testPipeDataTransferValidation() {
    mustChecks.push(() => {
        if (callCount_MyWrite1 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyWrite1} times.`);
        }
    });

    let readable = new MyRead1();
    let writable = new MyWrite1();
    writable.on('finish', mustCallNoArgs());
    readable.pipe(writable);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyWrite2 = 0;
class MyRead2 extends TestReadable {
    constructor() {
        super();
    }

    doRead(size: int) {
        this.push('Some data');
        this.push(null);
    }
}
class MyWrite2 extends CustomWritable {
    str = "{\"0\":83,\"1\":111,\"2\":109,\"3\":101,\"4\":32,\"5\":100,\"6\":97,\"7\":116,\"8\":97}";

    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite2++;
        arktest.assertEQ(JSON.stringify(chunk as Uint8Array), this.str);
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamPipe004
async function testPipeReadableAndWritableStreams() {
    mustChecks.push(() => {
        if (callCount_MyWrite2 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyWrite2} times.`);
        }
    });

    let readable = new MyRead2();
    let writable = new MyWrite2();
    readable.pipe(writable);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyWrite3 = 0;
class MyRead3 extends stream.Readable {
    sumData = 0;

    constructor() {
        super();
    }

    doRead(size: int) {
        const onceData = 8 * 1024;
        if ((this.sumData + onceData) < 41000) {
            this.sumData += onceData;
            this.push('a'.repeat(onceData));
        } else if (this.sumData === 41000) {
            this.push(null);
        } else {
            this.push('a'.repeat(41000 - this.sumData));
            this.sumData += (41000 - this.sumData);
        }
    }
}
class MyWrite3 extends stream.Writable {
    sum: number = 0;

    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite3++;
        if (chunk instanceof string) {
            this.sumAdd(chunk)
        } else if (chunk instanceof Uint8Array) {
            this.sumAdd(chunk)
        }
        if (callCount_MyWrite3 === 6) {
            arktest.assertEQ(this.sum, 41000);
        }
        setTimeout(() => {
            callback.unsafeCall();
        }, 100);
    }

    private sumAdd(chunk: string): void {
        this.sum += chunk.length
    }

    private sumAdd(chunk: Uint8Array): void {
        this.sum += chunk.length
    }
}
// Test cases ported from ArkTS 1.0:testStreamPipe005
async function testPipeLargeDataTransferValidation() {
    mustChecks.push(() => {
        if (callCount_MyWrite3 !== 6) {
            throw new Error(`Function was expected to be called 6 times, but was called ${callCount_MyWrite3} times.`);
        }
    });

    let readable = new MyRead3();
    let writable = new MyWrite3();
    readable.pipe(writable);
    readable.unpipe(writable);
    readable.pipe(writable);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

class TestReadable extends stream.Readable {
    constructor() {
        super();
    }

    doRead(size: int) {
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
