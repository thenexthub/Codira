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

    suite.addAsyncTest("Set default encoding validation (gb2312).", testWritableStreamSetDefaultEncodingGb2312);
    suite.addTest("Readable stream set encoding validation (gb18030).", testReadableStreamSetEncodingGb18030);
    suite.addTest("Readable stream set encoding validation (gbk).", testReadableStreamSetEncodingGbk);
    suite.addTest("Readable stream set encoding validation (gb2312).", testReadableStreamSetEncodingGb2312);
    suite.addAsyncTest("Duplex stream write method validation (gb18030).", testDuplexStreamWriteEncodingGb18030);
    suite.addAsyncTest("Duplex stream write method validation (gbk).", testDuplexStreamWriteEncodingGbk);
    suite.addAsyncTest("Duplex stream write method validation (gb2312).", testDuplexStreamWriteEncodingGb2312);

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
    mustChecks.push((): void => {
        if (callCount !== times) {
            throw new Error(`Function was expected to be called ${times} times, but was called ${callCount} times.`);
        }
    });
    return wrappedFn;
}

let callCount_MyWrite = 0;
class MyWrite extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, 'gb2312');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding009
async function testWritableStreamSetDefaultEncodingGb2312() {
    mustChecks.push(() => {
        if (callCount_MyWrite !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyWrite} times.`);
        }
    });

    let writable = new MyWrite();
    writable.setDefaultEncoding('gb2312');
    writable.write('你好');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

// Test cases ported from ArkTS 1.0:testStreamEncoding010
function testReadableStreamSetEncodingGb18030() {
    let readable = new stream.Readable();
    readable.setEncoding('gb18030');
    arktest.assertEQ(readable.readableEncoding, 'gb18030');
}

// Test cases ported from ArkTS 1.0:testStreamEncoding011
function testReadableStreamSetEncodingGbk() {
    let readable = new stream.Readable();
    readable.setEncoding('gbk');
    arktest.assertEQ(readable.readableEncoding, 'gbk');
}

// Test cases ported from ArkTS 1.0:testStreamEncoding012
function testReadableStreamSetEncodingGb2312() {
    let readable = new stream.Readable();
    readable.setEncoding('gb2312');
    arktest.assertEQ(readable.readableEncoding, 'gb2312');
}

let callCount_MyDuplex1 = 0;
class MyDuplex1 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex1++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, 'gb18030');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding016
async function testDuplexStreamWriteEncodingGb18030() {
    mustChecks.push(() => {
        if (callCount_MyDuplex1 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex1} times.`);
        }
    });

    let duplex = new MyDuplex1();
    duplex.write('你好', 'gb18030');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyDuplex2 = 0;
class MyDuplex2 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex2++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, 'gbk');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding017
async function testDuplexStreamWriteEncodingGbk() {
    mustChecks.push(() => {
        if (callCount_MyDuplex2 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex2} times.`);
        }
    });

    let duplex = new MyDuplex2();
    duplex.write('你好', 'gbk');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyDuplex3 = 0;
class MyDuplex3 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex3++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, 'gb2312');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding018
async function testDuplexStreamWriteEncodingGb2312() {
    mustChecks.push(() => {
        if (callCount_MyDuplex3 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex3} times.`);
        }
    });

    let duplex = new MyDuplex3();
    duplex.write('你好', 'gb2312');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
