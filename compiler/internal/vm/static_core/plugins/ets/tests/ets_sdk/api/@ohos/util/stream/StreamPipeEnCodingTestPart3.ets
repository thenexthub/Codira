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

    suite.addAsyncTest("Writable stream end method validation (gbk).", testWritableStreamEndEncodingGbk);
    suite.addAsyncTest("Writable stream end method validation (gb2312).", testWritableStreamEndEncodingGb2312);
    suite.addAsyncTest("Set default encoding validation (gb18030).", testWritableStreamSetDefaultEncodingGb18030);
    suite.addAsyncTest("Set default encoding validation (gbk).", testWritableStreamSetDefaultEncodingGbk);

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

function mustCall(fn?: Function, times: number = 1) {
    let callCount = 0;
    let wrappedFn: Function = (err: Error | undefined | null) => {
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

let callCount_MyWrite1 = 0;
class MyWrite1 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite1++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, 'gbk');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding005
async function testWritableStreamEndEncodingGbk() {
    mustChecks.push(() => {
        if (callCount_MyWrite1 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyWrite1} times.`);
        }
    });

    let writable = new MyWrite1();
    writable.end('你好', 'gbk', mustCall());
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyWrite2 = 0;
class MyWrite2 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite2++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, 'gb2312');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding006
async function testWritableStreamEndEncodingGb2312() {
    mustChecks.push(() => {
        if (callCount_MyWrite2 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyWrite2} times.`);
        }
    });

    let writable = new MyWrite2();
    writable.end('你好', 'gb2312', mustCall());
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyWrite3 = 0;
class MyWrite3 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite3++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, 'gb18030');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding007
async function testWritableStreamSetDefaultEncodingGb18030() {
    mustChecks.push(() => {
        if (callCount_MyWrite3 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyWrite3} times.`);
        }
    });

    let writable = new MyWrite3();
    writable.setDefaultEncoding('gb18030');
    writable.write('你好');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyWrite4 = 0;
class MyWrite4 extends stream.Writable {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyWrite4++;
        arktest.assertEQ(chunk, '你好');
        arktest.assertEQ(encoding, 'gbk');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding008
async function testWritableStreamSetDefaultEncodingGbk() {
    mustChecks.push(() => {
        if (callCount_MyWrite4 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyWrite4} times.`);
        }
    });

    let writable = new MyWrite4();
    writable.setDefaultEncoding('gbk');
    writable.write('你好');
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
