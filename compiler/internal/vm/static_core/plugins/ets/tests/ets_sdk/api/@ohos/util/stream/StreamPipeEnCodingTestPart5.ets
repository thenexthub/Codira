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

    suite.addAsyncTest("Duplex stream end method validation (gb18030).", testDuplexStreamEndEncodingGb18030);
    suite.addAsyncTest("Duplex stream end method validation (gbk).", testDuplexStreamEndEncodingGbk);
    suite.addAsyncTest("Duplex stream end method validation (gb2312).", testDuplexStreamEndEncodingGb2312);
    suite.addAsyncTest("Set default encoding for duplex stream (gb18030).", testDuplexStreamSetDefaultEncodingGb18030);

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

let callCount_MyDuplex1 = 0;
class MyDuplex1 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex1++;
        arktest.assertEQ(chunk, '测试');
        arktest.assertEQ(encoding, 'gb18030');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding019
async function testDuplexStreamEndEncodingGb18030() {
    mustChecks.push(() => {
        if (callCount_MyDuplex1 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex1} times.`);
        }
    });

    let duplex = new MyDuplex1();
    duplex.end('测试', 'gb18030', mustCall());
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyDuplex2 = 0;
class MyDuplex2 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex2++;
        arktest.assertEQ(chunk, '测试');
        arktest.assertEQ(encoding, 'gbk');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding020
async function testDuplexStreamEndEncodingGbk() {
    mustChecks.push(() => {
        if (callCount_MyDuplex2 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex2} times.`);
        }
    });

    let duplex = new MyDuplex2();
    duplex.end('测试', 'gbk', mustCall());
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyDuplex3 = 0;
class MyDuplex3 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex3++;
        arktest.assertEQ(chunk, '测试');
        arktest.assertEQ(encoding, 'gb2312');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding021
async function testDuplexStreamEndEncodingGb2312() {
    mustChecks.push(() => {
        if (callCount_MyDuplex3 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex3} times.`);
        }
    });

    let duplex = new MyDuplex3();
    duplex.end('测试', 'gb2312', mustCall());
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}

let callCount_MyDuplex4 = 0;
class MyDuplex4 extends stream.Duplex {
    constructor() {
        super();
    }

    doWrite(chunk: string | Uint8Array, encoding: string, callback: Function): void {
        callCount_MyDuplex4++;
        arktest.assertEQ(encoding, 'gb18030');
        callback.unsafeCall();
    }
}
// Test cases ported from ArkTS 1.0:testStreamEncoding022
async function testDuplexStreamSetDefaultEncodingGb18030() {
    mustChecks.push(() => {
        if (callCount_MyDuplex4 !== 1) {
            throw new Error(`Function was expected to be called 1 times, but was called ${callCount_MyDuplex4} times.`);
        }
    });

    let duplex = new MyDuplex4();
    let result = duplex.setDefaultEncoding('gb18030');
    duplex.write('你好');
    arktest.assertTrue(result);
    arktest.expectNoThrow(() => { await mustCallCheckAll(); });
}
